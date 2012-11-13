/* Copyright (C) 2004 Vincent Deffontaines - INL
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
* Author:  Vincent Deffontaines (vincent@inl.fr)
*
*/

#include "auth_nufw.h"

#ifdef DB_TYPE_MYSQL
const char *SQLconnect(MYSQL **sql,
                         char *host,
                         char *user,
                         char *database,
                         unsigned int port,
                         char *pass)
{
  *sql = mysql_init(*sql);
  if (*sql == NULL)
    return "NULL pointer";

/*  if (ssl_enabled)
    mysql_ssl_set(*sql,
                  ssl_key,
                  ssl_cert,
                  ssl_ca,
                  ssl_ca_dir,
                  ssl_cypher);*/

  if (! mysql_real_connect(*sql,host,user,pass,database,port,NULL,0))
//    nufw_log_error(APLOG_MARK,APLOG_WARNING,NULL,"nufw_sql_connectdb #2 : %s",mysql_error(*sql));
  {
      return (mysql_error(*sql));
  }
  return NULL;
}

int SQLlookup(SQLconnection *object,char *source_ip, char *source_port, char **user, int use_htons)
{
  MYSQL_RES *result;
  MYSQL_ROW row;
  char request[512];
  int count;
  struct in_addr src_addr;
  int port = 0;
  if (inet_pton(AF_INET,source_ip,&src_addr)<=0)
  {
      return CANT_READ_ADDRESS;
  }

  if (use_htons)
  {
      port = htons(atoi(source_port));
  }else{
      port = atoi(source_port);
  }

  if (snprintf(request,511,"SELECT DISTINCT username FROM %s WHERE (tcp_sport=%u AND ip_saddr=%lu AND (state=%d OR state=%d))",
        object->table,
        port,
        (long unsigned int)htonl(src_addr.s_addr),
        STATE_OPEN,
        STATE_ESTABLISHED)
      >=511)
      return SQL_CANT_BUILD_REQUEST;
  if (mysql_real_query(object->sql, request, strlen(request)) != 0)
      return SQL_QUERY_ERROR;

  result = mysql_store_result(object->sql);
  if (result == NULL)
      return SQL_RESULT_ERROR;

  count = mysql_num_rows(result);
  if (count < 1)
  {
      mysql_free_result(result);
      return SQL_NO_MATCH;
  }
  if (count > 1)
  {
      mysql_free_result(result);
      return SQL_TOO_MANY_MATCHES;
  }
  row = mysql_fetch_row(result);
  *user = strdup(row[0]);
  if (*user == NULL)
       return SQL_INSUFFICIENT_MEM;
  mysql_free_result(result);
  return 0;
}

int SQLclose(MYSQL *sql)
{
  if (sql != NULL)
    mysql_close(sql);

  return 0;
}
#else
char *SQLconnect(PGconn **sql,
                         char *host,
                         char *user,
                         char *database,
                         unsigned int port,
                         char *pass)
{
  char *pgsql_conninfo;
  char server_port[15];
  int pgsql_status;

  /* Guess why so many programmers write stuff for mysql rather than pg? this
   * API is not as convenient as mysql's IMHO*/
  if (snprintf(server_port,14,"%d",port) >= 14){return "snprintf() failed";}
  pgsql_conninfo = (char *)calloc(strlen(user) + strlen(pass) +
              strlen(host) + strlen(server_port) + strlen(database) +
              strlen("hostaddr='' port= dbname='' user='' password='' ") + 1,
              sizeof(char));
  if (pgsql_conninfo == NULL){return "could not calloc()";}
  strncat(pgsql_conninfo,"host='",6);
  strncat(pgsql_conninfo,host,strlen(host));
  strncat(pgsql_conninfo,"' port=",7);
  strncat(pgsql_conninfo,server_port,strlen(server_port));
  strncat(pgsql_conninfo," dbname='",9);
  strncat(pgsql_conninfo,database,strlen(database));
  strncat(pgsql_conninfo,"' user='",8);
  strncat(pgsql_conninfo,user,strlen(user));
  strncat(pgsql_conninfo,"' password='",12);
  strncat(pgsql_conninfo,pass,strlen(pass));
  strncat(pgsql_conninfo,"'",1);

  *sql = PQconnectdb(pgsql_conninfo);
  pgsql_status = PQstatus(*sql);

  free(pgsql_conninfo);
  if(pgsql_status != CONNECTION_OK) {
    PQfinish(*sql);
    return "Cannot connect to PostgreSQL database.";
  }
  return NULL;
}

int SQLclose(PGconn *sql)
{
  if (sql != NULL)
    PQfinish(sql);
  return 0;
}

int SQLlookup(SQLconnection *object,char *source_ip, char *source_port, char **user, int use_htons)
{
  PGresult *result;
  char request[512];
  int count;
  int port = 0;
  char *str_res;
  if (use_htons)
  {
      port = htons(atoi(source_port));
  }else{
      port = atoi(source_port);
  }
  if (snprintf(request,511,"SELECT DISTINCT username FROM %s WHERE (tcp_sport=%u AND ip_saddr='%s' AND (state=%d OR state=%d))",
        object->table,
        port,
        source_ip,
        STATE_OPEN,
        STATE_ESTABLISHED)
      >=511)
      return SQL_CANT_BUILD_REQUEST;

  result = PQexec(object->sql,request);
  if (PQresultStatus(result) != PGRES_COMMAND_OK)
  {
      PQclear(result);
      return SQL_QUERY_ERROR;
  }
  if (result == NULL)
  {
      PQclear(result);
      return SQL_QUERY_ERROR;
  }

  count = PQntuples(result);
  if (count < 1)
  {
      PQclear(result);
      return SQL_NO_MATCH;
  }
  if (count > 1)
  {
      PQclear(result);
      return SQL_TOO_MANY_MATCHES;
  }
  if (PQgetisnull(result,0,0))
  {
      PQclear(result);
      return SQL_NO_MATCH;
  }
  str_res = PQgetvalue(result,0,0);
  PQclear(result);
  *user = strdup(str_res);
  if (*user == NULL)
      return SQL_INSUFFICIENT_MEM;
  return 0;
}
#endif
