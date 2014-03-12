-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION eudc_drop" to drop eudc extension. \quit

DROP EXTENSION eudc;

CREATE DEFAULT CONVERSION pg_catalog.sjis_to_utf8 FOR 'SJIS' TO 'UTF8' FROM sjis_to_utf8;
CREATE DEFAULT CONVERSION pg_catalog.utf8_to_sjis FOR 'UTF8' TO 'SJIS' FROM utf8_to_sjis;
CREATE DEFAULT CONVERSION pg_catalog.euc_jp_to_utf8 FOR 'EUC_JP' TO 'UTF8' FROM euc_jp_to_utf8;
CREATE DEFAULT CONVERSION pg_catalog.utf8_to_euc_jp FOR 'UTF8' TO 'EUC_JP' FROM utf8_to_euc_jp;

ALTER EXTENSION eudc_drop DROP CONVERSION pg_catalog.sjis_to_utf8;
ALTER EXTENSION eudc_drop DROP CONVERSION pg_catalog.utf8_to_sjis;
ALTER EXTENSION eudc_drop DROP CONVERSION pg_catalog.euc_jp_to_utf8;
ALTER EXTENSION eudc_drop DROP CONVERSION pg_catalog.utf8_to_euc_jp;
