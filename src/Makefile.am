## Process this file with automake to produce Makefile.in

@SET_MAKE@

bin_PROGRAMS = utmp-jeber

utmp_jeber_COMMON_SRC =	defines.h \
	pgid.c  \
	pidlist.c  \
	proc.c  \
	settings.c  \
	utmp-jeber.c \
	includes.h  \
	pgid.h  \
	pidlist.h  \
	proc.h  \
	settings.h

utmp_jeber_SNPRINTF = ../shared/snprintf.c

if HAVE_SNPRINTF
utmp_jeber_SNPRINTF_OBJ =
else
utmp_jeber_SNPRINTF_OBJ = ../shared/snprintf.o
endif

utmp_jeber_SOURCES = $(utmp_jeber_COMMON_SRC)
utmp_jeber_LDADD = $(utmp_jeber_SNPRINTF_OBJ) $(INTLLIBS)
utmp_jeber_DEPENDENCIES = $(utmp_jeber_SNPRINTF_OBJ) $(INTLDEPS)

if HAVE_SNPRINTF
else
../shared/snprintf.o: $(utmp_jeber_SNPRINTF)
endif

EXTRA_DIST = $(utmp_jeber_SNPRINTF)

