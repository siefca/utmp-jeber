always_built_SUBDIRS = src macros
sometimes_built_SUBDIRS = 
SUBDIRS		= $(always_built_SUBDIRS)

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

WEB_LOCALDIR	= www
WEB_DIST	= index.html README README.polish $(TAR_GZ) $(SRC_RPM)

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
ftp-publish: Makefile rpm
	@echo Publishing FTP Directory..
	@scp $(TAR_GZ) $(HOME)/rpm/SRPMS/$(SRC_RPM) $(FTP_DIST) $(FTP_USER)@$(FTP_HOST):$(FTP_DIR)
	@ssh $(FTP_USER)@$(FTP_HOST) "cd $(FTP_DIR) && rm -f CURRENT_IS* *-current.* ; touch CURRENT_IS_$(VERSION) ; ln -s $(TAR_GZ) $(CUR_GZ) ; ln -s $(SRC_RPM) $(CUR_SRPM)"
	@ssh $(FTP_USER)@$(FTP_HOST) "cd $(FTP_DIR) && chmod 0644 $(TAR_GZ) $(FTP_DIST) $(SRC_RPM) CURRENT_IS*"

## fetch FTP mirros list and format it
mirrors: Makefile
	@echo fetching mirror list
	@lynx -dump -nolist http://www.pld.org.pl/distrib/ftp/ | grep "ftp://" | tr '*' -s ' ' \
	| awk '{print "<LI> <A HREF=\""$$1"software/$(PACKAGE)/\">"$$1"software/$(PACKAGE)/<\A>"}' > mirror-list

## WWW
www: Makefile
	@cp html-template $(WEB_LOCALDIR)/index.html
	@cp README* $(WEB_LOCALDIR)
	@cp $(TAR_GZ) $(WEB_LOCALDIR)
	@cp $(HOME)/rpm/SRPMS/$(SRC_RPM) $(WEB_LOCALDIR)

## publish dist on WWW
www-publish: Makefile www
	@echo Publishing Web Page..
	@cd $(WEB_LOCALDIR) && scp $(WEB_DIST) $(WWW_USER)@$(WWW_HOST):$(WWW_DIR)
	@ssh $(WWW_USER)@$(WWW_HOST) "cd $(WWW_DIR) && chmod 0644 *"

## publish both, on WWW and FTP
publish: Makefile ftp-publish www-publish
