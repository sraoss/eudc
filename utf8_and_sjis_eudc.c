/*
 * utf8_and_sjis_eudc.c: SJIS EUDC <--> UTF8
 *
 *	  Copyright (c) 2010, NIPPON TELEGRAPH AND TELEPHONE CORPORATION
 *
 * SJIS : F040 - F9FF
 * UTF8 : E000 - E757
 */
#include "postgres.h"
#include "fmgr.h"
#include "mb/pg_wchar.h"
#include "eudc.h"
#include "pgut/pgut-be.h"

static PGFunction sjis_to_utf8 = NULL;
static PGFunction utf8_to_sjis = NULL;

PG_FUNCTION_INFO_V1(sjis_eudc_to_utf8);
PG_FUNCTION_INFO_V1(utf8_to_sjis_eudc);

extern Datum PGDLLEXPORT sjis_eudc_to_utf8(PG_FUNCTION_ARGS);
extern Datum PGDLLEXPORT utf8_to_sjis_eudc(PG_FUNCTION_ARGS);

/* ----------
 * conv_proc(
 *		INTEGER,	-- source encoding id
 *		INTEGER,	-- destination encoding id
 *		CSTRING,	-- source string (null terminated C string)
 *		CSTRING,	-- destination string (null terminated C string)
 *		INTEGER		-- source string length
 * ) returns VOID;
 * ----------
 */
Datum
sjis_eudc_to_utf8(PG_FUNCTION_ARGS)
{
	unsigned char *src = (unsigned char *) PG_GETARG_CSTRING(2);
	unsigned char *dest = (unsigned char *) PG_GETARG_CSTRING(3);
	unsigned char *p;
	unsigned char *fallback_character = NULL;
	int			len = PG_GETARG_INT32(4);
	int			sjis_len;
	int			clen;

	CHECK_ENCODING_CONVERSION_ARGS(PG_SJIS, PG_UTF8);

	if (sjis_to_utf8 == NULL)
		sjis_to_utf8 = load_external_function(
			"utf8_and_sjis", "sjis_to_utf8", true, NULL);

	*dest = '\0';
	p = src;
	sjis_len = 0;
	for (; len > 0; len -= clen)
	{
		const unsigned char *c = p + sjis_len;

		if (c[0] == '\0')
			report_invalid_encoding(PG_SJIS, (const char *) p + sjis_len, len);

		if (c[0] >= 0xf0 && c[0] <= 0xf9 && len >= 2 && ISSJISTAIL(c[1]))
		{
			int	ucs;
			int	m;
			int	n;

			clen = 2;

			/* SJIS to UTF8 */
			if (sjis_len > 0)
			{
				DirectFunctionCall5(sjis_to_utf8, PG_SJIS, PG_UTF8,
									CStringGetDatum(p), CStringGetDatum(dest),
									sjis_len);
				dest = dest + strlen((char *) dest);
				p += sjis_len;
				sjis_len = 0;
			}
			p += clen;

			elog(eudc_log_level,
				"eudc character found: %02x%02x in SJIS to UTF8 conversion",
				c[0], c[1]);

			/* SJIS EUDC to UTF8 */
			if (eudc_fallback_character && eudc_fallback_character[0])
			{
				/* map to fallback character */
				int		i;

				if (fallback_character == NULL)
				{
					fallback_character = pg_do_encoding_conversion(
						(unsigned char *) eudc_fallback_character,
						strlen(eudc_fallback_character),
						GetDatabaseEncoding(),
						PG_UTF8);
				}

				for (i = 0; fallback_character[i]; i++)
					*dest++ = fallback_character[i];
			}
			else
			{
				/* linear mapping */
				n = c[0] - 0xf0;
				m = c[1] - 0x40;

				if (m >= 0x40)
					m--;

				ucs = 0xe000 + n * 188 + m;

				*dest++ = (ucs >> 12) | 0xe0;
				*dest++ = (ucs & 0x0fc0) >> 6 | 0x80;
				*dest++ = (ucs & 0x003f) | 0x80;
			}
			*dest = '\0';
		}
		else
		{
			clen = (ISSJISHEAD(c[0]) ? 2 : 1);
			sjis_len += clen;
		}
	}

	/* SJIS to UTF8 */
	if (sjis_len > 0)
		DirectFunctionCall5(sjis_to_utf8, PG_SJIS, PG_UTF8,
							CStringGetDatum(p), CStringGetDatum(dest),
							sjis_len);

	if (fallback_character != NULL &&
		fallback_character != (unsigned char*) eudc_fallback_character)
		pfree(fallback_character);

	PG_RETURN_VOID();
}

