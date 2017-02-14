# tddec/Makefile

prefix = /usr/local
bindir = $(prefix)/bin
mandir = $(prefix)/man

distdir = dist

#CROSS  = i686-mingw32-
#EXEEXT = .exe

CC      = $(CROSS)gcc
CFLAGS  = -Os -Wall

LD      = $(CROSS)gcc
LDFLAGS =

STRIP     = $(CROSS)strip
STRIP_OPT = -p

BIN := tddec$(EXEEXT)

all: $(BIN)

clean:
	rm -f $(BIN) tddec.o lh1.o lh1-dec.o lh1-enc.o

install: all
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(mandir)/man1
	for f in $(BIN) ; do \
		install -m 755 "$$f" "$(DESTDIR)$(bindir)/$$f" ; \
	done
	for f in $(MAN1) ; do \
		install -m 644 "$$f" "$(DESTDIR)$(mandir)/man1/$$f" ; \
	done


dist: all
	mkdir -p $(distdir)
	for f in $(BIN) ; do \
		install -m 755 "$$f" "$(distdir)/$$f" ; \
		$(STRIP) $(STRIP_OPT) "$(distdir)/$$f" ; \
	done
	for f in $(MAN1) ; do \
		install -m 644 "$$f" "$(distdir)/$$f" ; \
	done

tddec.o: tddec.c tddec.h
	$(CC) $(CFLAGS) -c -o $@ $<

tddec$(EXEEXT): tddec.o
	$(LD) $(LDFLAGS) -o $@ tddec.o
