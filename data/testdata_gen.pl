#!/usr/bin/perl

# Generate test quoted CSV data
# for SJIS   F040 - F9FF (f0-f9 40-ff)
# for EUC_JP F5A1 - FEFE (f5-fe a1-fe), 8FF5A1 - 8FFEFE (8f f5-fe a1-fe)
#
# usage sample:
#   perl testdata_gen.pl SJIS SQL > sjis.sql
#   perl testdata_gen.pl EUC_JP CSV > euc_jp.csv

$mode = "CSV";
if (@ARGV == 2) {
	if ($ARGV[1] eq "SQL") {
		$mode = "SQL";
	}
}

if (@ARGV >= 1) {

	if ($ARGV[0] eq "SJIS") {
		for ($c1 = 0xf0 ; $c1 <= 0xf9 ; ++$c1) {
			for ($c2 = 0x40 ; $c2 <= 0xff ; ++$c2) {
				if ($c2 == 0xfd || $c2 == 0xfe || $c2 == 0xff || $c2 == 0x7f) {
					next; # unused code
				}
				if ($mode eq "CSV") {
					printf("\"%x%x\",\"%c%c\"\n", $c1, $c2, $c1, $c2);
				} elsif ($mode eq "SQL") {
					printf("SELECT convert('\\x%x%x', 'SJIS', 'UTF8');\n", $c1, $c2);
				}
			}
		}

	} elsif ($ARGV[0] eq "EUC_JP") {
		for ($c1 = 0xf5 ; $c1 <= 0xfe ; ++$c1) {
			for ($c2 = 0xa1 ; $c2 <= 0xfe ; ++$c2) {
				if ($mode eq "CSV") {
					printf("\"%x%x\",\"%c%c\"\n", $c1, $2, $c1, $c2);
				} elsif ($mode eq "SQL") {
					printf("SELECT convert('\\x%x%x', 'EUC_JP', 'UTF8');\n", $c1, $c2);
				}
			}
		}

	} elsif ($ARGV[0] eq "EUC_JP3") {
		$c1 = 0x8f ;
		for ($c2 = 0xf5 ; $c2 <= 0xfe ; ++$c2) {
			for ($c3 = 0xa1 ; $c3 <= 0xfe ; ++$c3) {
				if ($mode eq "CSV") {
					printf("\"%x%x%x\",\"%c%c%c\"\n", $c1, $c2, $c3, $c1, $c2, $c3);
				} elsif ($mode eq "SQL") {
					printf("SELECT convert('\\x%x%x%x', 'EUC_JP', 'UTF8');\n", $c1, $c2, $c3);
				}
			}
		}
	}
}

