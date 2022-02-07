import io
import random
import sys

pattern_file_name = ""
dirty_rate = 0.5

arg_index = 1
arg_length = len(sys.argv)

if arg_length == 1:
    print "[INFO] python %s -p PATTERN_FILE -d DIRTY_RATE" % sys.argv[0]
    sys.exit(0)

print arg_length
while arg_index < arg_length:
    arg_string = sys.argv[arg_index]
    if arg_string == "-p":
        arg_index += 1
        if arg_index == arg_length:
            print "[INFO] Pattern file miss."
            sys.exit(0)
        else:
            pattern_file_name = sys.argv[arg_index]
    
    elif arg_string == "-d":
        arg_index += 1
        if arg_index == arg_length:
            print "[INFO] Dirty rate miss."
            sys.exit(0)
        else:
            dirty_rate = float(sys.argv[arg_index])
    
    arg_index += 1

if pattern_file_name == "":
    print "[INFO] Pattern file miss."
    sys.exit(0)

pattern_string_group = []
pattern_length_group = []
pattern_file = io.open(pattern_file_name, encoding='utf-8')
# pattern_file = io.open("patterns", encoding='utf-8')
count_pattern = 0
for line in pattern_file:
    string = line.strip()
    # string = map(ord, line.decode('hex'))
    length = len(string) / 2
    pattern_string_group.append(string)
    pattern_length_group.append(length)
    count_pattern = count_pattern + 1

trace_size = 2 * 1024 * 1024
# trace_size = 2 * 1024
# dirty_rate = 0.5

dirty_text_length = int(trace_size * dirty_rate)
dirty_text_count = 0

pattern_string_group_size = len(pattern_string_group)
random_pattern_id_group = []

print trace_size
print dirty_text_length

dirty_text_list_group = []
dirty_text_list_size = []
group_size = 5000
for i in range(0, group_size):
    single_list = []
    single_list_length = 0
    max_size = random.randint(200, 400)
    for j in range(0, max_size):
        index = random.randint(0, pattern_string_group_size - 1)
        length = pattern_length_group[index]
        single_list.append(index)
        single_list_length = single_list_length + length
    
    dirty_text_list_group.append(single_list)
    dirty_text_list_size.append(single_list_length)


min_list_length = min(dirty_text_list_size)
while (dirty_text_length - dirty_text_count) > min_list_length:
    index = random.randint(0, group_size - 1)
    single_list = dirty_text_list_group[index]
    single_list_length = dirty_text_list_size[index]

    if(dirty_text_count + single_list_length > dirty_text_length):
        continue
    else:
        random_pattern_id_group.extend(single_list)
        dirty_text_count = dirty_text_count + single_list_length

    sys.stdout.write("%u / %u = %f%% \r" % (dirty_text_count, dirty_text_length, float(dirty_text_count) / float(dirty_text_length)))
    # print "%u / %u = %f%% \r" % (dirty_text_count, dirty_text_length, float(dirty_text_count) / float(dirty_text_length))


print ""
min_pattern_length = min(pattern_length_group)
while (dirty_text_length - dirty_text_count) > min_pattern_length:
    index = random.randint(0, pattern_string_group_size - 1)
    length = pattern_length_group[index]

    if(dirty_text_count + length > dirty_text_length):
        continue
    else:
        random_pattern_id_group.append(index)
        dirty_text_count = dirty_text_count + length

    sys.stdout.write("%u / %u = %f%% \r" % (dirty_text_count, dirty_text_length, float(dirty_text_count) / float(dirty_text_length)))
    # print "%u / %u = %f%% \r" % (dirty_text_count, dirty_text_length, float(dirty_text_count) / float(dirty_text_length))


print ""
not_dirty_text_length = trace_size - dirty_text_count
insert_position_group = []
random_pattern_size = len(random_pattern_id_group)
for i in range(0, random_pattern_size):
    index = random.randint(0, not_dirty_text_length)
    insert_position_group.append(index)
    # print index

# print insert_position_group
insert_position_group = sorted(insert_position_group)
# print insert_position_group

# sys.exit(0)

dirty_percent_string = "%02u" % (dirty_rate * 100)
trace_file_name = 'trace_p%u_d%s.log' % (count_pattern, dirty_percent_string)
write_trace_file = io.open(trace_file_name, 'w')
# write_trace_file = io.open('trace.log', 'w')

trace_content = ""
insert_index = 0
for i in range(0, not_dirty_text_length):
    random_value = random.randint(0, 255)
    random_char = "%02x" % random_value
    write_trace_file.write(unicode(random_char))
    # trace_content = trace_content + random_char

    while insert_index < len(insert_position_group):
        # print insert_index
        # print len(insert_position_group)
        if i == insert_position_group[insert_index]:
            string_index = random_pattern_id_group[insert_index]
            random_string = pattern_string_group[string_index]
            random_length = pattern_length_group[string_index]
            write_trace_file.write(unicode(random_string))
            # write_trace_file.write(unicode("=" * random_length))
            # trace_content = trace_content + random_string
            # trace_content = trace_content + "*" * random_length
            insert_index = insert_index + 1
        else:
            break

    sys.stdout.write("%u / %u = %f%% \r" % (i, not_dirty_text_length, float(i) / float(not_dirty_text_length)))
    # print "%u / %u = %f%% \r" % (i, not_dirty_text_length, float(i) / float(not_dirty_text_length))


print ""
print dirty_text_count
print dirty_text_length
print not_dirty_text_length
print trace_content
print len(trace_content)
