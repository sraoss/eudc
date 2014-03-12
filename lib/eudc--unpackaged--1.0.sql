-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION eudc" to load this file. \quit

ALTER EXTENSION eudc ADD FUNCTION pg_catalog.sjis_eudc_to_utf8 (integer, integer, cstring, internal, integer);
ALTER EXTENSION eudc ADD FUNCTION pg_catalog.utf8_to_sjis_eudc (integer, integer, cstring, internal, integer);
ALTER EXTENSION eudc ADD FUNCTION pg_catalog.euc_jp_eudc_to_utf8 (integer, integer, cstring, internal, integer);
ALTER EXTENSION eudc ADD FUNCTION pg_catalog.utf8_to_euc_jp_eudc (integer, integer, cstring, internal, integer);
ALTER EXTENSION eudc ADD FUNCTION show_eudc();
