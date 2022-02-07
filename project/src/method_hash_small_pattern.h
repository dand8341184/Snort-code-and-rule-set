#ifndef __HEADER_METHOD_HASH_SMALL_PATTERN__
#define __HEADER_METHOD_HASH_SMALL_PATTERN__

extern unsigned int* lengthOneTable;

extern unsigned int* lengthTwoTable;

void initSmallPattern();

void addSmallPattern(unsigned char* pattern, unsigned int length);

void searchSmallPattern(unsigned int char1, unsigned int char2, unsigned int char3);

#endif