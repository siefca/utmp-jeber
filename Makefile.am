always_built_SUBDIRS = src macros
sometimes_built_SUBDIRS = 
SUBDIRS		= $(always_built_SUBDIRS)

WEB_LOCALDIR	= www
WEB_DIST	= $(PACKAGE).html $(PACKAGE).html.in
SPEC_DIST	= $(PACKAGE).spec $(PACKAGE).spec.in
FTP_DIST	= README README.polish COPYING KOPIOWANIE NEWS INSTALL
EXTRA_DIST	= $(FTP_DIST) $(SPEC_DIST) AUTHORS ChangeLog TODO CONFIGURE_OPTS

FTP_USER	= siewca
FTP_HOST	= team.pld.org.pl
FTP_DIR		= /home/ftp/software/$(PACKAGE)

WWW_USER	= siewca
WWW_HOST	= team.pld.org.pl
WWW_DIR		= /home/httpd/virtual/$(PACKAGE)

TAR_GZ		= $(PACKAGE)-$(VERSION).tar.gz
CUR_GZ		= $(PACKAGE)-current.tar.gz
SRC_RPM		= $(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm
CUR_SRPM	= $(PACKAGE)-current.src.rpm

CLEANFILES	= *~

## to automatically rebuild aclocal.m4 if any of the macros in `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep

## make rpms
rpm: Makefile
	$(MAKE) dist
	rpm -ta --clean $(TAR_GZ)

## publish dist on FTP
publish: Makefile
	$(MAKE) rpm
	scp $(TAR_GZ) $(HOME)/rpm/SRPMS/$(SRC_RPM) $(FTP_DIST) $(FTP_USER)@$(FTP_HOST):$(FTP_DIR)
	ssh $(FTP_USER)@$(FTP_HOST) "cd $(FTP_DIR) && rm -f CURRENT_IS* *-current.* ; touch CURRENT_IS_$(VERSION) ; ln -s $(TAR_GZ) $(CUR_GZ) ; ln -s $(SRC_RPM) $(CUR_SRPM)"
	ssh $(FTP_USER)@$(FTP_HOST) "cd $(FTP_DIR) && chmod 0644 $(TAR_GZ) $(FTP_DIST) $(SRC_RPM) CURRENT_IS*"

## WWW
www: Makefile
	$(MAKE) dist
	$(MAKE) rpm
	mv html-template $(WEB_LOCALDIR)/index.html
	cp $(TAR_GZ) $(WEB_LOCALDIR)
	cp $(HOME)/rpm/SRPMS/$(SRC_RPM) $(WEB_LOCALDIR)

## publish dist on WWW
www-publish: Makefile
	$(MAKE) www
	cd $(WEB_LOCALDIR) && scp *.png *.jpg *.html $(WWW_USER)@$(WWW_HOST):$(WWW_DIR)
	ssh $(WWW_USER)@$(WWW_HOST) "cd $(WWW_DIR) && chmod 0644 *"
