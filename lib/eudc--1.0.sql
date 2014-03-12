-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION eudc" to load this file. \quit
\echo Use "CREATE EXTENSION eudc_uninstall" to drop this extension. \quit

DROP EXTENSION IF EXISTS eudc_drop;

-- SJIS_EUDC --> UTF8
CREATE FUNCTION pg_catalog.sjis_eudc_to_utf8 (integer, integer, cstring, internal, integer)
  RETURNS void AS '$libdir/eudc', 'sjis_eudc_to_utf8' LANGUAGE C STRICT;
DROP CONVERSION IF EXISTS pg_catalog.sjis_to_utf8;
CREATE DEFAULT CONVERSION pg_catalog.sjis_eudc_to_utf8 FOR 'SJIS' TO 'UTF8' FROM sjis_eudc_to_utf8;

-- UTF8 --> SJIS_EUDC
CREATE FUNCTION pg_catalog.utf8_to_sjis_eudc (integer, integer, cstring, internal, integer)
  RETURNS void AS '$libdir/eudc', 'utf8_to_sjis_eudc' LANGUAGE C STRICT;
DROP CONVERSION IF EXISTS pg_catalog.utf8_to_sjis;
CREATE DEFAULT CONVERSION pg_catalog.utf8_to_sjis_eudc FOR 'UTF8' TO 'SJIS' FROM utf8_to_sjis_eudc;

-- EUC_JP_EUDC --> UTF8
DROP CONVERSION IF EXISTS pg_catalog.euc_jp_to_utf8;
CREATE FUNCTION pg_catalog.euc_jp_eudc_to_utf8 (integer, integer, cstring, internal, integer)
  RETURNS void AS '$libdir/eudc', 'euc_jp_eudc_to_utf8' LANGUAGE C STRICT;
CREATE DEFAULT CONVERSION pg_catalog.euc_jp_eudc_to_utf8 FOR 'EUC_JP' TO 'UTF8' FROM euc_jp_eudc_to_utf8;

-- UTF8 --> EUC_JP_EUDC
CREATE FUNCTION pg_catalog.utf8_to_euc_jp_eudc (integer, integer, cstring, internal, integer)
  RETURNS void AS '$libdir/eudc', 'utf8_to_euc_jp_eudc' LANGUAGE C STRICT;
DROP CONVERSION IF EXISTS pg_catalog.utf8_to_euc_jp;
CREATE DEFAULT CONVERSION pg_catalog.utf8_to_euc_jp_eudc FOR 'UTF8' TO 'EUC_JP' FROM utf8_to_euc_jp_eudc;

-- To show eudc function information
CREATE OR REPLACE FUNCTION show_eudc(OUT "Conversion Function" varchar,
				     OUT "Source" varchar, OUT "Destination" varchar,
				     OUT "Is Default?" varchar)
  RETURNS SETOF record
  AS '$libdir/eudc', 'show_eudc' LANGUAGE C IMMUTABLE STRICT;
