import os
import sys
import io
import re

# rulesFolderPath = os.path.join('../custom_rules')
rulesFolderPath = os.path.join('/home/user/workspace/snort/resource/snortrules-snapshot-2976/rules')

filesAndFoldersList = os.listdir(rulesFolderPath)

# Generate list contained path of files.
# [file1_path_string, file2_path_string, ...]

fileNameList = [];
for fileName in filesAndFoldersList:
    filePath = os.path.join(rulesFolderPath, fileName)
    if os.path.isfile(filePath):
        # print fileName
        fileNameList.append(fileName)

# Generate list contained lines of files, file name, file path
# [{    file_name: file1_name_string,
#       file_path: file1_path_string,
#       line_text: line1_of_file1_string
#  },
#  {    file_name: file1_name_string,
#       file_path: file1_path_string,
#       line_text: line2_of_file1_string
#  },
#  {    file_name: file2_name_string,
#       file_path: file2_path_string,
#       line_text: line1_of_file2_string
#  }...]

lineTextList = []	
for fileName in fileNameList:
    filePath = os.path.join(rulesFolderPath, fileName)
    file = io.open(filePath, 'r', encoding='utf-8')
    
    for text in file.readlines():
        entry = {'file_name': fileName, 'file_path': filePath, 'line_text': text }
        lineTextList.append(entry)

# Generate list contained lines of files, file name, file path
# [{    file_name: file1_name_string,
#       file_path: file1_path_string,
#       text: line1_of_file1_string,
#       is_comment: True
#  },
#  {    file_name: file1_name_string,
#       file_path: file1_path_string,
#       text: line2_of_file1_string,
#       is_comment: False
#  },
#  {    file_name: file2_name_string,
#       file_path: file2_path_string,
#       text: line1_of_file2_string,
#       is_comment: True
#  },
#  ...]

def checkSnortRuleAction(lineText):
    if lineText.startswith('# alert') or lineText.startswith('# log') or \
        lineText.startswith('# pass') or lineText.startswith('# activate') or \
        lineText.startswith('# dynamic') or lineText.startswith('# drop') or \
        lineText.startswith('# reject') or lineText.startswith('# sdrop'):
        return 0
    elif lineText.startswith('alert') or lineText.startswith('log') or \
        lineText.startswith('pass') or lineText.startswith('activate') or \
        lineText.startswith('dynamic') or lineText.startswith('drop') or \
        lineText.startswith('reject') or lineText.startswith('sdrop'):
        return 1
    else:
        return 2


ruleTextList = []
for entry in lineTextList:
    checkResult = checkSnortRuleAction(entry['line_text'])
    if checkResult == 0:
        entry['line_text'] = entry['line_text'][2:]
    elif checkResult == 1:
        entry['line_text'] = entry['line_text']
    elif checkResult == 2:
        continue;
        
    newEntry = {
        'file_name': entry['file_name'], 
        'file_path': entry['file_path'],
        'text': entry['line_text'],
        'is_comment': True,
    }
    ruleTextList.append(newEntry)    

# Generate list contained Snort rules.
# Example rule: alert tcp  any any -> any any (content:"ABC";)
#
# Result:
# [{    mode: 'alert',
#       protocol: 'tcp',
#       source_ip: 'any',
#       source_port: 'any',
#       traffic_flow: '->',
#       destination_ip: 'any',
#       destination_port: 'any',
#       options: [{
#               name: 'content',
#               value: '"ABC"',
#               not: False
#           },
#           ...]
#  },
# ...]

