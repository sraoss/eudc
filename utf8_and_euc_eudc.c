/*
 * utf8_and_euc_eudc.c: EUC_JP EUDC <--> UTF8
 *
 *	  Copyright (c) 2010, NIPPON TELEGRAPH AND TELEPHONE CORPORATION
 *
 * EUC_JP : F5A1 - FEFE, 8FF5A1 - 8FFEFE
 * UTF8   : E000 - E3AB, E3AC - E757
 */
#include "postgres.h"
#include "fmgr.h"
#include "mb/pg_wchar.h"
#include "eudc.h"
#include "pgut/pgut-be.h"

static PGFunction euc_jp_to_utf8 = NULL;
static PGFunction utf8_to_euc_jp = NULL;

PGDLLEXPORT PG_FUNCTION_INFO_V1(euc_jp_eudc_to_utf8);
PGDLLEXPORT PG_FUNCTION_INFO_V1(utf8_to_euc_jp_eudc);

static inline int
pg_euc_mblen(const unsigned char *s)
{
	int			len;

	if (*s == SS2)
		len = 2;
	else if (*s == SS3)
		len = 3;
	else if (IS_HIGHBIT_SET(*s))
		len = 2;
	else
		len = 1;
	return len;
}

/* ----------
 * conv_proc(
 *		INTEGER,	-- source encoding id
 *		INTEGER,	-- destination encoding id
 *		CSTRING,	-- source string (null terminated C string)
 *		CSTRING,	-- destination string (null terminated C string)
 *		INTEGER,	-- source string length
 *		boolean     -- ignore failure (only in V14 and newer)
 * ) returns int;   -- source string length (void on V13 and older)
 * ----------
 */
Datum
euc_jp_eudc_to_utf8(PG_FUNCTION_ARGS)
{
	unsigned char *src = (unsigned char *) PG_GETARG_CSTRING(2);
	unsigned char *dest = (unsigned char *) PG_GETARG_CSTRING(3);
	unsigned char *p;
	unsigned char *fallback_character = NULL;
	int			len = PG_GETARG_INT32(4);
	int			euc_jp_len;
	int			clen;
#if PG_VERSION_NUM >= 140000
	bool		ignorefail = PG_GETARG_BOOL(5);
	int 		converted;
#endif

	CHECK_ENCODING_CONVERSION_ARGS(PG_EUC_JP, PG_UTF8);

	if (euc_jp_to_utf8 == NULL)
		euc_jp_to_utf8 = load_external_function(
			"utf8_and_euc_jp", "euc_jp_to_utf8", true, NULL);

	*dest = '\0';
	p = src;
	euc_jp_len = 0;
	for (; len > 0; len -= clen)
	{
		const unsigned char *c = p + euc_jp_len;

		if (c[0] == '\0')
			report_invalid_encoding(PG_EUC_JP, (const char *) c, len);

		clen = pg_euc_mblen(c);

		/* invalid code check for EUC_JP including eudc region */
		if ((clen == 2 && 0xf5 <= c[0] && c[0] <= 0xfe &&
				(c[1] < 0xa1 || 0xfe < c[1])) ||
			(clen == 3 && c[0] == 0x8f && 0xf5 <= c[1] && c[1] <= 0xfe &&
				(c[2] < 0xa1 || 0xfe < c[2])) ||
			(clen == 2 && c[0] == 0x8e && (c[1] < 0xa1 || 0xdf < c[1])) ||
            (clen == 2 && 0xa1 <= c[0]  && c[0] <= 0xa8 &&
				(c[1] < 0xa1 || 0xfe < c[1])) ||
            (clen == 2 && c[0] == 0xad && (c[1] < 0xa1 || 0xfe < c[1])) ||
            (clen == 2 && 0xb0 <= c[0] && c[0] <= 0xf4 &&
				(c[1] < 0xa1 || 0xfe < c[1])) ||
            (clen == 2 && 0xf9 <= c[0] && c[0] <= 0xfc &&
				(c[1] < 0xa1 || 0xfe < c[1])) ||
			(clen == 3 && c[0] == 0x8f && (! ((0xa1 <= c[1] && c[1] <= 0xab) ||
				(0xb0 <= c[1] && c[1] <= 0xed) || (0xf3 <= c[1] && c[1] <= 0xfe)))) ||
			(clen == 3 && c[0] == 0x8f && ((0xa1 <= c[1] && c[1] <= 0xab) ||
				(0xb0 <= c[1] && c[1] <= 0xed) || (0xf3 <= c[1] && c[1] <= 0xfe)) &&
				(c[2] < 0xa1 || c[2] > 0xfe)))
		{
			report_invalid_encoding(PG_EUC_JP, (const char *) c, len);
		}

		if ((clen == 2 &&
			 0xf5 <= c[0] && c[0] <= 0xfe &&
			 0xa1 <= c[1] && c[1] <= 0xfe) ||
			(clen == 3 && c[0] == 0x8f &&
			 0xf5 <= c[1] && c[1] <= 0xfe &&
			 0xa1 <= c[2] && c[2] <= 0xfe))
		{
			int	ucs;
			int	m;
			int	n;

			/* EUC_JP to UTF8 */
			if (euc_jp_len > 0)
			{
#if PG_VERSION_NUM >= 140000
				DirectFunctionCall6(euc_jp_to_utf8, PG_EUC_JP, PG_UTF8,
									CStringGetDatum(p), CStringGetDatum(dest),
									euc_jp_len, ignorefail);
#else
				DirectFunctionCall5(euc_jp_to_utf8, PG_EUC_JP, PG_UTF8,
									CStringGetDatum(p), CStringGetDatum(dest),
									euc_jp_len);
#endif
				dest = dest + strlen((char *) dest);
				p += euc_jp_len;
				euc_jp_len = 0;
			}
			p += clen;

			if (clen == 2)
				elog(eudc_log_level,
					"eudc character found: %02x%02x in EUC_JP to UTF8 conversion",
					c[0], c[1]);
			else
				elog(eudc_log_level,
					"eudc character found: %02x%02x%02x in EUC_JP to UTF8 conversion",
					c[0], c[1], c[2]);

			/* EUC_JP EUDC to UTF8 */
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
				if (len == 3)
				{
					ucs = 0xe3ac;
					c++;
				}
				else
					ucs = 0xe000;

				n = c[0] - 0xf5;
				m = c[1] - 0xa1;
				ucs += n * 94 + m;

				*dest++ = (ucs >> 12) | 0xe0;
				*dest++ = (ucs & 0x0fc0) >> 6 | 0x80;
				*dest++ = (ucs & 0x003f) | 0x80;
			}
			*dest = '\0';
		}
		else
			euc_jp_len += clen;
	}

	/* EUC_JP to UTF8 */
	if (euc_jp_len > 0)
