-- SJIS to UTF8 and EUC_JP
CREATE TABLE sjis (sjis text, utf8 text, euc_jp text);
INSERT INTO sjis SELECT i,
       encode(convert(decode(i, 'hex'), 'SJIS', 'UTF8'), 'hex'),
       encode(convert(decode(i, 'hex'), 'SJIS', 'EUC_JP'), 'hex')
  FROM (VALUES ('8476'),
               ('82a0'),
               ('f040'),
               ('f1fc'),
               ('f38d'),
               ('f39e'),
               ('f39f'),
               ('f4fc'),
               ('f540'),
               ('f9fc')) AS t(i)
  RETURNING *;

-- UTF8 to SJIS and EUC_JP
CREATE TABLE utf8 (utf8 text, sjis text, euc_jp text);
INSERT INTO utf8 SELECT i,
       encode(convert(decode(i, 'hex'), 'UTF8', 'SJIS'), 'hex') AS sjis,
       encode(convert(decode(i, 'hex'), 'UTF8', 'EUC_JP'), 'hex') AS euc_jp
  FROM (VALUES ('d191'),
               ('e38182'),
               ('ee8080'),
               ('ee85b7'),
               ('ee8a80'),
               ('ee8a91'),
               ('ee8a92'),
               ('ee8eab'),
               ('ee8eac'),
               ('ee9d97')) AS t(i)
  RETURNING *;

-- EUC_JP to UTF8 and SJIS
CREATE TABLE euc_jp (euc_jp text, utf8 text, sjis text);
INSERT INTO euc_jp SELECT i,
       encode(convert(decode(i, 'hex'), 'EUC_JP', 'UTF8'), 'hex'),
       encode(convert(decode(i, 'hex'), 'EUC_JP', 'SJIS'), 'hex')
  FROM (VALUES ('a7d7'),
               ('a4a2'),
               ('f5a1'),
               ('f8fe'),
               ('fbed'),
               ('fbfe'),
               ('fca1'),
               ('fefe'),
               ('8ff5a1'),
               ('8ffefe')) AS t(i)
  RETURNING *;

-- sanity check
SELECT * FROM sjis AS s FULL JOIN utf8 AS u ON s.utf8 = u.utf8
 WHERE s.sjis <> u.sjis OR s.euc_jp <> u.euc_jp;
SELECT * FROM euc_jp AS e FULL JOIN utf8 AS u ON e.utf8 = u.utf8
 WHERE e.sjis <> u.sjis OR e.euc_jp <> u.euc_jp;
SELECT * FROM sjis AS s FULL JOIN euc_jp AS e ON s.utf8 = e.utf8
 WHERE s.sjis <> e.sjis OR s.euc_jp <> e.euc_jp;

-- error cases
SELECT encode(convert(decode('ee9d98', 'hex'), 'UTF8', 'SJIS'), 'hex');
SELECT encode(convert(decode('ee9d98', 'hex'), 'UTF8', 'EUC_JP'), 'hex');
SELECT encode(convert(decode('a460', 'hex'), 'EUC_JP', 'UTF8'), 'hex');

-- full test
\! data/testdata_gen.pl SJIS SQL > data/sjis.out.sql
\! psql -d contrib_regression -f data/sjis.out.sql > /dev/null
\! rm data/sjis.out.sql
\! data/testdata_gen.pl EUC_JP SQL > data/euc_jp.out.sql
\! psql -d contrib_regression -f data/euc_jp.out.sql > /dev/null
\! rm data/euc_jp.out.sql
\! data/testdata_gen.pl EUC_JP3 SQL > data/euc_jp3.out.sql
\! psql -d contrib_regression -f data/euc_jp3.out.sql > /dev/null
\! rm data/euc_jp3.out.sql