ruleList = []
for entry in ruleTextList:
    ruleHeaderEnd = entry['text'].find('(')
    ruleOptionStart = ruleHeaderEnd + 1
    ruleOptionEnd = entry['text'].rfind(')')
    ruleHeaderText = entry['text'][:ruleHeaderEnd];
    ruleOptionText = entry['text'][ruleOptionStart:ruleOptionEnd]
    
    ruleHeader = ruleHeaderText.split(' ');
    
    newEntry = entry
    newEntry['mode'] = ruleHeader[0]
    newEntry['protocol'] = ruleHeader[1]
    newEntry['source_ip'] = ruleHeader[2]
    newEntry['source_port'] = ruleHeader[3]
    newEntry['traffic_flow'] = ruleHeader[4]
    newEntry['destination_ip'] = ruleHeader[5]
    newEntry['destination_port'] = ruleHeader[6]
    
    newEntry['pattern_set'] = entry['file_name']
    newEntry['rule_text'] = entry['text']

    optionGroup = []
    while ruleOptionText != '':
        optionValueEnd = ruleOptionText.find(';')
        optionNameEnd = ruleOptionText.find(':')

        if optionValueEnd < optionNameEnd:
            option = {}
            option['name'] = ruleOptionText[:optionValueEnd]
            option['value'] = ''
            option['not'] = False
            optionGroup.append(option)
            ruleOptionText = ruleOptionText[optionValueEnd + 1:]
            ruleOptionText = ruleOptionText.strip()
            continue

        optionName = ruleOptionText[:optionNameEnd]

        ruleOptionText = ruleOptionText[optionNameEnd + 1:]
        ruleOptionText = ruleOptionText.strip()
        
        if ruleOptionText[0] == '!':
            optionNot = True
            ruleOptionText = ruleOptionText[1:]
        else:
            optionNot = False

        if ruleOptionText[0] == '"':
            ruleOptionText = ruleOptionText[1:]
        
            ruleSingleOptionEnd = 0
            subRuleOptionText = ruleOptionText
            while True:
                quoteIndex = subRuleOptionText.find('"')
            
                if subRuleOptionText[quoteIndex - 1] != '\\':
                    ruleSingleOptionEnd += quoteIndex
                    break
            
                continuousSlashEnd = quoteIndex - 1
                continuousSlashStart = continuousSlashEnd
                while subRuleOptionText[continuousSlashStart] == '\\':
                    continuousSlashStart -= 1
            
                if (continuousSlashEnd - continuousSlashStart) % 2 == 0:
                    ruleSingleOptionEnd += quoteIndex
                    break
            
                subRuleOptionText = subRuleOptionText[quoteIndex + 1:]
                ruleSingleOptionEnd += quoteIndex + 1
        
            optionValue = ruleOptionText[:ruleSingleOptionEnd]
            ruleOptionText = ruleOptionText[ruleSingleOptionEnd + 1:]

            semicolonIndex = ruleOptionText.find(';')
            ruleOptionText = ruleOptionText[semicolonIndex + 1:]
            ruleOptionText = ruleOptionText.strip()
        else:
            semicolonIndex = ruleOptionText.find(';')
            optionValue = ruleOptionText[:semicolonIndex]
            optionValue = optionValue.strip()
            ruleOptionText = ruleOptionText[semicolonIndex + 1:]
            ruleOptionText = ruleOptionText.strip()
        
        option = {}
        option['name'] = optionName
        option['value'] = optionValue
        option['not'] = optionNot
        optionGroup.append(option)

    newEntry['options'] = optionGroup
    ruleList.append(newEntry)


def convert_content_to_hex_array(content):    
    hex_string = []
    index = 0
    length = len(content)
    state = 0
    while index < length:
        hex_value = 0
        char = content[index]

        if state == 0 and char == "|":
            state = 1
        elif state == 1 and char == "|":
            state = 0
        elif state == 1 and char == " ":
            hex_value = int(content[index+1:index+3], 16)
            hex_string.append(hex_value)
            index += 2
        elif state == 1:
            hex_value = int(content[index:index+2], 16)
            hex_string.append(hex_value)
            index += 1
        else:
            hex_value = ord(content[index])
            hex_string.append(hex_value)

        index += 1

    return hex_string


for rule in ruleList:
    for option in rule['options']:
        if option['name'] == "content":
            option['content_value'] = convert_content_to_hex_array(option['value'])
            option['content_length'] = len(option['content_value'])

# === Finish Snort Rule Processing =========================================================================


