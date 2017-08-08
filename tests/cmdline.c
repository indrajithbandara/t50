#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//----- configuration structure -----
struct config_s {
  long threshld;
  _Bool boguscsum;
};

//----- options structure -------
struct option_s {
  int  id;
  char short_option;
  char *long_option;
  _Bool has_arg;
  _Bool alone;
  _Bool used;
};

//------ options enumeration ------
enum {
  OPT_HELP,
  OPT_VERSION,
  OPT_LSTPROTO,
  OPT_BGUSCSUM,
  OPT_THRESHLD
};

//----- options table ------
struct option_s options[] = {
 /* id            shrt, long,         args, alone, used */
  { OPT_HELP,     'h',  "help",           0,    1        },
  { OPT_VERSION,  'v',  "version",        0,    1        },  
  { OPT_LSTPROTO, 'l',  "list-protocols", 0,    1        },
  { OPT_BGUSCSUM, 'B',  "bocus-csum",     0              },
  { OPT_THRESHLD, 't',  "threshold",      1              },
  { -1 }  // null option.
};

//----- configuration table -----
struct config_s config = { .threshld = 1000 };

int is_option(char *s) { return *s == '-'; }

//--- tries to find an option on table -----
struct option_s *find_short_option(char opt)
{
  struct option_s *p;

  p = options;
  while (p->id != -1)
  {
    if (p->short_option == opt)
      break;
    p++;
  }

  if (p->id == -1)
    return NULL;

  return p;
}

struct option_s *find_long_option(char *opt)
{
  struct option_s *p;

  p = options;
  while (p->id != -1)
  {
    if (!strcmp(p->long_option, opt))
      break;
    p++;
  }

  if (p->id == -1)
    return NULL;

  return p;
}

#define CFG_ERRBUF_SIZE 64

void config_no_args_option(int, struct config_s *);
int config_arg_option(int, char *, char *, struct config_s *);

int parse_cmdline(char **argv)
{
  struct option_s *optptr;
  int count = 0;
  _Bool arg = 0;

  for (; *argv; argv++)
  {
    if (!arg) // if not an arg, an option!
    {
      if (is_option(*argv))
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
          arg = 1;

        if (!arg)   // Process option with no args.
          config_no_args_option(optptr->id, &config);

        optptr->used = 1;
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

      arg = 0;  // Arg processed, next one must be an option.
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

int main(int argc, char *argv[])
{
  parse_cmdline(++argv);
  return EXIT_SUCCESS;
}
