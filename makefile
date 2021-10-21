CC = gcc
DEBUG = -g -Wall --pedantic
#DEBUG = -O3 -march=pentium3
INCLUDES = `pkg-config --cflags mariadb`
LIBS = `pkg-config --libs mariadb`
GTK_THD_LIBS = `pkg-config --libs gthread-2.0 gtk+-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0`

GTK_INC = `pkg-config --cflags gtk+-2.0`
ALSA_LIBS = -lasound

all: europa

europa_patch.o: europa_patch.c
	$(CC) $(DEBUG) -c europa_patch.c $(GTK_INC) $(INCLUDES)

europa_midi.o: europa_midi.c
	$(CC) $(DEBUG) -c europa_midi.c $(GTK_INC) $(INCLUDES)

europa_mysql.o: europa_mysql.c
	$(CC) $(DEBUG) -c europa_mysql.c $(GTK_INC) $(INCLUDES)

europa_library.o: europa_library.c
	$(CC) $(DEBUG) -c europa_library.c $(GTK_INC) $(INCLUDES)

europa: europa_patch.o europa_midi.o europa_mysql.o europa_library.o
	$(CC) $(DEBUG) -o europa europa_patch.o europa_midi.o europa_mysql.o europa_library.o $(ALSA_LIBS) $(GTK_LIBS) $(LIBS)

clean:
	rm -f europa europa_patch.o europa_midi.o europa_mysql.o europa_library.o