Datum
utf8_to_sjis_eudc(PG_FUNCTION_ARGS)
{
	unsigned char *src = (unsigned char *) PG_GETARG_CSTRING(2);
	unsigned char *dest = (unsigned char *) PG_GETARG_CSTRING(3);
	unsigned char *p;
	unsigned char *fallback_character = NULL;
	int			len = PG_GETARG_INT32(4);
	int			utf8_len;
	int			clen;

	CHECK_ENCODING_CONVERSION_ARGS(PG_UTF8, PG_SJIS);

	if (utf8_to_sjis == NULL)
		utf8_to_sjis = load_external_function(
			"utf8_and_sjis", "utf8_to_sjis", true, NULL);

	*dest = '\0';
	p = src;
	utf8_len = 0;
	for (; len > 0; len -= clen)
	{
		const unsigned char *c = p + utf8_len;
		int ucs;

		if (c[0] == '\0')
			break;

		clen = pg_utf_mblen(p + utf8_len);

		if (len < clen)
			break;

		if (clen != 3)
		{
			utf8_len += clen;
			continue;
		}

		ucs = (((int) c[0] & 0x0f) << 12) |
			  (((int) c[1] & 0x3f) << 6) |
			  ((int) c[2] & 0x3f);
		if (ucs >= 0xe000 && ucs <= 0xe757)
		{
			int	m;
			int	n;

			/* UTF8 to SJIS */
			if (utf8_len > 0)
			{
				DirectFunctionCall5(utf8_to_sjis, PG_UTF8, PG_SJIS,
									CStringGetDatum(p), CStringGetDatum(dest),
									utf8_len);
				dest = dest + strlen((char *) dest);
				p += utf8_len;
				utf8_len = 0;
			}
			p += clen;

			elog(eudc_log_level,
				"eudc character found: %02x%02x%02x in UTF8 to SJIS conversion",
				c[0], c[1], c[2]);

			/* UTF8 to SJIS EUDC */
			if (eudc_fallback_character && eudc_fallback_character[0])
			{
				/* map to fallback character */
				int		i;

				if (fallback_character == NULL)
				{
					fallback_character = pg_do_encoding_conversion(
						(unsigned char *) eudc_fallback_character,
						strlen(eudc_fallback_character),
						GetDatabaseEncoding(),
						PG_SJIS);
				}

				for (i = 0; fallback_character[i]; i++)
					*dest++ = fallback_character[i];
			}
			else
			{
				/* linear mapping */
				m = (ucs - 0xE000) % 188;
				n = (ucs - 0xE000) / 188;

				if (m >= 63)
					m++;

				/* sjis = 0xF040 + n * 0x100 + m */
				*dest++ = 0xf0 + n;
				*dest++ = 0x40 + m;
			}
			*dest = '\0';
		}
		else
			utf8_len += clen;
	}

	if (len > 0)
		report_invalid_encoding(PG_UTF8, (const char *) p + utf8_len, len);

	/* UTF8 to SJIS */
	if (utf8_len > 0)
		DirectFunctionCall5(utf8_to_sjis, PG_UTF8, PG_SJIS,
							CStringGetDatum(p), CStringGetDatum(dest),
							utf8_len);

	if (fallback_character != NULL &&
		fallback_character != (unsigned char*) eudc_fallback_character)
		pfree(fallback_character);

	PG_RETURN_VOID();
}
