always_built_SUBDIRS = src macros

sometimes_built_SUBDIRS = 

SUBDIRS = $(always_built_SUBDIRS)

EXTRA_DIST = INSTALL README.polish COPYING KOPIOWANIE AUTHORS README NEWS ChangeLog TODO

FTP_USER = siewca
FTP_HOST = team.pld.org.pl
FTP_DIR = /home/ftp/software/$(PACKAGE)

## to automatically rebuild aclocal.m4 if any of the macros in `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep

## make rpms
rpm: Makefile
	$(MAKE) dist
	rpm -ta --clean $(PACKAGE)-$(VERSION).tar.gz

## publish dist on FTP
publish: Makefile
	$(MAKE) dist
	scp $(PACKAGE)-$(VERSION).tar.gz $(EXTRA_DIST) $(FTP_USER)@$(FTP_HOST):$(FTP_DIR)
	ssh $(FTP_USER)@$(FTP_HOST) "cd $(FTP_DIR) && chmod 0644 $(PACKAGE)-$(VERSION).tar.gz $(EXTRA_DIST)"
