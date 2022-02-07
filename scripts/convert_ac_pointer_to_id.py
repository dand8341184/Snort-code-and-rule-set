import os
import sys
import io
import re

ac_file = io.open('result_ac', 'r')
new_ac_file = io.open('result_ac_id', 'wb')
ac_pointer_map_id = {}
ac_index = 0

pattern_size = 0 
pattern_size_group = []
for line in ac_file.readlines():
    if 'AC_OBJECT' in line:
        if pattern_size != 0:
            pattern_size_group.append(pattern_size)
            pattern_size = 0

        value_group = line.strip().split(',')
        ac_pointer_map_id[value_group[1]] = ac_index
        
        new_ac_file.write('AC_OBJECT,')
        new_ac_file.write('%u\n' % (ac_index))
        ac_index += 1
    else:
        new_ac_file.write(line)
        if 'AC_PATTERN' in line:
            pattern_size += 1

pattern_size_group.append(pattern_size)
pattern_size = 0

max_pattern_size = max(pattern_size_group)
min_pattern_size = min(pattern_size_group)
sum_pattern_size = sum(pattern_size_group)
number_of_ac = len(pattern_size_group)
avg_pattern_size = sum_pattern_size / float(number_of_ac)

print "Total AC Size: %u" % (number_of_ac)
print "Max Pattern Size: %u" % (max_pattern_size)
print "Min Pattern Size: %u" % (min_pattern_size)
print "Sum Pattern Size: %u" % (sum_pattern_size)
print "Avg Pattern Size: %u" % (avg_pattern_size)

text_file = io.open('result_text', 'r')
new_text_file = io.open('result_text_id', 'wb')

for line in text_file.readlines():
    if 'PACKET_TEXT' in line:
        value_group = line.strip().split(',')
        if value_group[1] not in ac_pointer_map_id:
            print 'Found a pointer not existing in ac file.'
            exit(0)
        
        new_text_file.write('PACKET_TEXT,')
        new_text_file.write('%d,' % ac_pointer_map_id[value_group[1]])
        new_text_file.write('%s,' % value_group[2])
        new_text_file.write('%s,' % value_group[3])
        new_text_file.write('%s\n' % value_group[4])
    else:
        new_text_file.write(line)