ruleFileList = {}
for rule in ruleList:
    patternSet = rule['pattern_set']
    if patternSet not in ruleFileList:
        ruleFileList[patternSet] = {}
        ruleFileList[patternSet]["count_rule"] = 0
        ruleFileList[patternSet]["count_ip"] = 0
        ruleFileList[patternSet]["count_icmp"] = 0
        ruleFileList[patternSet]["count_udp"] = 0
        ruleFileList[patternSet]["count_tcp"] = 0
        ruleFileList[patternSet]["count_content"] = 0
        ruleFileList[patternSet]["count_pcre"] = 0
        ruleFileList[patternSet]["content_min"] = 1000000
        ruleFileList[patternSet]["content_max"] = 0
        ruleFileList[patternSet]["content_sum"] = 0
        ruleFileList[patternSet]["count_alert"] = 0
        ruleFileList[patternSet]["count_log"] = 0
        ruleFileList[patternSet]["count_passive"] = 0
        ruleFileList[patternSet]["count_activate"] = 0
        ruleFileList[patternSet]["count_dynamic"] = 0
        ruleFileList[patternSet]["count_drop"] = 0
        ruleFileList[patternSet]["count_reject"] = 0
        ruleFileList[patternSet]["count_sdrop"] = 0

    
    ruleFileList[patternSet]["count_rule"] += 1
    for option in rule['options']:
        if option['name'] == "content":
            ruleFileList[patternSet]["count_content"] += 1

            if option['content_length'] > ruleFileList[patternSet]["content_max"]:
                ruleFileList[patternSet]["content_max"] = option['content_length']
            
            if option['content_length'] < ruleFileList[patternSet]["content_min"]:
                ruleFileList[patternSet]["content_min"] = option['content_length']
            
            ruleFileList[patternSet]["content_sum"] += option['content_length']

        elif option['name'] == "pcre":
            ruleFileList[patternSet]["count_pcre"] += 1
        
    if rule['protocol'] == "ip":
        ruleFileList[patternSet]["count_ip"] += 1
    elif rule['protocol'] == "icmp":
        ruleFileList[patternSet]["count_icmp"] += 1
    elif rule['protocol'] == "udp":
        ruleFileList[patternSet]["count_udp"] += 1
    elif rule['protocol'] == "tcp":
        ruleFileList[patternSet]["count_tcp"] += 1

    if rule['mode'] == "alert":
        ruleFileList[patternSet]["count_alert"]  += 1
    elif rule['mode'] == "log":
        ruleFileList[patternSet]["count_log"]  += 1
    elif rule['mode'] == "passive":
        ruleFileList[patternSet]["count_passive"]  += 1
    elif rule['mode'] == "activate":
        ruleFileList[patternSet]["count_activate"]  += 1
    elif rule['mode'] == "dynamic":
        ruleFileList[patternSet]["count_dynamic"]  += 1
    elif rule['mode'] == "drop":
        ruleFileList[patternSet]["count_drop"]  += 1
    elif rule['mode'] == "reject":
        ruleFileList[patternSet]["count_reject"]  += 1
    elif rule['mode'] == "sdrop":
        ruleFileList[patternSet]["count_sdrop"]  += 1


print "FILE,RULE,IP,ICMP,UDP,TCP,CONTENT,PCRE,CONTENT_MAX,CONTENT_MIN,CONTENT_SUM,ALERT,LOG,PASSIVE,ACTIVATE,DYNAMIC,DROP,REJECT,SDROP"
for name, ruleFile in ruleFileList.items():
    print "%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u" % (name, 
        ruleFile["count_rule"], ruleFile["count_ip"], 
        ruleFile["count_icmp"], ruleFile["count_udp"],
        ruleFile["count_tcp"], ruleFile["count_content"],
        ruleFile["count_pcre"], ruleFile["content_max"],
        ruleFile["content_min"], ruleFile["content_sum"],
        ruleFile["count_alert"], ruleFile["count_log"],
        ruleFile["count_passive"], ruleFile["count_activate"],
        ruleFile["count_dynamic"], ruleFile["count_drop"],
        ruleFile["count_reject"], ruleFile["count_sdrop"])


