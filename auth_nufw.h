/* $Id: ip_user.h,v 1.2 2003/01/23 00:36:01 robertc Exp $
* Copyright (C) 2002 Rodrigo Campos
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
* Author: Rodrigo Campos (rodrigo@geekbunker.org)
* Adaptation to NuFW by Vincent Deffontaines (vincent@inl.fr)
*
*/

#ifndef ALREADY_READ
#define ALREADY_READ

#define STATE_OPEN 0x1
#define STATE_ESTABLISHED 0x2

#define CANT_READ_ADDRESS -1
#define SQL_CANT_BUILD_REQUEST -2
#define SQL_QUERY_ERROR -3
#define SQL_RESULT_ERROR -4
#define SQL_NO_MATCH -5
#define SQL_TOO_MANY_MATCHES -6
#define SQL_INSUFFICIENT_MEM -7

#define BUFSIZE 1024

#include "config.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef DB_TYPE_MYSQL
#  include <mysql/mysql.h>
#else
#  include <postgresql/libpq-fe.h>
#endif


typedef struct _SQLconnection {
#ifdef DB_TYPE_MYSQL
  MYSQL *sql;
#else
  PGconn *sql;
#endif
  char *host;
  unsigned int port;
  char *user;
  char *database;
  char *pass;
  char *table;
  int ssl_enabled;
  char *ssl_key;
  char *ssl_cert;
  char *ssl_ca;
  char *ssl_ca_dir;
  char *ssl_cypher;
//  struct _SQLconnection *next;
}SQLconnection;


SQLconnection *read_conf(FILE *FH);

#ifdef DB_TYPE_MYSQL
  const char *SQLconnect(MYSQL **sql, char *host, char *user, char *database, unsigned int port, char *pass);
  int SQLclose(MYSQL *sql);
#else
  char *SQLconnect(PGconn **sql, char *host, char *user, char *database, unsigned int port, char *pass);
  int SQLclose(PGconn *sql);
#endif
int SQLlookup(SQLconnection *object,char *source_ip, char *source_port, char **user, int use_htons);

#endif
