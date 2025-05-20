#
# eudc : Makefile
#
#    Copyright (c) 2010-2014, NIPPON TELEGRAPH AND TELEPHONE CORPORATION
#    Copyright (c) 2023, SRA OSS LLC
#

MODULE_big = eudc
PG_CPPFLAGS = -I$(libpq_srcdir) -L$(libdir)
OBJS = eudc.o utf8_and_sjis_eudc.o utf8_and_euc_eudc.o show_eudc.o
REGRESS = init conv copy fallback

EXTENSION = lib/eudc

#supports both EXTENSION and without_EXTENSION
DATA_built = eudc.sql lib/eudc--2.0.sql

ifndef USE_PGXS
top_builddir = ../..
makefile_global = $(top_builddir)/src/Makefile.global

ifeq "$(wildcard $(makefile_global) )" ""
USE_PGXS = 1	# use pgxs if not in contrib directory
endif

endif

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/$(MODULE_big)
include $(makefile_global)
include $(top_srcdir)/contrib/contrib-global.mk
endif

# remove dependency to libxml2 and libxslt
LIBS := $(filter-out -lxml2, $(LIBS))
LIBS := $(filter-out -lxslt, $(LIBS))

ifndef MAJORVERSION
MAJORVERSION := $(basename $(VERSION))
endif

lib/eudc--2.0.sql: lib/eudc--2.0.sql.in
	(test $(MAJORVERSION) -lt 14 && cp lib/eudc--2.0.sql.pg13.in lib/eudc--2.0.sql) || \
	(test $(MAJORVERSION) -ge 14 && cp lib/eudc--2.0.sql.in lib/eudc--2.0.sql)

eudc.sql: eudc.sql.in
	(test $(MAJORVERSION) -lt 14 && cp eudc.sql.pg13.in eudc.sql) || \
	(test $(MAJORVERSION) -ge 14 && cp eudc.sql.in eudc.sql)

expected/conv.out: expected/conv-$(MAJORVERSION).out
	cp expected/conv-$(MAJORVERSION).out expected/conv.out

expected/conv-11.out:
	cp expected/conv-extension.out expected/conv-11.out

expected/conv-12.out:
	cp expected/conv-extension.out expected/conv-12.out

expected/conv-13.out:
	cp expected/conv-extension.out expected/conv-13.out

expected/conv-14.out:
	cp expected/conv-extension.out expected/conv-14.out

expected/conv-15.out:
	cp expected/conv-extension.out expected/conv-15.out

expected/conv-16.out:
	cp expected/conv-extension.out expected/conv-16.out

expected/conv-17.out:
	cp expected/conv-extension.out expected/conv-17.out

expected/conv-18.out:
	cp expected/conv-extension.out expected/conv-18.out

.PHONY: subclean
clean: subclean

subclean:
	rm -f expected/conv.out expected/conv-{11,12,13,14,15,16,17,18}.out

installcheck: expected/conv.out

