#include <stdio.h>
#include "x86_adapt.h"
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

void print_help(char ** argv)
{
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage: %s [ %s-ARGS ...] \"COMMAND [ ARGS ...]\"\n", argv[0], argv[0]);
  fprintf(stderr, "\n");
  fprintf(stderr, "%s-ARGS:\n",argv[0]);
  fprintf(stderr, "\t -h --help: print this help\n");
  fprintf(stderr, "\t -H --hex: print results in hexadecimal form\n");
  fprintf(stderr, "\t -d --die: print die options instead of CPU options\n");
  fprintf(stderr, "\t -c --cpu: Use this CPU (default=all)\n"
                                "If -d is set, use the die with this nr.\n");
  fprintf(stderr, "\t -v --verbose: print more information\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "This program reads all available CPU knobs from x86_adapt.\n");
}

void print_cis(x86_adapt_device_type type, int verbose)
{
  struct x86_adapt_configuration_item item;
  int nr_items = x86_adapt_get_number_cis(type);
  int ci_nr = 0;
  for ( ci_nr = 0 ; ci_nr < nr_items ; ci_nr++ )
  {
    if (x86_adapt_get_ci_definition(type,ci_nr,&item))
    {
      fprintf(stderr,"Could not read item definition\n");
      exit(1);
    }
    if (verbose)
      fprintf(stdout,"Item %i: %s\n%s\n----------------\n",ci_nr,item.name,item.description);
    else
      fprintf(stdout,"Item %i: %s\n----------------\n",ci_nr,item.name);
  }
}

void print_header(x86_adapt_device_type type)
{
  int nr_items = x86_adapt_get_number_cis(type);
  int ci_nr = 0;

  switch (type)
  {
    case X86_ADAPT_CPU:
      fprintf(stdout,"CPU;");
    case X86_ADAPT_DIE:
      fprintf(stdout,"Node;");
  }
  for ( ci_nr = 0 ; ci_nr < nr_items ; ci_nr++ )
  {
    fprintf(stdout,"Item %d;",ci_nr);
  }
  fprintf(stdout,"\n");
}

void print_cpu(x86_adapt_device_type type, int cpu, int print_hex)
{
  struct x86_adapt_configuration_item item;
  int nr_items = x86_adapt_get_number_cis(type);
  int fd = x86_adapt_get_device_ro(type, cpu);
  uint64_t result;
  int ret;
  int ci_nr = 0;
  fprintf(stdout,"%d;",cpu);
  if ( fd > 0 )
  {
    for ( ci_nr = 0 ; ci_nr < nr_items ; ci_nr++ )
    {
      if ((ret = x86_adapt_get_setting(fd,ci_nr,&result)) != 8)
      {
        fprintf(stderr,"Could not read definitions for cpu/die %d\n",cpu);
        exit(1);
      }
      if (print_hex)
        fprintf(stdout,"%"PRIx64";",result);
      else
        fprintf(stdout,"%"PRIu64";",result);
    }
  } else
  {
    fprintf(stderr,"Could not open /dev/x86_adapt/... file for cpu/die %d\n",cpu);
    exit(-1);
  }
  fprintf(stdout,"\n");
  x86_adapt_put_device(type, cpu);
}

int main(int argc, char ** argv)
{
  int cpu=-1;
  int hex=0;
  char c;
  int verbose=0;
  x86_adapt_device_type type = X86_ADAPT_CPU;


  while (1)
  {
    static struct option long_options[] =
      {
          {"help", no_argument, 0, 'h'},
          {"hex", no_argument, 0, 'H'},
          {"die", no_argument, 0, 'd'},
          {"cpu", required_argument, 0, 'c'},
          {"verbose", no_argument, 0, 'v'},
          {0,0,0,0}
      };
    int option_index = 0;

    c = getopt_long(argc, argv, "hHdc:v", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 'h':
      print_help(argv);
      return 1;
    case 'H':
      hex = 1;
      break;
    case 'd':
      type = X86_ADAPT_DIE;
      break;
    case 'c':
      cpu = atoll(optarg);
      break;
    case 'v':
      verbose = 1;
      break;
    default:
      break;
    }
  } /* end of parse arguments */

  if (x86_adapt_init())
  {
    fprintf(stderr,"Could not initialize x86_adapt library");
  }

  print_cis(type, verbose);
  print_header(type);

  if ( cpu == -1 )
  {
    for ( cpu = 0 ; cpu < x86_adapt_get_nr_avaible_devices(type) ; cpu++ )
    {
      print_cpu(type, cpu, hex);
    }
  } else
  {
    print_cpu(type, cpu, hex);
  }
}
