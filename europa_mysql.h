void
do_disconnect (MYSQL *conn);

MYSQL *
do_connect (char *host_name, char *user_name, char *password, char *db_name,
            unsigned int port_num, char *socket_name, unsigned int flags);

int
real_save_patch (unsigned char *p, unsigned int len);
