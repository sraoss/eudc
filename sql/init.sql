--
-- First, define the conversion.  Turn off echoing so that expected file
-- does not depend on the installer script.
--
SET client_min_messages = warning;
\set ECHO none
\i eudc.sql
DROP DATABASE IF EXISTS contrib_regression_utf8;
\set ECHO all
RESET client_min_messages;
