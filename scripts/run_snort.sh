if [ -z "$1" ]
then
    echo "No Snort Rule File Name."
    exit 0
fi

if ! [ -f "/etc/snort/rules/$1" ] ; then
    echo "File is not exist."    
    exit 0
fi

sudo sed -i "s/^include \$RULE\_PATH/#include \$RULE\_PATH/" /etc/snort/snort.conf
sudo sed -i "548s/.*/include \$RULE\_PATH\/$1/" /etc/snort/snort.conf 
# sudo sed -i "s/^#include \$RULE\_PATH\/$1/include \$RULE\_PATH\/$1/" /etc/snort/snort.conf

RULE_FILE=$(grep "^include \$RULE_PATH" "/etc/snort/snort.conf" | grep -o "[^\/]\+$")
SNORT_RULE_SIZE=$(grep "alert " "/etc/snort/rules/"$1 | wc -l)

sudo rm result_ac result_text result_ac_id result_text_id > /dev/null 2>&1
sudo rm test_ac test_trace result_ruleset result_snort > /dev/null 2>&1
# sudo snort -r /home/user/odd_even_files/resources/defcon_traces/ctf_dc17.pcap000 -c /etc/snort/snort.conf > result_snort 2>&1
# sudo snort -r /home/user/odd_even_files/resources/defcon_traces/ctf_dc17.pcap023 -c /etc/snort/snort.conf > result_snort 2>&1
sudo snort -r /home/user/odd_even_files/resources/defcon_traces/ctf_dc17.pcap048 -c /etc/snort/snort.conf > result_snort 2>&1
# sudo snort --pcap-dir /home/user/workspace/snort/resource/ctf_dc2017_parts -c /etc/snort/snort.conf > result_snort 2>&1

sudo rm gmon.out > /dev/null 2>&1

if ! [ -f "result_ac" ] ; then
    echo "No result_ac file with $1."
    mkdir -p "./snort_results/$1"
    mv result_snort "./snort_results/$1/"
    exit 0
else
    sudo chmod 666 result_ac
fi

if ! [ -f "result_text" ] ; then
    echo "No result_text file with $1."    
    mkdir -p "./snort_results/$1"
    mv result_ac result_snort "./snort_results/$1/"
    exit 0
else
    sudo chmod 666 result_text
fi


touch result_ruleset
echo $RULE_FILE
echo $RULE_FILE >> result_ruleset
ls -alh "/etc/snort/rules/"$RULE_FILE >> result_ruleset
echo "Number of Snort Rule: "$SNORT_RULE_SIZE >> result_ruleset

python convert_ac_pointer_to_id.py >> result_ruleset
# echo "Number of AC: "$(cat result_ac_id | grep "AC_OBJECT" | wc -l)
# echo "Number of Pattern: "$(cat result_ac_id | wc -l)
echo "Number of Pattern Min Len 1: "$(cat result_ac_id | grep "AC_PATTERN,..$" | wc -l) >> result_ruleset

mkdir -p "./snort_results/$1"
mv result_ac result_text result_ac_id result_text_id result_ruleset result_snort "./snort_results/$1/"

# rm -f result.tar.gz
# GZIP=-9 tar cvzf result.tar.gz result_ac_id result_text_id