#if PG_VERSION_NUM >= 140000
		DirectFunctionCall6(euc_jp_to_utf8, PG_EUC_JP, PG_UTF8,
							CStringGetDatum(p), CStringGetDatum(dest),
							euc_jp_len, ignorefail);
#else
		DirectFunctionCall5(euc_jp_to_utf8, PG_EUC_JP, PG_UTF8,
							CStringGetDatum(p), CStringGetDatum(dest),
							euc_jp_len);
#endif

	if (fallback_character != NULL &&
		fallback_character != (unsigned char*) eudc_fallback_character)
		pfree(fallback_character);

#if PG_VERSION_NUM >= 140000
	converted = p - src + euc_jp_len;
	PG_RETURN_INT32(converted);
#else
	PG_RETURN_VOID();
#endif
}

Datum
utf8_to_euc_jp_eudc(PG_FUNCTION_ARGS)
{
	unsigned char *src = (unsigned char *) PG_GETARG_CSTRING(2);
	unsigned char *dest = (unsigned char *) PG_GETARG_CSTRING(3);
	unsigned char *p;
	unsigned char *fallback_character = NULL;
	int			len = PG_GETARG_INT32(4);
	int			utf8_len;
	int			clen;
#if PG_VERSION_NUM >= 140000
	bool		ignorefail = PG_GETARG_BOOL(5);
	int 		converted;
#endif


	CHECK_ENCODING_CONVERSION_ARGS(PG_UTF8, PG_EUC_JP);

	if (utf8_to_euc_jp == NULL)
		utf8_to_euc_jp = load_external_function(
			"utf8_and_euc_jp", "utf8_to_euc_jp", true, NULL);

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

			/* UTF8 to EUC_JP */
			if (utf8_len > 0)
			{
#if PG_VERSION_NUM >= 140000
				DirectFunctionCall6(utf8_to_euc_jp, PG_UTF8, PG_EUC_JP,
									CStringGetDatum(p), CStringGetDatum(dest),
									utf8_len, ignorefail);
#else
				DirectFunctionCall5(utf8_to_euc_jp, PG_UTF8, PG_EUC_JP,
									CStringGetDatum(p), CStringGetDatum(dest),
									utf8_len);
#endif
				dest = dest + strlen((char *) dest);
				p += utf8_len;
				utf8_len = 0;
			}
			p += clen;

			elog(eudc_log_level,
				"eudc character found: %02x%02x%02x in UTF8 to EUC_JP conversion",
				c[0], c[1], c[2]);

			/* UTF8 to EUC_JP EUDC */
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
						PG_EUC_JP);
				}

				for (i = 0; fallback_character[i]; i++)
					*dest++ = fallback_character[i];
			}
			else
			{
				/* linear mapping */
				if (ucs < 0xe3ac)
				{
					m = (ucs - 0xE000) % 94;
					n = (ucs - 0xE000) / 94;

 					/* euc_jp = 0xF5A1 + n * 0x100 + m */
					*dest++ = 0xf5 + n;
					*dest++ = 0xa1 + m;
				}
				else
				{
					m = (ucs - 0xE3AC) % 94;
					n = (ucs - 0xE3AC) / 94;

					/* euc_jp = 0x8FF5A1 + n * 0x100 + m */
					*dest++ = 0x8f;
					*dest++ = 0xf5 + n;
					*dest++ = 0xa1 + m;
				}
			}
			*dest = '\0';
		}
		else
			utf8_len += clen;
	}

	if (len > 0)
		report_invalid_encoding(PG_UTF8, (const char *) p + utf8_len, len);

	/* UTF8 to EUC_JP */
	if (utf8_len > 0)
#if PG_VERSION_NUM >= 140000
		DirectFunctionCall6(utf8_to_euc_jp, PG_UTF8, PG_EUC_JP,
							CStringGetDatum(p), CStringGetDatum(dest),
							utf8_len, ignorefail);
#else
		DirectFunctionCall5(utf8_to_euc_jp, PG_UTF8, PG_EUC_JP,
							CStringGetDatum(p), CStringGetDatum(dest),
							utf8_len);
#endif

	if (fallback_character != NULL &&
		fallback_character != (unsigned char*) eudc_fallback_character)
		pfree(fallback_character);

#if PG_VERSION_NUM >= 140000
	converted = p - src + utf8_len;
	PG_RETURN_INT32(converted);
#else
	PG_RETURN_VOID();
#endif
}
