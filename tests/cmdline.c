#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define NUL 0

//----- configuration structure -----
struct config_s {
  long threshld;
  bool boguscsum;
};

//----- options structure -------
struct option_s {
  int  id;            // Maybe this can be deleted and the array index
                      // used, instead.
  char short_option;  // short option character
  char *long_option;  // long option string.

  bool has_arg;      // has args?
  bool alone;        // this option must appear alone?
  bool used;         // already in use?
};

//------ options enumeration ------
enum {
  OPT_HELP=1,
  OPT_VERSION,
  OPT_LSTPROTO,
  OPT_BGUSCSUM,
  OPT_FLOOD,
  OPT_THRESHLD
};

//----- options table ------
struct option_s options[] = {
 /* id            shrt, long,         args, alone, used */
  { OPT_HELP,     'h',  "help",           0,    1        },
  { OPT_VERSION,  'v',  "version",        0,    1        },  
  { OPT_LSTPROTO, 'l',  "list-protocols", 0,    1        },
  { OPT_BGUSCSUM, 'B',  "bocus-csum",     0              },
  { OPT_FLOOD,    NUL,  "flood",          0              },
  { OPT_THRESHLD, 't',  "threshold",      1              },
  {  }  // null option.
};

//----- configuration table -----
struct config_s config = { .threshld = 1000 };

#define CFG_ERRBUF_SIZE 64

static struct option_s *find_short_option(char);
static struct option_s *find_long_option(char *);
static void config_no_args_option(int, struct config_s *);
static int config_arg_option(int, char *, char *, struct config_s *);

int parse_cmdline(char **argv)
{
  struct option_s *optptr;
  int count = 0;
  bool arg = false;

  for (; *argv; argv++)
  {
    if (!arg) // if not an arg, an option!
    {
      if (**argv == '-')
      {
        char *opt = *argv + 1;

        if (*opt == '-')    // long option.
          optptr = find_long_option(opt + 1);
        else                // short option
          optptr = find_short_option(*opt);

        if (!optptr)
        {
          fprintf(stderr, "Option '%s' not recognized.\n", *argv);
          return 0;
        }

        count++;

        if (optptr->used)
        {
          fprintf(stderr, "Option '%s' used more than once.\n", *argv);
          return 0;
        }

        if (optptr->has_arg)
          arg = true;

        if (!arg)   // Process option with no args.
          config_no_args_option(optptr->id, &config);

        optptr->used = true;
      }
      else
      {
        // FIXME: Could be IP/CIDR...
        fprintf(stderr, "Option expected but found '%s'.\n", *argv);
        return 0;
      }
    }
    else  // this must be an arg.
    {
      char errstr[CFG_ERRBUF_SIZE+1];

      if (!config_arg_option(optptr->id, *argv, errstr, &config))  // if argument is wrong...
      {
        fprintf(stderr, "Option '%s' argument is wrong%s\n",
          *(argv - 1), errstr);
        return 0;
      }

      arg = false;  // Arg processed, next one must be an option.
    }
  }

  // Check for alone options.
  int r = 1;
  for (optptr = options; optptr->id != -1; optptr++) 
    if (optptr->alone && count > 1)
    {
      fprintf(stderr, "Option '-%c' (--%s) must be used alone.\n",
        optptr->short_option, optptr->long_option);
      r = 0;
    }

  return r;
}

//--- tries to find an option on table -----
struct option_s *find_short_option(char opt)
{
  struct option_s *p = options;

  while (p->id)
  {
    if (p->short_option == opt)
      break;
    p++;
  }

  if (!p->id)
    return NULL;

  // Return the struct option_s entry pointer if found.
  return p;
}

struct option_s *find_long_option(char *opt)
{
  struct option_s *p = options;

  while (p->id)
  {
    if (!strcmp(p->long_option, opt))
      break;
    p++;
  }

  if (!p->id)
    return NULL;

  return p;
}

void config_no_args_option(int opt, struct config_s *cfg)
{ 
  switch (opt)
  {
    case OPT_BGUSCSUM: 
      cfg->boguscsum = 1; break;
  }
}

int config_arg_option(int opt, char *arg, char *errstr, struct config_s *cfg)
{
  switch (opt)
  {
    case OPT_THRESHLD:
      {
        char *estr;
        long v;

        v = strtol(arg, &estr, 10);
        if (*estr || v <= 0)
        {
          strcpy(errstr, ": invalid value");
          return 0;
        }
        cfg->threshld = v;
      }
      break;
  }

  return 1;
}

int validate_options(void)
{
  struct option_s *optptr;
  bool flooding, thresholding;

  flooding = thresholding = false;
  for (optptr = options; optptr->id != -1; optptr++)
  {
    if (optptr->short_option == 't' && optptr->used) thresholding = 1;
    if (!strcmp(optptr->long_option, "flood") && optptr->used) flooding = 1;

    // Flooding and thresholding is not allowed!
    if (thresholding && flooding)
    {
      fputs("Cannot use threshold while trying to flood.\n", stderr);
      return 0;
    }
  }

  return 1;
}

void list_protocols(void) {}
void doWork(void) {}

int main(int argc, char *argv[])
{
  struct option_s *optptr;

  if (parse_cmdline(++argv))
    if (validate_options())
    {
      optptr = find_short_option('v');
      if (optptr->used)
      {
        puts("T50 5.7.1");
        return EXIT_SUCCESS;
      }

      // Show Help!
      optptr = find_short_option('h');
      if (optptr->used)
      {
        puts("Help!");
        return EXIT_SUCCESS;
      }

      // Show list of protocols...
      optptr = find_short_option('l');
      if (optptr->used)
      {
        list_protocols();
        return EXIT_SUCCESS;
      }

      // Only root can use T50.
      if (getuid())
      {
        fputs("You must have root privileges to use T50.\n", stderr);
        return EXIT_FAILURE;
      }

      doWork();
      return EXIT_SUCCESS;
    }

  return EXIT_FAILURE;
}
