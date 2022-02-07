CFLAG= -std=gnu99 -pthread 

main: build/main.o build/util_paser.o build/statistic.o build/method_char_tree.o \
	build/clock_count.o build/profile.o build/method_bitmap_tree.o \
	build/aho_corasick.o build/analysis_remain_pattern.o build/analysis_remain_reverse.o \
	build/analysis_remain_file.o build/analysis_remain_dfa.o build/method_odd_even.o \
	build/util_file.o build/method_origin.o build/method_longest_sub_pattern.o \
	build/method_synthesis.o build/method_empty.o  build/method_last_char_hash.o \
	build/method_hash_small_pattern.o build/method_filter_digest.o build/method_odd_even_ac_filter.o \
	build/method_odd_even_queue.o build/simulate_snort_ac_search.o build/method_odd_even_snort.o \
	build/method_twice_odd_even.o build/snort_wu_manber.o build/simulate_snort_wu_search.o \
	build/method_count_dirty.o
	
	gcc $(CFLAG) -o bin/main \
	build/main.o build/util_paser.o build/statistic.o \
	build/method_char_tree.o build/clock_count.o build/profile.o \
	build/method_bitmap_tree.o build/aho_corasick.o build/analysis_remain_file.o \
	build/analysis_remain_dfa.o build/method_odd_even.o build/util_file.o \
	build/method_origin.o build/method_longest_sub_pattern.o build/method_synthesis.o \
	build/method_empty.o build/method_last_char_hash.o build/method_hash_small_pattern.o \
	build/method_filter_digest.o build/method_odd_even_ac_filter.o build/method_odd_even_queue.o \
	build/simulate_snort_ac_search.o build/method_odd_even_snort.o build/method_twice_odd_even.o \
	build/snort_wu_manber.o build/simulate_snort_wu_search.o build/method_count_dirty.o

gdb: CFLAG= -std=gnu99 -pthread -g3 -O0
gdb: main

build/main.o: src/main.c src/main.h
	gcc $(CFLAG) -c src/main.c -o build/main.o

build/util_paser.o: src/util_paser.c src/util_paser.h
	gcc $(CFLAG) -c src/util_paser.c -o build/util_paser.o

build/statistic.o: src/statistic.c src/statistic.h
	gcc $(CFLAG) -c src/statistic.c -o build/statistic.o

build/method_char_tree.o: src/method_char_tree.c src/method_char_tree.h
	gcc $(CFLAG) -c src/method_char_tree.c -o build/method_char_tree.o

build/clock_count.o: src/clock_count.c src/clock_count.h
	gcc $(CFLAG) -c src/clock_count.c -o build/clock_count.o

build/profile.o: src/profile.c src/profile.h
	gcc $(CFLAG) -c src/profile.c -o build/profile.o

build/method_bitmap_tree.o: src/method_bitmap_tree.c src/method_bitmap_tree.h
	gcc $(CFLAG) -c src/method_bitmap_tree.c -o build/method_bitmap_tree.o

build/aho_corasick.o: src/aho_corasick.c src/aho_corasick.h
	gcc $(CFLAG) -c src/aho_corasick.c -o build/aho_corasick.o

build/analysis_remain_pattern.o: src/analysis_remain_pattern.c src/analysis_remain_pattern.h
	gcc $(CFLAG) -c src/analysis_remain_pattern.c -o build/analysis_remain_pattern.o

build/analysis_remain_reverse.o: src/analysis_remain_reverse.c src/analysis_remain_reverse.h
	gcc $(CFLAG) -c src/analysis_remain_reverse.c -o build/analysis_remain_reverse.o

build/analysis_remain_file.o: src/analysis_remain_file.c src/analysis_remain_file.h
	gcc $(CFLAG) -c src/analysis_remain_file.c -o build/analysis_remain_file.o

build/analysis_remain_dfa.o: src/analysis_remain_dfa.c src/analysis_remain_dfa.h
	gcc $(CFLAG) -c src/analysis_remain_dfa.c -o build/analysis_remain_dfa.o

