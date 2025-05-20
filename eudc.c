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

#if PG_VERSION_NUM >= 90100
char   *eudc_fallback_character = "";
#else
char   *eudc_fallback_character = NULL;
#endif

int		eudc_log_level = DEBUG2;

extern void PGDLLEXPORT _PG_init(void);

#if PG_VERSION_NUM >= 140000
static bool eudc_fallback_character_check_hook(
	const char **newval, void** extra, GucSource source);

static void eudc_fallback_character_assign_hook(
	const char *newval, void* extra);


#elif PG_VERSION_NUM >= 90100
static bool eudc_fallback_character_check_hook(
	const char **newval, void** extra, GucSource source);

static const char *eudc_fallback_character_assign_hook(
	const char *newval, void* extra);

#else

static const char *eudc_fallback_character_assign_hook(
	const char *newval, bool doit, GucSource source);

#endif


void
_PG_init(void)
{
	/* Define custom GUC variables. */
	DefineCustomStringVariable("eudc.fallback_character",
							   "Character used for EUDC. Or, use linear mapping when empty.",
							   NULL,
							   &eudc_fallback_character,
							   "",
							   PGC_USERSET,
							   0,
#if PG_VERSION_NUM >= 90100
							   (GucStringCheckHook)&eudc_fallback_character_check_hook,
							   (GucStringAssignHook)&eudc_fallback_character_assign_hook,
#else
							   eudc_fallback_character_assign_hook,
#endif
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
#if PG_VERSION_NUM >= 90100
							 NULL,
#endif
							 NULL,
							 NULL);
#endif

	EmitWarningsOnPlaceholders("eudc");
}



#if PG_VERSION_NUM >= 90100

#if PG_VERSION_NUM >= 140000
static void
eudc_fallback_character_assign_hook(
	const char *newval, void* extra)
{
	return;
}
#else
static const char *
eudc_fallback_character_assign_hook(
	const char *newval, void* extra)
{
	return newval;
}
#endif

static bool eudc_fallback_character_check_hook(
	const char **newval, void** extra, GucSource source)
{
	int		len;

	GUC_check_errhint("must be one character or empty string");

	/* handle explicit setting of eudc.fallback_character */
	if (*newval)
	{
		/* eudc.fallback_character
		 * should be 0 or 1 character
		 */
		len = pg_mbstrlen(*newval);
		if (len >= 2 || len < 0)
			return false;
	}

	return true;
}

#else

static const char *
eudc_fallback_character_assign_hook(
	const char *newval, bool doit, GucSource source)
{
	int		len = pg_mbstrlen(newval);

	if (len >= 2 || len < 0)
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			 errmsg("invalid value for parameter \"eudc.fallback_character\": \"%s\"", newval),
			 errhint("must be one character or empty string")));

	return newval;
}

#endif
