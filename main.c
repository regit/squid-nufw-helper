/* $Id: main.c,v 1.3 2003/01/23 00:36:01 robertc Exp $
* Copyright (C) 2002 Rodrigo Campos
*           (C) 2004-2008 Vincent Deffontaines www.inl.fr
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* Original author: Rodrigo Campos (rodrigo@geekbunker.org)
* Adaptation to NuFW by Vincent Deffontaines
*
*/

#include "auth_nufw.h"

void
usage (char *program_name)
{
  fprintf (stderr, "Usage:\n%s -f <configuration file>\n\t\t[-c <num>] (leave after that number of entries - default 1000)\n\t\t[-u] (tell squid about user when identified)\n\t\t[-a] (run htons() on source port)\n\t\t[-U] (In case of problems connecting or requesting the database, let traffic pass through unauthenticated)\n",
          program_name);
}

void check_params(SQLconnection *object)
{
  int error = 0;
  if (object->host == NULL)
  {
      fprintf(stderr,"No host specified! Use db_host\n");
      error++;
  }
  if (object->user == NULL)
  {
      fprintf(stderr,"No user specified! Use db_user\n");
      error++;
  }
  if (object->pass == NULL)
  {
      fprintf(stderr,"No password specified! Use db_pass\n");
      error++;
  }
  if (object->database == NULL)
  {
      fprintf(stderr,"No database name specified! Use db_database\n");
      error++;
  }
  if (object->table == NULL)
  {
      fprintf(stderr,"No table name specified! Use db_table\n");
      error++;
  }
  if (error > 0)
  {
      printf("Sorry. Cannot run. Please refer to the above message(s)\n");
      exit(-1);
  }
}

int
main (int argc, char *argv[])
{
  FILE *FH;
  char *filename = NULL;
  int leave_count = 1000;
  int mention_user=0;
  int ignore_sql_problems=0;
  char *program_name = argv[0];
  char *cp;
  char *source_port;
  char *source_ip;
  char line[BUFSIZE];
  int ch;
  int count;
  int use_htons = 0;
  SQLconnection *SQLconn;
#ifdef DB_TYPE_MYSQL
  const char *result;
#else
  char *result;
#endif

  setvbuf (stdout, NULL, _IOLBF, 0);
  while ((ch = getopt (argc, argv, "auUc:f:")) != -1) {
    switch (ch) {
    case 'f':
      filename = optarg;
      break;
    case 'c':
      leave_count = atoi(optarg);
      break;
    case 'a':
      use_htons = 1;
      break;
    case 'u':
      mention_user = 1;
      break;
    case 'U':
      ignore_sql_problems = 1;
      break;
    default:
      usage (program_name);
      exit (1);
    }
  }
  if (filename == NULL) {
    usage (program_name);
    exit(1);
  }
  FH = fopen (filename, "r");
  if (!FH) {
    fprintf(stderr, "Unable to open configuration file: %s\n", filename);
    exit(1);
  }
  SQLconn = read_conf(FH);
  fclose(FH);
  check_params(SQLconn);
  result = SQLconnect(&SQLconn->sql,
                            SQLconn->host,
                            SQLconn->user,
                            SQLconn->database,
                            SQLconn->port,
                            SQLconn->pass);
  if (result!=NULL)
  {
      fprintf(stderr, "helper: unable to connect to database : %s\n",result);
      if (ignore_sql_problems == 0)
      {
      	exit(1);
      }
  }

  while (fgets (line, sizeof (line), stdin)) {
    char *user = NULL;
    leave_count --;
    if (leave_count <= 0)
    {
	if (result==NULL) //only skipped if ignore_sql_problems == 1
	{
        	SQLclose(SQLconn->sql);
	}
        return 0;
    }
    if ((cp = strchr (line, '\n')) != NULL) {
      *cp = '\0';
    }
    if ((cp = strtok (line, " \t")) != NULL) {
      source_ip = cp;
      source_port = strtok (NULL, " \t");
    } else {
      fprintf (stderr, "helper: unable to read tokens\n");
      printf ("ERR\n");
      continue;
    }
    if ((source_ip == NULL) || (source_port == NULL))
    {
        fprintf (stderr, "helper: unable to read tokens %d\n",leave_count);
        printf ("ERR\n");
        continue;
    }

    if (result==NULL)
    {
     if (SQLlookup(SQLconn,source_ip,source_port,&user,use_htons) == 0) {
      printf ("OK");
      if (mention_user)
      {
          printf(" user=%s\n",user);
          free(user);
      }
      else
          printf("\n");
     } else {
      printf ("ERR\n");
      count++;
     }
    }else{
	if (ignore_sql_problems == 1) //Actually always true when reaching this code
	{
	    printf("OK\n");
	}else{
	    fprintf(stderr, "BUG : consistency options problem, please contact authors!\n");
	    exit(2);
	}
    }
  }
  if (result==NULL)
  {
  	SQLclose(SQLconn->sql);
  }
  return 0;
}
