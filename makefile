CC = gcc -std=c99
DEBUG = -g -Wall -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_44 -DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_60 -DGTK_DISABLE_DEPRECATED
#DEBUG = -O3 -march=pentium3
INCLUDES = `mysql_config --cflags`
LIBS = `mysql_config --libs`
GTK_THD_LIBS = `pkg-config --libs gthread-2.0 gtk+-2.0`
GTK_LIBS = `pkg-config --libs gtk+-2.0`

GTK_INC = `pkg-config --cflags gtk+-2.0`
ALSA_LIBS = -lasound

all: europa

main.o: main.c
	$(CC) $(DEBUG) -c main.c $(GTK_INC) $(INCLUDES)

europa_patch.o: europa_patch.c
	$(CC) $(DEBUG) -c europa_patch.c $(GTK_INC) $(INCLUDES)

europa_midi.o: europa_midi.c
	$(CC) $(DEBUG) -c europa_midi.c $(GTK_INC) $(INCLUDES)

europa_mysql.o: europa_mysql.c
	$(CC) $(DEBUG) -c europa_mysql.c $(GTK_INC) $(INCLUDES)

europa_library.o: europa_library.c
	$(CC) $(DEBUG) -c europa_library.c $(GTK_INC) $(INCLUDES)

europa: europa_patch.o europa_midi.o europa_mysql.o europa_library.o main.o
	$(CC) $(DEBUG) -o europa main.o europa_patch.o europa_midi.o europa_mysql.o europa_library.o $(ALSA_LIBS) $(GTK_LIBS) $(LIBS)

clean:
	rm -f europa *.o
