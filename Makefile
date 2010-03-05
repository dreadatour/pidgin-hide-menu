LINUX32_COMPILER = gcc

LIBPURPLE_CFLAGS = -I/usr/include/pidgin/ -I/usr/include/libpurple -DPURPLE_PLUGINS -DENABLE_NLS
GLIB_CFLAGS =                   \
    -I/usr/include/gtk-2.0      \
    -I/usr/lib/gtk-2.0/include  \
    -I/usr/include/cairo        \
    -I/usr/include/pango-1.0    \
    -I/usr/include/atk-1.0      \
    -I/usr/include/glib-2.0     \
    -I/usr/lib/glib-2.0/include \
    -I/usr/include

all:	release

install:
	cp hidemenu.so /usr/lib/pidgin/

uninstall:
	rm -f /usr/lib/pidgin/hidemenu.so

clean:
	rm -f hidemenu.so

hidemenu.so:	hidemenu.c
	${LINUX32_COMPILER} ${LIBPURPLE_CFLAGS} -Wall -pthread ${GLIB_CFLAGS} -I. -g -O2 -pipe hidemenu.c -o hidemenu.so -shared -fPIC -DPIC

release:	hidemenu.so