build/method_odd_even.o: src/method_odd_even.c src/method_odd_even.h
	gcc $(CFLAG) -c src/method_odd_even.c -o build/method_odd_even.o

build/util_file.o: src/util_file.c src/util_file.h
	gcc $(CFLAG) -c src/util_file.c -o build/util_file.o

build/method_origin.o: src/method_origin.c src/method_origin.h
	gcc $(CFLAG) -c src/method_origin.c -o build/method_origin.o

build/method_longest_sub_pattern.o: src/method_longest_sub_pattern.c src/method_longest_sub_pattern.h
	gcc $(CFLAG) -c src/method_longest_sub_pattern.c -o build/method_longest_sub_pattern.o

build/method_synthesis.o: src/method_synthesis.c src/method_synthesis.h
	gcc $(CFLAG) -c src/method_synthesis.c -o build/method_synthesis.o

build/method_empty.o: src/method_empty.c src/method_empty.h
	gcc $(CFLAG) -c src/method_empty.c -o build/method_empty.o

build/method_last_char_hash.o: src/method_last_char_hash.c src/method_last_char_hash.h
	gcc $(CFLAG) -c src/method_last_char_hash.c -o build/method_last_char_hash.o

build/method_hash_small_pattern.o: src/method_hash_small_pattern.c src/method_hash_small_pattern.h
	gcc $(CFLAG) -c src/method_hash_small_pattern.c -o build/method_hash_small_pattern.o

build/method_filter_digest.o: src/method_filter_digest.c src/method_filter_digest.h
	gcc $(CFLAG) -c src/method_filter_digest.c -o build/method_filter_digest.o

build/method_odd_even_ac_filter.o: src/method_odd_even_ac_filter.c src/method_odd_even_ac_filter.h
	gcc $(CFLAG) -c src/method_odd_even_ac_filter.c -o build/method_odd_even_ac_filter.o

build/method_odd_even_queue.o: src/method_odd_even_queue.c src/method_odd_even_queue.h
	gcc $(CFLAG) -c src/method_odd_even_queue.c -o build/method_odd_even_queue.o

build/simulate_snort_ac_search.o: src/simulate_snort_ac_search.c src/simulate_snort_ac_search.h
	gcc $(CFLAG) -c src/simulate_snort_ac_search.c -o build/simulate_snort_ac_search.o

build/method_odd_even_snort.o: src/method_odd_even_snort.c src/method_odd_even_snort.h
	gcc $(CFLAG) -c src/method_odd_even_snort.c -o build/method_odd_even_snort.o

build/method_twice_odd_even.o: src/method_twice_odd_even.c src/method_twice_odd_even.h
	gcc $(CFLAG) -c src/method_twice_odd_even.c -o build/method_twice_odd_even.o

build/snort_wu_manber.o: src/snort_wu_manber.c src/snort_wu_manber.h
	gcc $(CFLAG) -c src/snort_wu_manber.c -o build/snort_wu_manber.o

build/simulate_snort_wu_search.o: src/simulate_snort_wu_search.c src/simulate_snort_wu_search.h
	gcc $(CFLAG) -c src/simulate_snort_wu_search.c -o build/simulate_snort_wu_search.o

build/method_count_dirty.o: src/method_count_dirty.c src/method_count_dirty.h
	gcc $(CFLAG) -c src/method_count_dirty.c -o build/method_count_dirty.o

init:
	mkdir -p ./src
	mkdir -p ./bin
	mkdir -p ./res
	mkdir -p ./build
	mkdir -p ./log

