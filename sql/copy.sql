SET client_min_messages = warning;
\set ECHO none
DROP DATABASE IF EXISTS contrib_regression_utf8;
\set ECHO all
RESET client_min_messages;

CREATE DATABASE contrib_regression_utf8 TEMPLATE template0 ENCODING 'utf8';

\connect contrib_regression_utf8
\set ECHO none
\i eudc.sql
\set ECHO all
CREATE TABLE foo(str text);
SET client_encoding = SJIS;
\copy foo FROM 'data/sjis.csv' WITH CSV
\copy foo TO 'results/sjis.csv' WITH CSV
\! diff data/sjis.csv results/sjis.csv

SELECT encode(str::bytea, 'hex') FROM foo;
