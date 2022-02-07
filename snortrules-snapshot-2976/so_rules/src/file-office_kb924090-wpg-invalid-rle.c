
/* Does NOT use the built-in detection function!! 
# alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg:"WEB-CLIENT WordPerfect Graphics file invalid RLE buffer overflow attempt"; flow:to_client,established; content:"|FF 57 50 43 00 00 00 10 01 16 01 00 00 00|"; reference:cve,2008-3460; reference:url,technet.microsoft.com/en-us/security/bulletin/ms08-044; classtype:attempted-user; sid:13958; rev:1;)
*/
/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 
 * This file is autogenerated via rules2c, by Brian Caswell <bmc@sourcefire.com>
 */


#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_WRAP(code) code
#else
#define DEBUG_WRAP(code)
#endif

/* declare detection functions */
int rule13958eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_client; */
static FlowFlags rule13958flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule13958option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule13958flow0
    }
};
// content:"|FF|WPC|00 00 00 10 01 16 01 00 00|"; 
static ContentInfo rule13958content1 = 
{
    (uint8_t *) "|FF|WPC|10 00 00 00 01 16 01 00 00 00|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule13958option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule13958content1
    }
};

/* references for sid 13958 */
/* reference: cve "2008-3460"; */
static RuleReference rule13958ref1 = 
{
    "cve", /* type */
    "2008-3460" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/ms08-044"; */
static RuleReference rule13958ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/ms08-044" /* value */
};

static RuleReference *rule13958refs[] =
{
    &rule13958ref1,
    &rule13958ref2,
    NULL
};
/* metadata for sid 13958 */
/* metadata:; */


static RuleMetaData *rule13958metadata[] =
{
    NULL
};
RuleOption *rule13958options[] =
{
    &rule13958option0,
    &rule13958option1,
    NULL
};

Rule rule13958 = {
   
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "$HTTP_PORTS", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13958, /* sigid */
       7, /* revision */
   
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "FILE-OFFICE WordPerfect Graphics file invalid RLE buffer overflow attempt",     /* message */
       rule13958refs /* ptr to references */
       ,rule13958metadata
   },
   rule13958options, /* ptr to rule options */
   &rule13958eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule13958eval(void *p) {
    const uint8_t *cursor_normal = 0, *beg_of_payload, *end_of_payload;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    uint8_t RecordType;
    uint32_t RecordSize;
    uint16_t Width, Height;

    DEBUG_WRAP(printf("rule13958eval (WPG invalid RLE) enter\n"));

    if(sp == NULL)
        return RULE_NOMATCH;

    if(sp->payload == NULL)
        return RULE_NOMATCH;
    
   // flow:established, to_client;
   if(checkFlow(p, rule13958options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   // content:"|FF|WPC|00 00 00 10 01 16 01 00 00|";
   if(contentMatch(p, rule13958options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   cursor_normal += 2; // skip over reserved word   

   // Now we're at the beginning of records.  Jump over records until we
   // find a BitMap Type 1 record.  Jumping over records involves figuring
   // out how many bytes are in the header and then jumping over the
   // specified record.  
   // Useful info here: http://www.fileformat.info/format/wpg/

   // On second thought, those guys suck.  Check out the source at libwpg project
   // http://libwpg.sourceforge.net/
   // Of key interest is lib/WPGXParser.cpp readVariableLengthInteger()

   // Note FileFormat.info's biggest failing is the file is little endian

   // Get the Image Size from RECPREFIX. 
   if(cursor_normal + 8 >= end_of_payload)
      return RULE_NOMATCH;

   if(*cursor_normal != 0x0f) {
      return RULE_NOMATCH;
   } else {
      cursor_normal += 4;
      Width = *cursor_normal++;
      Width |= *cursor_normal++ << 8;
      Height = *cursor_normal++;
      Height |= *cursor_normal++ << 8;
      DEBUG_WRAP(printf("Image Width=0x%02x Height=0x%02x\n", Width, Height));
   }

   while(cursor_normal + 6 < end_of_payload) { 
      RecordType = *cursor_normal++;

      if(*cursor_normal == 0xFF) { // multibyte size
         cursor_normal++;
         RecordSize = *cursor_normal++;
         RecordSize |= *cursor_normal++ << 8;
         if(RecordSize & 0x8000) { // dword size
            RecordSize &= 0x7FFF; 
            RecordSize <<= 16;     // If four-byte size, 0xABCD is stored as BADC
            RecordSize |= *cursor_normal++ ;
            RecordSize |= *cursor_normal++ << 8;
         }
      } else {
         RecordSize = *cursor_normal++;
      }

      DEBUG_WRAP(printf("RecordType = 0x%02x, RecordSize = 0x%04x\n", RecordType, RecordSize));

      if(RecordType == 0x0b) { // BitMap Type 1
         // WPG files aren't used anymore, and BitMap Type 1 is deprecated.  Unless we
         // start getting FP reports, we're just going to alert on the presence of
         // this RecordType
         DEBUG_WRAP(printf("Found BitMap Type 1\n"));

         return RULE_MATCH;
      } else { // Skip record
         cursor_normal += RecordSize;
      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule13958,
    NULL
};
*/

