import sys
import os
import string

def analyticsMainNdb():
    ac_file = open('../logs/main.ndb', 'r')

    textGroup = []
    for text in ac_file.readlines():
        textGroup.append(text.strip())

    symbolArray = ['A', 'C', 'B', 'E', 'D', 'F', ')', '(', '*', '-', '{', '}', '|', '?']
    for text in textGroup:
        stringGroup = text.split(':')
        pattern = stringGroup[3]

        isRegex = 0
        for char in pattern:
            if char in symbolArray:
                isRegex = 1
    
        if isRegex == 0:
            print pattern


def analyticsSimplePattern():
    patternFile = open('../logs/clamav_simple_pattern.log', 'r')
    
    patternGroup = []
    for pattern in patternFile.readlines():
        patternGroup.append(pattern.strip())

    patternGroup.sort(key=len, reverse=False)
    
    minLength = len(min(patternGroup, key=len))
    maxLength = len(max(patternGroup, key=len))
    # print minLength
    # print maxLength

    for pattern in patternGroup:
        if len(pattern) > 2:
            print pattern


# analyticsMainNdb()
analyticsSimplePattern()


