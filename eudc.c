/*
 * eudc.c
 */
#include "postgres.h"
#include "mb/pg_wchar.h"
#include "utils/guc.h"
#include "pgut/pgut-be.h"

PG_MODULE_MAGIC;

static const struct config_enum_entry log_level_options[] = {
	{"debug", DEBUG2, true},
	{"debug5", DEBUG5, false},
	{"debug4", DEBUG4, false},
	{"debug3", DEBUG3, false},
	{"debug2", DEBUG2, false},
	{"debug1", DEBUG1, false},
	{"log", LOG, false},
	{"info", INFO, true},
	{"notice", NOTICE, false},
	{"warning", WARNING, false},
	{"error", ERROR, false},
	{"fatal", FATAL, true},
	/* should not have PANIC here! */
	{NULL, 0, false}
};

char   *eudc_fallback_character = NULL;
int		eudc_log_level = DEBUG2;

extern void PGDLLEXPORT _PG_init(void);

static const char *eudc_fallback_character_assign_hook(
	const char *newval, bool doit, GucSource source);

void
_PG_init(void)
{
	/* Define custom GUC variables. */
	DefineCustomStringVariable("eudc.fallback_character",
							   "Character used for EUDC. Or, use linear mapping when empty.",
							   NULL,
							   &eudc_fallback_character,
							   NULL,
							   PGC_USERSET,
							   0,
							   eudc_fallback_character_assign_hook,
							   NULL);

#if PG_VERSION_NUM >= 80400
	DefineCustomEnumVariable("eudc.log_level",
							 "Level to log EUDC characters.",
							 NULL,
							 &eudc_log_level,
							 DEBUG2,
							 log_level_options,
							 PGC_USERSET,
							 0,
							 NULL,
							 NULL);
#endif

	EmitWarningsOnPlaceholders("eudc");
}

static const char *
eudc_fallback_character_assign_hook(
	const char *newval, bool doit, GucSource source)
{
	int		len = pg_mbstrlen(newval);

	if (len >= 2)
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			 errmsg("invalid value for parameter \"eudc.fallback_character\": \"%s\"", newval),
			 errhint("must be one character or empty string")));

	return newval;
}
