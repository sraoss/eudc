DROP CONVERSION pg_catalog.sjis_eudc_to_utf8;
DROP CONVERSION pg_catalog.utf8_to_sjis_eudc;
DROP CONVERSION pg_catalog.euc_jp_eudc_to_utf8;
DROP CONVERSION pg_catalog.utf8_to_euc_jp_eudc;

DROP FUNCTION pg_catalog.sjis_eudc_to_utf8(integer, integer, cstring, internal, integer);
DROP FUNCTION pg_catalog.utf8_to_sjis_eudc(integer, integer, cstring, internal, integer);
DROP FUNCTION pg_catalog.euc_jp_eudc_to_utf8(integer, integer, cstring, internal, integer);
DROP FUNCTION pg_catalog.utf8_to_euc_jp_eudc(integer, integer, cstring, internal, integer);

CREATE DEFAULT CONVERSION pg_catalog.sjis_to_utf8 FOR 'SJIS' TO 'UTF8' FROM sjis_to_utf8;
CREATE DEFAULT CONVERSION pg_catalog.utf8_to_sjis FOR 'UTF8' TO 'SJIS' FROM utf8_to_sjis;
CREATE DEFAULT CONVERSION pg_catalog.euc_jp_to_utf8 FOR 'EUC_JP' TO 'UTF8' FROM euc_jp_to_utf8;
CREATE DEFAULT CONVERSION pg_catalog.utf8_to_euc_jp FOR 'UTF8' TO 'EUC_JP' FROM utf8_to_euc_jp;