clean:
	rm -rf ./bin/*
	rm -rf ./build/*

sync:
	make clean > /dev/null
	bash res/scp_file.sh
	make gdb > /dev/null

notes:
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 0 # Odd-Even AC
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 1 # Odd-Even AC with Match Tree
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 2 # Odd-Even AC with Match Tree using Bitmap
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 3 # Origin AC
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 4 # Twice Odd-Even AC
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 5 # Longest Sub Pattern Odd-Even AC
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 6 # Synthesis Odd-Even AC
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 7 # Odd-Even AC without Full Match (For testing clock time using by AC)
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 8 # Odd-Even AC with Last Char Filter (LCF)
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 9 # Odd-Even AC with Hash Small Pattern (HSP)
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 10 # Odd-Even AC without Function call
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 11 # Odd-Even AC using Last Char Filter without Function call
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 12 # Odd-Even AC using Digest Filter (DF)
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 0 -l -n 1 	# -l: Show detail, -n 1: Run trace one time.
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_2_05p -m 0 -l -n 1
	./bin/main -p ../patterns/patterns_500 -t ../traces/trace_7_90p -m 0 -l -n 100	# -n 100: Run trace 100 times.

	./bin/main -snort -p ./log/result_ac_id -t ./log/result_text_id -m 16	# -snort Simluate Snort Ac Search
	./bin/main -snort -p ./log/result_ac_id -t ./log/result_text_id -m 3
	./bin/main -snort -p ~/workspace/snort/scripts/parse_snort_ac_text/result_ac_id -t ~/workspace/snort/scripts/parse_snort_ac_text/result_text_id -m 16
	./bin/main -snort -p ~/workspace/snort/scripts/parse_snort_ac_text/result_ac_id_min_len2 -t ~/workspace/snort/scripts/parse_snort_ac_text/result_text_id -m 16 > result_m16
	./bin/main -p ~/workspace/snort/scripts/parse_snort_ac_text/test_ac -t ~/workspace/snort/scripts/parse_snort_ac_text/test_trace -m 3
	./bin/main -wu -p ~/workspace/snort/scripts/parse_snort_ac_text/snort_results/malware-cnc.rules/result_ac_id -t ~/workspace/snort/scripts/parse_snort_ac_text/snort_results/malware-cnc.rules/result_text_id > ./log/result_wu
	./bin/main -snort -p ~/workspace/snort/scripts/parse_snort_ac_text/snort_results/$(RULE_FILE)/result_ac_id -t ~/workspace/snort/scripts/parse_snort_ac_text/snort_results/$(RULE_FILE)/result_text_id -m 7 > ./log/result_m07_len1

test_snort_all:
	$(eval SNORT_PATTERN_FILE := "./log/snort_results/"$(RULE_FILE)"/result_ac_id")
	$(eval SNORT_TEXT_FILE="./log/snort_results/"$(RULE_FILE)"/result_text_id")
	$(eval SNORT_OUTPUT_FILE_ORIGIN="./log/snort_results/"$(RULE_FILE)"/result_m03")
	$(eval SNORT_OUTPUT_FILE_SCHEME_0="./log/snort_results/"$(RULE_FILE)"/result_m17")
	$(eval SNORT_OUTPUT_FILE_SCHEME_1="./log/snort_results/"$(RULE_FILE)"/result_wu")
	$(eval SNORT_OUTPUT_FILE_SCHEME_2="./log/snort_results/"$(RULE_FILE)"/result_m19")
	rm -f $(SNORT_OUTPUT_FILE_ORIGIN) $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_1)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 3 > $(SNORT_OUTPUT_FILE_ORIGIN)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 17 > $(SNORT_OUTPUT_FILE_SCHEME_0)
	./bin/main -wu -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) > $(SNORT_OUTPUT_FILE_SCHEME_1)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 19 > $(SNORT_OUTPUT_FILE_SCHEME_2)
	vimdiff $(SNORT_OUTPUT_FILE_ORIGIN) $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_1) $(SNORT_OUTPUT_FILE_SCHEME_2)
	# vimdiff $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_2)

test_snort_all_no_len1:
	$(eval SNORT_PATTERN_FILE := "./log/snort_results/"$(RULE_FILE)"/result_ac_id")
	$(eval SNORT_TEXT_FILE="./log/snort_results/"$(RULE_FILE)"/result_text_id")
	$(eval SNORT_OUTPUT_FILE_ORIGIN="./log/snort_results/"$(RULE_FILE)"/result_m20")
	$(eval SNORT_OUTPUT_FILE_SCHEME_0="./log/snort_results/"$(RULE_FILE)"/result_m23")
	$(eval SNORT_OUTPUT_FILE_SCHEME_1="./log/snort_results/"$(RULE_FILE)"/result_m16")
	$(eval SNORT_OUTPUT_FILE_SCHEME_2="./log/snort_results/"$(RULE_FILE)"/result_wu_no_len_one")
	rm -f $(SNORT_OUTPUT_FILE_ORIGIN) $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_1) $(SNORT_OUTPUT_FILE_SCHEME_2)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 20 > $(SNORT_OUTPUT_FILE_ORIGIN)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 23 > $(SNORT_OUTPUT_FILE_SCHEME_0)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 16 > $(SNORT_OUTPUT_FILE_SCHEME_1)
	./bin/main -wu -no-len-one -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) > $(SNORT_OUTPUT_FILE_SCHEME_2)
	vimdiff $(SNORT_OUTPUT_FILE_ORIGIN) $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_1) $(SNORT_OUTPUT_FILE_SCHEME_2)
	@# vimdiff $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_2)

test_snort_all_optimize:
	$(eval SNORT_PATTERN_FILE := "./log/snort_results/"$(RULE_FILE)"/result_ac_id")
	$(eval SNORT_TEXT_FILE="./log/snort_results/"$(RULE_FILE)"/result_text_id")
	$(eval SNORT_OUTPUT_FILE_SCHEME_0="./log/snort_results/"$(RULE_FILE)"/result_m23")
	$(eval SNORT_OUTPUT_FILE_SCHEME_1="./log/snort_results/"$(RULE_FILE)"/result_m21")
	$(eval SNORT_OUTPUT_FILE_SCHEME_2="./log/snort_results/"$(RULE_FILE)"/result_m22")
	rm -f $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_1) $(SNORT_OUTPUT_FILE_SCHEME_2)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 23 > $(SNORT_OUTPUT_FILE_SCHEME_0)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 21 > $(SNORT_OUTPUT_FILE_SCHEME_1)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 22 > $(SNORT_OUTPUT_FILE_SCHEME_2)
	vimdiff $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_1) $(SNORT_OUTPUT_FILE_SCHEME_2)
	# vimdiff $(SNORT_OUTPUT_FILE_SCHEME_0) $(SNORT_OUTPUT_FILE_SCHEME_2)

test_twice_odd_even:
	$(eval SNORT_PATTERN_FILE := "./log/snort_results/"$(RULE_FILE)"/result_ac_id")
	$(eval SNORT_TEXT_FILE="./log/snort_results/"$(RULE_FILE)"/result_text_id")
	$(eval SNORT_OUTPUT_FILE_ORIGIN="./log/snort_results/"$(RULE_FILE)"/result_m03")
	$(eval SNORT_OUTPUT_FILE_SCHEME_0="./log/snort_results/"$(RULE_FILE)"/result_m18")
	rm -f $(SNORT_OUTPUT_FILE_ORIGIN) $(SNORT_OUTPUT_FILE_SCHEME_0)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 3 > $(SNORT_OUTPUT_FILE_ORIGIN)
	./bin/main -snort -p $(SNORT_PATTERN_FILE) -t $(SNORT_TEXT_FILE) -m 18 > $(SNORT_OUTPUT_FILE_SCHEME_0)
	@#vimdiff $(SNORT_OUTPUT_FILE_ORIGIN) $(SNORT_OUTPUT_FILE_SCHEME_0)

test_all_ruleset:
	make RULE_FILE=malware-cnc.rules test_snort_all
	make RULE_FILE=browser-plugins.rules test_snort_all
	make RULE_FILE=blacklist.rules test_snort_all
	make RULE_FILE=browser-ie.rules test_snort_all
	make RULE_FILE=file-flash.rules test_snort_all
	make RULE_FILE=server-webapp.rules test_snort_all
	make RULE_FILE=file-office.rules test_snort_all
	make RULE_FILE=server-other.rules test_snort_all
	make RULE_FILE=os-windows.rules test_snort_all
	make RULE_FILE=file-identify.rules test_snort_all
	make RULE_FILE=file-other.rules test_snort_all
	make RULE_FILE=file-pdf.rules test_snort_all
	make RULE_FILE=exploit-kit.rules test_snort_all
	make RULE_FILE=pua-adware.rules test_snort_all
	make RULE_FILE=malware-other.rules test_snort_all
	make RULE_FILE=malware-backdoor.rules test_snort_all
	make RULE_FILE=server-oracle.rules test_snort_all
	make RULE_FILE=file-multimedia.rules test_snort_all
	make RULE_FILE=file-image.rules test_snort_all
	make RULE_FILE=netbios.rules test_snort_all
	make RULE_FILE=file-java.rules test_snort_all
	make RULE_FILE=indicator-compromise.rules test_snort_all
	make RULE_FILE=browser-firefox.rules test_snort_all
	make RULE_FILE=protocol-voip.rules test_snort_all
	make RULE_FILE=protocol-scada.rules test_snort_all
	make RULE_FILE=protocol-rpc.rules test_snort_all
	make RULE_FILE=pua-toolbars.rules test_snort_all
	make RULE_FILE=indicator-shellcode.rules test_snort_all
	make RULE_FILE=indicator-obfuscation.rules test_snort_all
	make RULE_FILE=policy-other.rules test_snort_all
	make RULE_FILE=server-iis.rules test_snort_all
	make RULE_FILE=server-mail.rules test_snort_all
	make RULE_FILE=policy-spam.rules test_snort_all
	make RULE_FILE=malware-tools.rules test_snort_all
	make RULE_FILE=app-detect.rules test_snort_all
	make RULE_FILE=os-mobile.rules test_snort_all
	make RULE_FILE=server-apache.rules test_snort_all
	make RULE_FILE=protocol-ftp.rules test_snort_all
	make RULE_FILE=sql.rules test_snort_all
	make RULE_FILE=browser-webkit.rules test_snort_all
	make RULE_FILE=protocol-icmp.rules test_snort_all
	make RULE_FILE=file-executable.rules test_snort_all
	make RULE_FILE=server-mysql.rules test_snort_all
	make RULE_FILE=server-mssql.rules test_snort_all
	# make RULE_FILE=protocol-dns.rules test_snort_all
	make RULE_FILE=policy-social.rules test_snort_all
	make RULE_FILE=browser-other.rules test_snort_all
	# make RULE_FILE=protocol-imap.rules test_snort_all
	make RULE_FILE=browser-chrome.rules test_snort_all
	make RULE_FILE=os-other.rules test_snort_all
	# make RULE_FILE=protocol-snmp.rules test_snort_all
	make RULE_FILE=server-samba.rules test_snort_all
	make RULE_FILE=os-linux.rules test_snort_all
	# make RULE_FILE=protocol-telnet.rules test_snort_all
	make RULE_FILE=indicator-scan.rules test_snort_all
	make RULE_FILE=pua-other.rules test_snort_all
	# make RULE_FILE=protocol-pop.rules test_snort_all
	# make RULE_FILE=protocol-tftp.rules test_snort_all
	make RULE_FILE=pua-p2p.rules test_snort_all
	make RULE_FILE=content-replace.rules test_snort_all
	# make RULE_FILE=protocol-services.rules test_snort_all
	# make RULE_FILE=protocol-nntp.rules test_snort_all
	# make RULE_FILE=os-solaris.rules test_snort_all
	# make RULE_FILE=protocol-finger.rules test_snort_all
	make RULE_FILE=policy-multimedia.rules test_snort_all
	# make RULE_FILE=x11.rules test_snort_all
	# make RULE_FILE=scada.rules test_snort_all

test_all_no_len1_ruleset:
	make RULE_FILE=malware-cnc.rules test_snort_all_no_len1
	make RULE_FILE=browser-plugins.rules test_snort_all_no_len1
	make RULE_FILE=blacklist.rules test_snort_all_no_len1
	make RULE_FILE=browser-ie.rules test_snort_all_no_len1
	make RULE_FILE=file-flash.rules test_snort_all_no_len1
	make RULE_FILE=server-webapp.rules test_snort_all_no_len1
	make RULE_FILE=file-office.rules test_snort_all_no_len1
	make RULE_FILE=server-other.rules test_snort_all_no_len1
	make RULE_FILE=os-windows.rules test_snort_all_no_len1
	make RULE_FILE=file-identify.rules test_snort_all_no_len1
	make RULE_FILE=file-other.rules test_snort_all_no_len1
	make RULE_FILE=file-pdf.rules test_snort_all_no_len1
	make RULE_FILE=exploit-kit.rules test_snort_all_no_len1
	make RULE_FILE=pua-adware.rules test_snort_all_no_len1
	make RULE_FILE=malware-other.rules test_snort_all_no_len1
	make RULE_FILE=malware-backdoor.rules test_snort_all_no_len1
	make RULE_FILE=server-oracle.rules test_snort_all_no_len1
	make RULE_FILE=file-multimedia.rules test_snort_all_no_len1
	make RULE_FILE=file-image.rules test_snort_all_no_len1
	make RULE_FILE=netbios.rules test_snort_all_no_len1
	make RULE_FILE=file-java.rules test_snort_all_no_len1
	make RULE_FILE=indicator-compromise.rules test_snort_all_no_len1
	make RULE_FILE=browser-firefox.rules test_snort_all_no_len1
	make RULE_FILE=protocol-voip.rules test_snort_all_no_len1
	make RULE_FILE=protocol-scada.rules test_snort_all_no_len1
	make RULE_FILE=protocol-rpc.rules test_snort_all_no_len1
	make RULE_FILE=pua-toolbars.rules test_snort_all_no_len1
	make RULE_FILE=indicator-shellcode.rules test_snort_all_no_len1
	make RULE_FILE=indicator-obfuscation.rules test_snort_all_no_len1
	make RULE_FILE=policy-other.rules test_snort_all_no_len1
	make RULE_FILE=server-iis.rules test_snort_all_no_len1
	make RULE_FILE=server-mail.rules test_snort_all_no_len1
	make RULE_FILE=policy-spam.rules test_snort_all_no_len1
	make RULE_FILE=malware-tools.rules test_snort_all_no_len1
	make RULE_FILE=app-detect.rules test_snort_all_no_len1
	make RULE_FILE=os-mobile.rules test_snort_all_no_len1
	make RULE_FILE=server-apache.rules test_snort_all_no_len1
	make RULE_FILE=protocol-ftp.rules test_snort_all_no_len1
	make RULE_FILE=sql.rules test_snort_all_no_len1
	make RULE_FILE=browser-webkit.rules test_snort_all_no_len1
	make RULE_FILE=protocol-icmp.rules test_snort_all_no_len1
	make RULE_FILE=file-executable.rules test_snort_all_no_len1
	make RULE_FILE=server-mysql.rules test_snort_all_no_len1
	make RULE_FILE=server-mssql.rules test_snort_all_no_len1
	make RULE_FILE=protocol-dns.rules test_snort_all_no_len1
	make RULE_FILE=policy-social.rules test_snort_all_no_len1
	make RULE_FILE=browser-other.rules test_snort_all_no_len1
	make RULE_FILE=protocol-imap.rules test_snort_all_no_len1
	make RULE_FILE=browser-chrome.rules test_snort_all_no_len1
	make RULE_FILE=os-other.rules test_snort_all_no_len1
	make RULE_FILE=protocol-snmp.rules test_snort_all_no_len1
	make RULE_FILE=server-samba.rules test_snort_all_no_len1
	make RULE_FILE=os-linux.rules test_snort_all_no_len1
	make RULE_FILE=protocol-telnet.rules test_snort_all_no_len1
	make RULE_FILE=indicator-scan.rules test_snort_all_no_len1
	make RULE_FILE=pua-other.rules test_snort_all_no_len1
	make RULE_FILE=protocol-pop.rules test_snort_all_no_len1
	make RULE_FILE=protocol-tftp.rules test_snort_all_no_len1
	make RULE_FILE=pua-p2p.rules test_snort_all_no_len1
	make RULE_FILE=content-replace.rules test_snort_all_no_len1
	make RULE_FILE=protocol-services.rules test_snort_all_no_len1
	make RULE_FILE=protocol-nntp.rules test_snort_all_no_len1
	make RULE_FILE=os-solaris.rules test_snort_all_no_len1
	make RULE_FILE=protocol-finger.rules test_snort_all_no_len1
	make RULE_FILE=policy-multimedia.rules test_snort_all_no_len1
	make RULE_FILE=x11.rules test_snort_all_no_len1
	make RULE_FILE=scada.rules test_snort_all_no_len1

test_show_all_result:
	@make RULE_FILE=malware-cnc.rules show_result
	make RULE_FILE=browser-plugins.rules show_result
	make RULE_FILE=blacklist.rules show_result
	make RULE_FILE=browser-ie.rules show_result
	make RULE_FILE=file-flash.rules show_result
	make RULE_FILE=server-webapp.rules show_result
	make RULE_FILE=file-office.rules show_result
	make RULE_FILE=server-other.rules show_result
	make RULE_FILE=os-windows.rules show_result
	make RULE_FILE=file-identify.rules show_result
	make RULE_FILE=file-other.rules show_result
	make RULE_FILE=file-pdf.rules show_result
	make RULE_FILE=exploit-kit.rules show_result
	make RULE_FILE=pua-adware.rules show_result
	make RULE_FILE=malware-other.rules show_result
	make RULE_FILE=malware-backdoor.rules show_result
	make RULE_FILE=server-oracle.rules show_result
	make RULE_FILE=file-multimedia.rules show_result
	make RULE_FILE=file-image.rules show_result
	make RULE_FILE=netbios.rules show_result
	make RULE_FILE=file-java.rules show_result
	make RULE_FILE=indicator-compromise.rules show_result
	make RULE_FILE=browser-firefox.rules show_result
	make RULE_FILE=protocol-voip.rules show_result
	make RULE_FILE=protocol-scada.rules show_result
	make RULE_FILE=protocol-rpc.rules show_result
	make RULE_FILE=pua-toolbars.rules show_result
	make RULE_FILE=indicator-shellcode.rules show_result
	make RULE_FILE=indicator-obfuscation.rules show_result
	make RULE_FILE=policy-other.rules show_result
	make RULE_FILE=server-iis.rules show_result
	make RULE_FILE=server-mail.rules show_result
	make RULE_FILE=policy-spam.rules show_result
	make RULE_FILE=malware-tools.rules show_result
	make RULE_FILE=app-detect.rules show_result
	make RULE_FILE=os-mobile.rules show_result
	make RULE_FILE=server-apache.rules show_result
	make RULE_FILE=protocol-ftp.rules show_result
	make RULE_FILE=sql.rules show_result
	make RULE_FILE=browser-webkit.rules show_result
	make RULE_FILE=protocol-icmp.rules show_result
	make RULE_FILE=file-executable.rules show_result
	make RULE_FILE=server-mysql.rules show_result
	make RULE_FILE=server-mssql.rules show_result
	# make RULE_FILE=protocol-dns.rules show_result
	make RULE_FILE=policy-social.rules show_result
	make RULE_FILE=browser-other.rules show_result
	# make RULE_FILE=protocol-imap.rules show_result
	make RULE_FILE=browser-chrome.rules show_result
	make RULE_FILE=os-other.rules show_result
	# make RULE_FILE=protocol-snmp.rules show_result
	make RULE_FILE=server-samba.rules show_result
	make RULE_FILE=os-linux.rules show_result
	# make RULE_FILE=protocol-telnet.rules show_result
	make RULE_FILE=indicator-scan.rules show_result
	make RULE_FILE=pua-other.rules show_result
	# make RULE_FILE=protocol-pop.rules show_result
	# make RULE_FILE=protocol-tftp.rules show_result
	make RULE_FILE=pua-p2p.rules show_result
	make RULE_FILE=content-replace.rules show_result
	# make RULE_FILE=protocol-services.rules show_result
	# make RULE_FILE=protocol-nntp.rules show_result
	# make RULE_FILE=os-solaris.rules show_result
	# make RULE_FILE=protocol-finger.rules show_result
	make RULE_FILE=policy-multimedia.rules show_result
	# make RULE_FILE=x11.rules show_result
	# make RULE_FILE=scada.rules show_result