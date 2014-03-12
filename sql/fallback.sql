LOAD 'eudc';
SET client_encoding = 'UTF8';
SET eudc.fallback_character = '〓';
SET eudc.log_level = 'info';

SELECT encode(convert('〓', 'UTF8', 'UTF8'), 'hex') AS utf8,
       encode(convert('〓', 'UTF8', 'SJIS'), 'hex') AS sjis,
       encode(convert('〓', 'UTF8', 'EUC_JP'), 'hex') AS euc_jp;

SELECT i,
       encode(convert(decode(i, 'hex'), 'SJIS', 'UTF8'), 'hex') AS utf8,
       encode(convert(decode(i, 'hex'), 'SJIS', 'EUC_JP'), 'hex') AS euc_jp
  FROM (VALUES ('8476'),
               ('82a0'),
               ('f040'),
               ('f1fc'),
               ('f38d'),
               ('f39e'),
               ('f39f'),
               ('f4fc'),
               ('f540'),
               ('f9fc')) AS t(i);

SELECT i,
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
               ('ee9d97')) AS t(i);

SELECT i,
       encode(convert(decode(i, 'hex'), 'EUC_JP', 'UTF8'), 'hex') AS utf8,
       encode(convert(decode(i, 'hex'), 'EUC_JP', 'SJIS'), 'hex') AS sjis
  FROM (VALUES ('a7d7'),
               ('a4a2'),
               ('f5a1'),
               ('f8fe'),
               ('fbed'),
               ('fbfe'),
               ('fca1'),
               ('fefe'),
               ('8ff5a1'),
               ('8ffefe')) AS t(i);

RESET eudc.fallback_character;
RESET eudc.log_level;


-- bad configuration
SET eudc.fallback_character = 'two or more characters';
SET eudc.log_level = 'panic';
