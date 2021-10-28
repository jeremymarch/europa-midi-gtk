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

void
do_disconnect (MYSQL *conn);

MYSQL *
do_connect (char *host_name, char *user_name, char *password, char *db_name,
            unsigned int port_num, char *socket_name, unsigned int flags);

int
real_save_patch (unsigned char *p, unsigned int len);

void tohex(unsigned char * in, size_t insz, char * out, size_t outsz);

void init_db(MYSQL *conn);