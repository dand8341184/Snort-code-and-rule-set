import sys
import random

pattern_length = int(sys.argv[1])
if pattern_length == 0:
    print "Pattern Length is zero."
    sys.exit(0)

pattern_file_name = "pattern_%02d" % (pattern_length)
pattern_file = open(pattern_file_name, "w")

for i in range(0, 3000):
    pattern = ""
    for position in range(0, pattern_length):
         pattern += "%02x" % random.randint(0, 255)
    
    pattern_file.write(pattern + "\n")

# print pattern_file_name
