rootlibdir		= @rootlibdir@
rootsysconfdir	= /etc
rootbindir		= /bin
rootsbindir		= /sbin
rootscriptdir	= /lib
rootdatadir		= /lib
scimdatadir		= $(rootdatadir)/scim
scimscriptdir	= $(rootscriptdir)/scim

lib_LTLIBRARIES		 = libscim.la

scimscript_SCRIPTS	= rdinit

sysconf_DATA		= inittab

rootsbin_PROGRAMS	= shutd srvc srvcd

rootsbin_SCRIPTS	= mkinitrd

noinst_PROGRAMS		= test

nobase_include_HEADERS = \
	scim/api.h \
	scim/code.h \
	scim/data.h \
	scim/netlink.h \
	scim/scim.h

nodist_include_HEADERS = \
	scim/api.h \
	scim/code.h \
	scim/data.h

BUILT_SOURCES = \
	scim/api.h \
	scim/code.h \
	scim/data.h

srvc_SOURCES = \
	srvc.c

srvcd_SOURCES = \
	srvcd.c

libscim_la_netlink_sources = \
	scim_netlink.c \
	scim_netlink_address.c \
	scim_netlink_link.c

libscim_la_sources = \
	scim_data.c \
	scim_port.c \
	scim_site.c \
	scim_cell.c \
	scim_cpio.c \
	scim_crew.c \
	scim_host.c \
	scim_note.c \
	scim_root.c \
	scim_task.c \
	scim_team.c \
	scim_term.c \
	scim_tool.c

libscim_la_SOURCES = \
	$(nobase_include_HEADERS) \
	$(libscim_la_netlink_sources) \
	$(libscim_la_sources)

libscim_la_LDFLAGS = -larchive

scim_data = \
	aide.txt \
	crew.txt \
	host.txt \
	port.txt \
	task.txt \
	ward.txt

LDADD = libscim.la

AM_CFLAGS = -O3 -g -Wall -Winline -finline-functions -finline-small-functions -D_REENTRANT -std=gnu99

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-DHOST_CPU=\"$(host_cpu)\" \
	-DPREFIX=\"$(prefix)/\" \
	-DEXEC_PREFIX=\"$(exec_prefix)/\" \
	-DLIBDIR=\"$(libdir)/\" \
	-DLIBEXECDIR=\"$(libexecdir)/\" \
	-DBINDIR=\"$(bindir)/\" \
	-DSBINDIR=\"$(sbindir)/\" \
	-DLOCALSTATEDIR=\"$(localstatedir)/\" \
	-DROOTSYSCONFDIR=\"$(rootsysconfdir)/\" \
	-DROOTBINDIR=\"$(rootbindir)/\" \
	-DROOTLIBDIR=\"$(rootlibdir)/\" \
	-DROOTSBINDIR=\"$(rootsbindir)/\" \
	-DSCIMDATADIR=\"$(scimdatadir)/\" \
	-DSCIMSCRIPTDIR=\"$(scimscriptdir)/\"

AM_LDFLAGS = -lpthread

DISTCLEANFILES = \
	mkinitrd \
	rdinit

EXTRA_DIST = \
	api.sh \
	data.sh \
	inittab \
	mkinitrd.sh \
	rdinit.sh \
	$(scim_data)

scim/api.h: $(libscim_la_sources)
	$(SHELL) $(srcdir)/api.sh $(srcdir)

scim/code.h scim/data.h: $(scim_data)
	$(SHELL) $(srcdir)/data.sh $(srcdir)

%: %.sh
	sed \
		-e "s%@PACKAGE\@%$(PACKAGE)%g" \
		-e "s%@VERSION\@%$(VERSION)%g" \
		-e "s%@DISTRIBUTION\@%$(DISTRIBUTION)%g" \
		-e "s%@host\@%$(host)%g" \
		-e "s%@prefix\@%$(prefix)%g" \
		-e "s%@exec_prefix\@%$(exec_prefix)%g" \
		-e "s%@bindir\@%$(bindir)%g" \
		-e "s%@sbindir\@%$(sbindir)%g" \
		-e "s%@libexecdir\@%$(libexecdir)%g" \
		-e "s%@libdir\@%$(libdir)%g" \
		-e "s%@includedir\@%$(includedir)%g" \
		-e "s%@datadir\@%$(datadir)%g" \
		-e "s%@localstatedir\@%$(localstatedir)%g" \
		-e "s%@sysconfdir\@%$(sysconfdir)%g" \
		-e "s%@pkglibdir\@%$(sysconfdir)%g" \
		-e "s%@pkgdatadir\@%$(pkgdatadir)%g" $< > $@.tmp
	rm -f $@
	mv $@.tmp $@

