# sudo snort -A console -k none -c /etc/snort/snort.conf -i enp2s0
# sudo snort -A console -k none -c /etc/snort/snort_local.conf -i enp2s0
# sudo snort -r /home/user/workspace/snort/resource/ctf_dc2017/ctf_dc17.pcap048 -c /etc/snort/snort.conf
# sudo snort -r /home/user/workspace/snort/resource/ctf_dc2017/ctf_dc17.pcap048 -A console -c /etc/snort/snort.conf > result 2>&1
sudo snort -r /home/user/workspace/snort/resource/ctf_dc2017/ctf_dc17.pcap048 -c /etc/snort/snort_local.conf
# sudo snort -r /home/user/workspace/snort/resource/ctf_dc2017/ctf_dc17.pcap048 -c /etc/snort/snort_single_defcon.conf
# sudo gdb --args snort -r /home/user/workspace/snort/resource/ctf_dc2017/ctf_dc17.pcap048 -c /etc/snort/snort_local.conf
# sudo snort -T -i enp2s0 -c /etc/snort/snort_single.conf
# sudo snort -T -i enp2s0 -c /etc/snort/snort_single.conf > result 2>&1
