/*
 *
 * DOES NOT USE BUILT-IN DETECTION FUNCTION
 alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg: "WEB-CLIENT Microsoft Excel malformed OBJ record arbitrary code execution attempt"; flow: to_client, established; flowbits: isset, file.xls; content:"|5D 00|"; metadata: policy balanced-ips drop, policy security-ips drop, service http; classtype:attempted-user; reference:cve,2008-4264; reference:url,technet.microsoft.com/en-us/security/bulletin/MS08-074; sid:15117;)
*/
/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 
 * This file is autogenerated via rules2c, by Brian Caswell <bmc@sourcefire.com>
 */
/* this rule does NOT use the built-in detection function */

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_WRAP(code) code
#else
#define DEBUG_WRAP(code)
#endif


/* declare detection functions */
int rule15117eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_client; */
static FlowFlags rule15117flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule15117option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule15117flow0
    }
};
/* flowbits:isset "file.xls"; */
static FlowBitsInfo rule15117flowbits1 =
{
    "file.xls",
    FLOWBIT_ISSET,
    0,
};

static RuleOption rule15117option1 =
{
    OPTION_TYPE_FLOWBIT,
    {
        &rule15117flowbits1
    }
};
// content:"|5D 00|"; 
static ContentInfo rule15117content2 = 
{
    (uint8_t *) "|5D 00|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule15117option2 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule15117content2
    }
};

/* references for sid 1 */
/* reference: cve "2008-4264"; */
static RuleReference rule15117ref1 = 
{
    "cve", /* type */
    "2008-4264" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS08-074"; */
static RuleReference rule15117ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS08-074" /* value */
};

static RuleReference *rule15117refs[] =
{
    &rule15117ref1,
    &rule15117ref2,
    NULL
};
/* metadata for sid 1 */
/* metadata:service http, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule15117service1 = 
{
    "service http"
};


//static RuleMetaData rule15117policy1 = 
//{
//    "policy balanced-ips drop"
//};
//
//static RuleMetaData rule15117policy2 = 
//{
//    "policy security-ips drop"
//};

static RuleMetaData rule15117policy3 = 
{
    "policy max-detect-ips drop"
};

static RuleMetaData *rule15117metadata[] =
{
    &rule15117service1,
//    &rule15117policy1,
//    &rule15117policy2,
    &rule15117policy3,
    NULL
};
RuleOption *rule15117options[] =
{
    &rule15117option0,
    &rule15117option1,
    &rule15117option2,
    NULL
};

Rule rule15117 = {
   
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
       15117, /* sigid */
       12, /* revision */
   
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "FILE-OFFICE Microsoft Excel malformed OBJ record arbitrary code execution attempt",     /* message */
       rule15117refs /* ptr to references */
       ,rule15117metadata
   },
   rule15117options, /* ptr to rule options */
   &rule15117eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule15117eval(void *p) {
   const uint8_t *cursor_normal = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;
   const uint8_t *end_of_payload, *end_of_record;
   const uint8_t *cursor_nextloop;

   uint16_t obj_record_len;
   uint16_t ft;
   uint16_t cb;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
    
   // flow:established, to_client;
   if(checkFlow(p, rule15117options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   // flowbits:isset "file.xls";
   if(processFlowbits(p, rule15117options[1]->option_u.flowBit) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   // content:"|5D 00|";
   while(contentMatch(p, rule15117options[2]->option_u.content, &cursor_normal) > 0) {
      // If we need to search again, we want to start here in case we matched on data
      cursor_nextloop = cursor_normal; 

      // The first sub-record is 22 bytes plus the 2-byte obj_record_len
      if(cursor_normal + 24 >= end_of_payload)
         return RULE_NOMATCH;

      // Verify the first sub-record is type 0x0015 to reduce false positives.
      // This also adds false negatives, but otherwise the FP rate will be
      // astronomical.
      if((*(cursor_normal + 2) != 0x15) || (*(cursor_normal + 3) != 0x00))
         continue;

      // Verify the second sub-record is type 0x0012 to reduce false positives.
      if((*(cursor_normal + 4) != 0x12) || (*(cursor_normal + 5) != 0x00))
         continue;

      // Verify the last 3 bytes in the sub-record are null
      if(*(cursor_normal + 12) != 0)
         continue;
      if(*((uint16_t*)(cursor_normal + 13)) !=0)  // byte order doesn't matter here
         continue;

      obj_record_len = *cursor_normal++;
      obj_record_len |= *cursor_normal++ << 8;

      DEBUG_WRAP(printf("obj_record_len=0x%04x\n", obj_record_len));

      if(obj_record_len == 0) // if 0, not a valid record
         return RULE_NOMATCH;

      obj_record_len -= 4; // minus 2 for tag and 2 for size to properly find end_of_record

      //the record with no variable length data will not exceed 160 ish bytes. Use this check to eliminate FPs
      if(obj_record_len > 1000)
         continue;

      // Stop at the end of the record or when we run out of data
      end_of_record = (cursor_normal + obj_record_len >= end_of_payload) ? end_of_payload : cursor_normal + obj_record_len;

      // Skip the first sub-record, it's always 22 bytes. Bounds check was prior to this
      cursor_normal += 22;

      while(cursor_normal + 2 < end_of_record) {

         ft = *cursor_normal++;
         ft |= *cursor_normal++ << 8;
                 
         DEBUG_WRAP(printf("FT: 0x%02x\n", ft));
                    
         if(ft > 0x15)
            return RULE_MATCH;

         if(cursor_normal + 2 > end_of_record)
            break; // Get out of the loop, look for another OBJ record

         cb = *cursor_normal++;
         cb |= *cursor_normal++ << 8;

         DEBUG_WRAP(printf("  cb: 0x%02x\n", cb));

         cursor_normal += cb;
      }

      DEBUG_WRAP(printf("Searching for new OBJ record (|5D 00|)\n"));

      // Start our next search immediately after the last "|5D 00|"
      cursor_normal = cursor_nextloop;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule15117,
    NULL
};
*/