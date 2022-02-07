
mkdir -p results

find_entry(){
    NUMBER="$1"
    PATTERN="$2"
    echo $(cat "results/$NUMBER.log" | grep "$PATTERN" | grep -o " [0-9]\+ " | tr -d "[:space:]")
}

sudo sed -i "s/include \$RULE\_PATH/#include \$RULE\_PATH/" /etc/snort/snort.conf

echo "FILE,ETH,VLAN,IP4,FRAG,ICMP,UDP,TCP,IP6,IP6_EXT,IP6_OPTS,FRAG6,ICMP6,UDP6,TCP6,TEREDO,ICMPIP,IP4_IP4,IP4_IP6,IP6_IP4,IP6_IP6,GRE,GRE_ETH,GRE_VLAN,GRE_IP4,GRE_IP6,GRE_IP6EXT,GRE_PPTP,GRE_ARP,GRE_IPX,GRE_LOOP,MPLS,ARP,IPX,ETH_LOOP,ETH_DISC,IP4_DISC,IP6_DISC,TCP_DISC,UDP_DISC,ICMP_DISC,ALL_DISCARD,OTHER,BAD_CHK_SUM,BAD_TTL,SSG1,SSG2,TOTAL" > result_statistic.csv

for i in $(seq -f "%03g" 0 77); do
    # echo $i
    sudo snort -r /home/user/odd_even_files/resources/defcon_traces/ctf_dc17.pcap$i -c /etc/snort/snort.conf > results/$i.log 2>&1
    
    PATTERN="        Eth:"
    ETH=$(find_entry $i "$PATTERN")

    PATTERN="       VLAN:"
    VLAN=$(find_entry $i "$PATTERN")

    PATTERN="        IP4:"
    IP4=$(find_entry $i "$PATTERN")
    
    PATTERN="       Frag:"
    FRAG=$(find_entry $i "$PATTERN")

    PATTERN="       ICMP:"
    ICMP=$(find_entry $i "$PATTERN")

    PATTERN="        UDP:"
    UDP=$(find_entry $i "$PATTERN")

    PATTERN="        TCP:"
    TCP=$(find_entry $i "$PATTERN")

    PATTERN="        IP6:"
    IP6=$(find_entry $i "$PATTERN")

    PATTERN="    IP6 Ext:"
    IP6_EXT=$(find_entry $i "$PATTERN")

    PATTERN="   IP6 Opts:"
    IP6_OPTS=$(find_entry $i "$PATTERN")

    PATTERN="      Frag6:"
    FRAG6=$(find_entry $i "$PATTERN")

    PATTERN="      ICMP6:"
    ICMP6=$(find_entry $i "$PATTERN")

    PATTERN="       UDP6:"
    UDP6=$(find_entry $i "$PATTERN")

    PATTERN="       TCP6:"
    TCP6=$(find_entry $i "$PATTERN")

    PATTERN="     Teredo:"
    TEREDO=$(find_entry $i "$PATTERN")

    PATTERN="    ICMP-IP:"
    ICMPIP=$(find_entry $i "$PATTERN")

    PATTERN="    IP4/IP4:"
    IP4_IP4=$(find_entry $i "$PATTERN")

    PATTERN="    IP4/IP6:"
    IP4_IP6=$(find_entry $i "$PATTERN")

    PATTERN="    IP6/IP4:"
    IP6_IP4=$(find_entry $i "$PATTERN")

    PATTERN="    IP6/IP6:"
    IP6_IP6=$(find_entry $i "$PATTERN")

    PATTERN="        GRE:"
    GRE=$(find_entry $i "$PATTERN")

    PATTERN="    GRE Eth:"
    GRE_ETH=$(find_entry $i "$PATTERN")

    PATTERN="   GRE VLAN:"
    GRE_VLAN=$(find_entry $i "$PATTERN")

    PATTERN="    GRE IP4:"
    GRE_IP4=$(find_entry $i "$PATTERN")

    PATTERN="    GRE IP6:"
    GRE_IP6=$(find_entry $i "$PATTERN")

    PATTERN="GRE IP6 Ext:"
    GRE_IP6EXT=$(find_entry $i "$PATTERN")

    PATTERN="   GRE PPTP:"
    GRE_PPTP=$(find_entry $i "$PATTERN")

    PATTERN="    GRE ARP:"
    GRE_ARP=$(find_entry $i "$PATTERN")

    PATTERN="    GRE IPX:"
    GRE_IPX=$(find_entry $i "$PATTERN")

    PATTERN="   GRE Loop:"
    GRE_LOOP=$(find_entry $i "$PATTERN")

    PATTERN="       MPLS:"
    MPLS=$(find_entry $i "$PATTERN")

    PATTERN="        ARP:"
    ARP=$(find_entry $i "$PATTERN")

    PATTERN="        IPX:"
    IPX=$(find_entry $i "$PATTERN")

    PATTERN="   Eth Loop:"
    ETH_LOOP=$(find_entry $i "$PATTERN")

    PATTERN="   Eth Disc:"
    ETH_DISC=$(find_entry $i "$PATTERN")

    PATTERN="   IP4 Disc:"
    IP4_DISC=$(find_entry $i "$PATTERN")

    PATTERN="   IP6 Disc:"
    IP6_DISC=$(find_entry $i "$PATTERN")

    PATTERN="   TCP Disc:"
    TCP_DISC=$(find_entry $i "$PATTERN")

    PATTERN="   UDP Disc:"
    UDP_DISC=$(find_entry $i "$PATTERN")

    PATTERN="  ICMP Disc:"
    ICMP_DISC=$(find_entry $i "$PATTERN")

    PATTERN="All Discard:"
    ALL_DISCARD=$(find_entry $i "$PATTERN")

    PATTERN="      Other:"
    OTHER=$(find_entry $i "$PATTERN")

    PATTERN="Bad Chk Sum:"
    BAD_CHK_SUM=$(find_entry $i "$PATTERN")

    PATTERN="    Bad TTL:"
    BAD_TTL=$(find_entry $i "$PATTERN")

    PATTERN="     S5 G 1:"
    SSG1=$(find_entry $i "$PATTERN")

    PATTERN="     S5 G 2:"
    SSG2=$(find_entry $i "$PATTERN")

    PATTERN="      Total:"
    TOTAL=$(cat "results/$i.log" | grep "$PATTERN" | grep -o " [0-9]\+$" | tr -d "[:space:]")

    echo $i,$ETH,$VLAN,$IP4,$FRAG,$ICMP,$UDP,$TCP,$IP6,$IP6_EXT,$IP6_OPTS,$FRAG6,$ICMP6,$UDP6,$TCP6,$TEREDO,$ICMPIP,$IP4_IP4,$IP4_IP6,$IP6_IP4,$IP6_IP6,$GRE,$GRE_ETH,$GRE_VLAN,$GRE_IP4,$GRE_IP6,$GRE_IP6EXT,$GRE_PPTP,$GRE_ARP,$GRE_IPX,$GRE_LOOP,$MPLS,$ARP,$IPX,$ETH_LOOP,$ETH_DISC,$IP4_DISC,$IP6_DISC,$TCP_DISC,$UDP_DISC,$ICMP_DISC,$ALL_DISCARD,$OTHER,$BAD_CHK_SUM,$BAD_TTL,$SSG1,$SSG2,$TOTAL >> result_statistic.csv
done

