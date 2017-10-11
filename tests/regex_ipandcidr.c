#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <stdio.h>
#include "cycle_counting.h"

typedef struct
{
  unsigned char cidr;
  unsigned int addr;
} T50_tmp_addr_t;


#define CIDR_MINIMUM 2
#define CIDR_MAXIMUM 32
#define error(...) { fprintf(stderr, __VA_ARGS__); }

/*--- Ok, this piece of code was, initially, an experiment.
      I have to review this code and get rip of this regex. --- */

/* POSIX Extended Regular Expression used to match IP addresses with optional CIDR. */
#define IP_REGEX "^([1-2]*[0-9]{1,2})" \
  "(\\.[1-2]*[0-9]{1,2}){0,1}" \
  "(\\.[1-2]*[0-9]{1,2}){0,1}" \
  "(\\.[1-2]*[0-9]{1,2}){0,1}" \
  "(/[0-9]{1,2}){0,1}$"

/* Auxiliary "match" macros. */
#define MATCH(a)        ((a).rm_so >= 0)
#define MATCH_LENGTH(a) ((a).rm_eo - (a).rm_so)

/* NOTE: There is a bug in strncpy() function.
         '\0' is not set at the end of substring. */
#define COPY_SUBSTRING(d, s, len) { \
    strncpy((d), (s), (len)); \
    *((char *)(d) + (len)) = '\0'; \
  }

/* Ok... this is UGLY. */
_Bool get_ip_and_cidr_from_string(char const *const addr, T50_tmp_addr_t *addr_ptr)
{
  regex_t re;
  regmatch_t rm[6];
  unsigned matches[5];
  size_t i, len;
  char *t;
  int bits;

  addr_ptr->addr = addr_ptr->cidr = 0;

  /* Try to compile the regular expression. */
  if (regcomp(&re, IP_REGEX, REG_EXTENDED))
    return false;

  /* Try to execute regex against the addr string. */
  if (regexec(&re, addr, 6, rm, 0))
  {
    regfree(&re);
    return false;
  }

  /* Allocate enough space for temporary string. */
  if ((t = strdup(addr)) == NULL) 
    error("Cannot allocate temporary string: " __FILE__ ":%d", __LINE__);

  /* Convert IP octects matches. */
  len = MATCH_LENGTH(rm[1]);
  COPY_SUBSTRING(t, addr + rm[1].rm_so, len);
  matches[0] = atoi(t);

  bits = CIDR_MAXIMUM;  /* default is 32 bits netmask. */

  for (i = 2; i <= 4; i++)
  {
    if (MATCH(rm[i]))
    {
      len = MATCH_LENGTH(rm[i]) - 1;
      COPY_SUBSTRING(t, addr + rm[i].rm_so + 1, len);
      matches[i - 1] = atoi(t);
    }
    else
    {
      /* if octect is missing, decrease 8 bits from netmask */
      bits -= 8;
      matches[i - 1] = 0;
    }
  }

  /* Convert cidr match. */
  if (MATCH(rm[5]))
  {
    len = MATCH_LENGTH(rm[5]) - 1;
    COPY_SUBSTRING(t, addr + rm[5].rm_so + 1, len);

    if ((matches[4] = atoi(t)) == 0)
    {
      /* if cidr is actually '0', then it is an error! */
      free(t);
      regfree(&re);
      return false;
    }
  }
  else
  {
    /* if cidr is not given, use the calculated one. */
    matches[4] = bits;
  }

  /* We don't need 't' string anymore. */
  free(t);

  /* Validate ip octects */
  for (i = 0; i < 4; i++)
    if (matches[i] > 255)
    {
      regfree(&re);
      return false;
    }

  /* NOTE: Check 'bits' here! */
  /* Validate cidr. */
  if (matches[4] < CIDR_MINIMUM || matches[4] > CIDR_MAXIMUM)
  {
    error("CIDR must be between %u and %u.\n", CIDR_MINIMUM, CIDR_MAXIMUM);
    regfree(&re);
    return false;
  }

  regfree(&re);

  /* Prepare CIDR structure */
  addr_ptr->cidr = matches[4];
  addr_ptr->addr = ( matches[3]         |
                     (matches[2] << 8)  |
                     (matches[1] << 16) |
                     (matches[0] << 24)) &
                   (0xffffffffUL << (32 - addr_ptr->cidr));

  return true;
}

void main(void)
{
  unsigned long c1;
  T50_tmp_addr_t addr;

  BEGIN_TSC;
  if (!get_ip_and_cidr_from_string("10.10.10.0/28", &addr))
    fprintf(stderr, "error");
  END_TSC(c1);
  printf("Cycles: %lu\n", c1);
}
