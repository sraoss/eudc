EUDC (End User Defined Character) : 外字対応文字コード
======================================================

eudc 拡張モジュールは、外字領域の文字コード変換をサポートしたエンコーディングです。
シフトSJIS (SJIS) や EUC\_JP と、UTF-8 の変換において、外字領域の変換を追加します。
絵文字等の外字を扱う必要があり、かつクライアント・アプリケーションとデータベースで異なる文字コードを使う場合に有用です。
eudc を導入すると、以下の相互変換が可能になります。

## 外字のマッピング規則

| エンコード 1      | エンコード 2     | 特記事項 |
|:-------------------|:-----------------|:----------|
| UTF-8: U+E000 - U+E757 | SJIS: F040 - F9FF | 利用可能なコード領域を線形に対応 |
| UTF-8: U+E000 - U+E3AB, U+E3AC - U+E757 | EUC\_JP: F5A1 - FEFE, 8FF5A1 - 8FFEFE | 利用可能なコード領域を線形に対応 |
| SJIS: F040 - F9FF  | EUC\_JP: F5A1 - FEFE, 8FF5A1 - 8FFEFE | eudc を導入しなくても変換可能 |

UTF-8 と SJIS とのマッピングは Windows での外字のマッピングと一致させています。

表内の SJIS のコード範囲 F040 - F9FF は 1バイト目が F0 から F9、2バイト目が 40 から FF という意味です。EUC\_JP についても同様に解釈してください。また SJIS では 2バイト目が 07、FD、FE、FF であるコードは利用可能では無いため、範囲内であっても除外されます。

単純な線形マッピングを行いますので、例えば字形に合わせた独自にマッピングが必要な場合には対応できません。
外字の単純な保存、検索、取得で十分な場合にご利用ください。

## インストール

インストール先 PostgreSQL のコマンドに PATH が通った状態で以下を実行します。
PostgreSQL のヘッダファイルもインストールされている必要があります。

~~~console
$ make USE_PGXS=1
$ su -c 'make install'
~~~

CREATE EXTENSION で使用するデータベースに登録します。DBスーパーユーザにて実行してください。

~~~console
$ psql -U postgres {database_name}
database_name=# CREATE EXTENSION eudc;
~~~


### 注意

CREATE EXTENSION で登録して、それをアンインストールする際に DROP EXTENSION の前に SELECT disable\_eudc(); を忘れずに実行してください。
元々の文字コード変換関数がデフォルト設定に戻らないため暗黙的な文字コード変換が行われなくなってしまいます。

~~~console
database_name=# SELECT disable_eudc(); DROP EXTENSION eudc;
~~~

以下のようなイベントトリガを作ることで、DROP EXTENSION eudc の後に自動的に SELECT disable\_eudc() と同様の処理を実行させる方法も考えられます。


~~~
CREATE FUNCTION pg_catalog.trgf_disable_eudc()
  RETURNS event_trigger LANGUAGE plpgsql AS
$$
DECLARE obj RECORD;
BEGIN
  FOR obj IN SELECT * FROM pg_event_trigger_dropped_objects() LOOP
    IF tg_tag = 'DROP EXTENSION' AND obj.object_identity = 'eudc' THEN
      UPDATE pg_conversion SET condefault = TRUE WHERE conname IN
        ('sjis_to_utf8', 'utf8_to_sjis', 'euc_jp_to_utf8', 'utf8_to_euc_jp');
      EXIT;
    END IF;
  END LOOP;
END;
$$;

CREATE EVENT TRIGGER evt_drop_eudc ON sql_drop
  EXECUTE FUNCTION pg_catalog.trgf_disable_eudc();
~~~

これを導入することは全ての DROP文に対するオーバーヘッドになるため、このコードは eudc拡張自体には含めていません。


## 関数

eudcは下記の変換関数を導入します。

* sjis\_eudc\_to\_utf8 : SJISからUTF-8に外字領域を含めた変換を行います
* utf8\_to\_sjis\_eudc : UTF-8からSJISに外字領域を含めた変換を行います
* euc\_jp\_eudc\_to\_utf8 : EUC\_JPからUTF-8に外字領域を含めた変換を行います
* utf8\_to\_euc\_jp\_eudc : UTF-8からEUC\_JPに外字領域を含めた変換を行います

以下の関数で、これらをデフォルトの変換として使用するかを切り替えできます。

~~~console
database_name=# SELECT enable_eudc();
database_name=# SELECT disable_eudc();
~~~

eudcによる変換がデフォルトであるか確認するには以下の関数を実行してください。

~~~console
database_name=# SELECT * FROM show_eudc();
 Conversion Function | Source | Destination | Is Default?
---------------------+--------+-------------+-------------
 sjis_eudc_to_utf8   | SJIS   | UTF8        | yes
 utf8_to_sjis_eudc   | UTF8   | SJIS        | yes
 euc_jp_eudc_to_utf8 | EUC_JP | UTF8        | yes
 utf8_to_euc_jp_eudc | UTF8   | EUC_JP      | yes
(4 rows)
~~~

## 設定パラメータ

<dl>
<dt>eudc.fallback_character</dt>
<dd>
外字を変換する際に、常に特定の文字にマッピングする場合に、その文字を指定します。
指定する文字はマルチバイト文字でも構いませんが、1文字である必要があります。
デフォルトは空文字で、特定の文字にマッピングするのではなく、外字領域のコードを線形にマッピングします。
</dd>
<dd>
<strong>[注意]</strong>
このパラメータは動的に変更できますが、サービス開始後に変更すると検索結果の整合性無くなる可能性があります。
設定値の変更前と変更後で実際にデータベースで扱う文字が変化するため、同一の問合せであっても同一の結果が返却されなくなる可能性があります。
</dd>
<dt>eudc.log_level</dt>
<dd>
バージョン 8.4 以降でのみ利用できます。
外字を変換した際にログに出力するメッセージのレベルを指定します。
有効な値は DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1, LOG, NOTICE, WARNING, ERROR, FATAL です。
デフォルトは DEBUG2 です。
出力されるログの例を以下に示します。
<pre>eudc character found: f040 in SJIS to UTF8 conversion</pre>
</dd>
</dl>

これらのパラメータは postgresql.conf で指定することもできます。
設定例を以下に示します。

~~~
# postgresql.conf
eudc.fallback_character = '〓'   # すべての EUDC はゲタに変換する
eudc.log_level = warning
~~~

## 動作環境

<dl>
<dt>PostgreSQLバージョン</dt>
<dd>PostgreSQL 11, 12, 13, 14, 15, 16, 17, 18</dd>
</dl>

## バージョン2.0について

eudc バージョン 2.0 以降は バージョン 1.x までとは拡張の構成を大幅に変えています。
そのため、既にバージョン 1.x を導入している環境で、バージョン 2.x を導入するには、バージョン 1.x のアンインストール手順を実行後、
バージョン 2.x のインストールを行う必要があります。

-----------------------------------------------------------

Copyright (c) 2014, NIPPON TELEGRAPH AND TELEPHONE CORPORATION

Copyright (c) 2025, SRA OSS K.K.

