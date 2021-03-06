/*
europa-gtk - a midi librarian for the Roland Jupiter 6 synth with Europa mod.

Copyright (C) 2005-2021  Jeremy March

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>. 
*/

#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include "europa_mysql.h"
#include "europa.h"

MYSQL *mysql;

static char *
strmov (register char *dst, register const char *src)
{
  while ((*dst++ = *src++)) ;
  return dst-1;
}

static void
print_error (MYSQL *conn, char *message)
{
  fprintf (stderr, "%s\n", message);
  if (conn != NULL)
  {
    fprintf (stderr, "Error %u (%s)\n",
             mysql_errno (conn), mysql_error (conn));
  }
}

MYSQL *
do_connect (char *host_name, char *user_name, char *password, char *db_name,
            unsigned int port_num, char *socket_name, unsigned int flags)
{
  MYSQL *conn;

  if (db_name == NULL)
    return NULL;  /* db name is required */

  conn = mysql_init (NULL);
  if (conn == NULL)
  {
    print_error (NULL, "mysql_init() failed (probably out of memory)");
    return (NULL);
  }

#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID >= 32200 /* 3.22 and up */
  if (mysql_real_connect (conn, host_name, user_name, password,
          db_name, port_num, socket_name, flags) == NULL)
  {
    print_error (conn, "mysql_real_connect() failed");
    return (NULL);
  }
#else              /* pre-3.22 */
  if (mysql_real_connect (conn, host_name, user_name, password,
                          port_num, socket_name, flags) == NULL)
  {
    print_error (conn, "mysql_real_connect() failed");
    return (NULL);
  }
  if (db_name != NULL)    /* simulate effect of db_name parameter */
  {
    if (mysql_select_db (conn, db_name) != 0)
    {
      print_error (conn, "mysql_select_db() failed");
      mysql_close (conn);
      return (NULL);
    }
  }
#endif
#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID >= 4100 /* 4.1.0 and up */
  if (mysql_query(conn, "SET CHARACTER SET utf8") != 0)
    return (NULL);
#endif
  return (conn);
}

void
do_disconnect (MYSQL *conn)
{
  mysql_close (conn);
}

int
real_save_patch (unsigned char *p, unsigned int len)
{
  char query[1024];
  char *end;

  char buf[1024];
	tohex(p, len, buf, 1024);
  //printf("insert: %s\n", buf);  

  end  = strmov (query, "INSERT INTO patches VALUES (NULL, '");
  end += mysql_real_escape_string(mysql, end, (char*)&p[PATCH_NAME_LEN_BYTE + 1], p[PATCH_NAME_LEN_BYTE]);
  end  = strmov (end, "', '', '");
  end += mysql_real_escape_string(mysql, end, buf, strlen(buf));
  end  = strmov (end, "', NULL, NOW())");

  if (mysql_real_query(mysql, query, end - query) != 0)
  {
    printf("query failed: %s\n", mysql_error(mysql));
    printf("query: %s\n", query);
    return 0;
  }
  return 1;
}

void init_db(MYSQL *conn) {

  char *query = "CREATE TABLE IF NOT EXISTS `patches` ("
  "`patch_id` int(10) unsigned NOT NULL AUTO_INCREMENT,"
  "`name` varchar(255) NOT NULL,"
  "`temp` varchar(255) NOT NULL,"
  "`patch` varchar(255) NOT NULL,"
  "`temp2` varchar(255) DEFAULT NULL,"
  "`timeadded` timestamp NOT NULL DEFAULT current_timestamp() ON UPDATE current_timestamp(),"
  "PRIMARY KEY (`patch_id`)"
") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

  mysql_query(conn, query);
}
