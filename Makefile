DESTDIR?=/usr
PREFIX?=/local

ifneq ($V,1)
Q ?= @
endif

CC	= gcc
CFLAGS	= $(DEBUG) -Wall $(INCLUDE) -Winline -pipe 

LDFLAGS	= -L$(DESTDIR)$(PREFIX)/lib
LIBS    = -lpthread -lrt -lm -lcrypt

SRC	=	src/relay.c src/comm.c src/thread.c src/rs485.c src/modbus-data.c src/modbus-rtu.c src/modbus.c

OBJ	=	$(SRC:.c=.o)

all:	3relind

3relind:	$(OBJ)
	$Q echo [Link]
	$Q $(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

.c.o:
	$Q echo [Compile] $<
	$Q $(CC) -c $(CFLAGS) $< -o $@

.PHONY:	clean
clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) 3relind *~ core tags *.bak

.PHONY:	install
install: 3relind
	$Q echo "[Install]"
	$Q cp 3relind		$(DESTDIR)$(PREFIX)/bin
ifneq ($(WIRINGPI_SUID),0)
	$Q chown root:root	$(DESTDIR)$(PREFIX)/bin/3relind
	$Q chmod 4755		$(DESTDIR)$(PREFIX)/bin/3relind
endif
#	$Q mkdir -p		$(DESTDIR)$(PREFIX)/man/man1
#	$Q cp megaio.1		$(DESTDIR)$(PREFIX)/man/man1

.PHONY:	uninstall
uninstall:
	$Q echo "[UnInstall]"
	$Q rm -f $(DESTDIR)$(PREFIX)/bin/3relind
	$Q rm -f $(DESTDIR)$(PREFIX)/man/man1/3relind.1
	$Q rm -f $(DESTDIR)$(PREFIX)/etc/modbus_rs485_settings.txt
