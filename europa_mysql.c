#include <stdio.h>
#include <mysql.h>
#include "europa_mysql.h"
#include "europa.h"

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
  char query[1000];
  char *end;

  end  = strmov (query, "INSERT INTO patches VALUES (NULL, '");
  end += mysql_real_escape_string(mysql, end, &p[PATCH_NAME_LEN_BYTE + 1], p[PATCH_NAME_LEN_BYTE]);
  end  = strmov (end, "', '', '");
  end += mysql_real_escape_string(mysql, end, p, len);
  end  = strmov (end, "', NULL, NOW())");

  if (mysql_real_query(mysql, query, end - query) != 0)
  {
    printf("query failed: %s\n", mysql_error(mysql));
    printf("query: %s\n", query);
    return 0;
  }
  return 1;
}
