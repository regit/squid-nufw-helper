SQUIDDIR=$(sysconfdir)/squid
SQUID_GROUP=@SQUID_GROUP@

all: squid_nufw_helper

bin_PROGRAMS = squid_nufw_helper

EXTRA_DIST = squid_nufw_helper.conf

squid_nufw_helper_SOURCES=main.c sql.c conf.c auth_nufw.h

squid_nufw_helper: $(squid_nufw_helper_SOURCES) config.h
	$(LINK) $(squid_nufw_helper_SOURCES) $(squid_nufw_helper_LDFLAGS) $(LIBS)

clean:
	rm -f squid_nufw_helper

distclean: clean
	rm -f Makefile config.status config.cache config.log

install-data-am:
	install -d $(SQUIDDIR)
	test -e $(SQUIDDIR)/squid_nufw_helper.conf || install -o root -g $(SQUID_GROUP) -m 640 squid_nufw_helper.conf $(SQUIDDIR)

