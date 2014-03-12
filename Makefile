# eudc: Makefile

MODULE_big = eudc
PG_CPPFLAGS = -I$(libpq_srcdir) -L$(libdir)
OBJS = eudc.o utf8_and_sjis_eudc.o utf8_and_euc_eudc.o
DATA_built = eudc.sql
DATA = uninstall_eudc.sql
REGRESS = init conv copy fallback

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
