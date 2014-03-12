#
# eudc : Makefile
#
#    Copyright (c) 2010-2014, NIPPON TELEGRAPH AND TELEPHONE CORPORATION
#

MODULE_big = eudc
PG_CPPFLAGS = -I$(libpq_srcdir) -L$(libdir)
OBJS = eudc.o utf8_and_sjis_eudc.o utf8_and_euc_eudc.o show_eudc.o
REGRESS = init conv copy fallback

EXTENSION = lib/eudc lib/eudc_drop

#supports both EXTENSION (for >=9.1) and without_EXTENSION (for <PG 9.1)
DATA_built = eudc.sql
DATA = lib/eudc--1.0.sql lib/eudc--unpackaged--1.0.sql \
uninstall_eudc.sql lib/eudc_drop--1.0.sql

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

REGRESS_OPTS += $(if $(filter 8.4 9.0, $(MAJORVERSION)), --multibyte=UTF8, --encoding=UTF8)

expected/conv.out: expected/conv-$(MAJORVERSION).out
	cp expected/conv-$(MAJORVERSION).out expected/conv.out

expected/conv-8.4.out:
	cp expected/conv-legacy.out expected/conv-8.4.out
expected/conv-9.0.out:
	cp expected/conv-legacy.out expected/conv-9.0.out
expected/conv-9.1.out:
	cp expected/conv-legacy.out expected/conv-9.1.out
expected/conv-9.2.out:
	cp expected/conv-extension.out expected/conv-9.2.out
expected/conv-9.3.out:
	cp expected/conv-extension.out expected/conv-9.3.out

.PHONY: subclean
clean: subclean

subclean:
	rm -f expected/conv.out expected/conv-{8.4,9.0,9.1,9.2,9.3}.out

installcheck: expected/conv.out
