/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 
 * This file is autogenerated via rules2c, by Brian Caswell <bmc@sourcefire.com>
 */

/*
*   alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg:"EXPLOIT Microsoft GDI malformed metarecord buffer overflow attempt"; flow: to_client, established; content:"|D7 CD C6 9A 00 00|"; content:"|38 05|" distance: 0; classtype: attempted-user; metadata: policy balanced-ips drop, policy security-ips drop, service http; sid: 14251;)
*/


#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util.h"

/* declare detection functions */
int rule14251eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_client; */
static FlowFlags rule14251flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule14251option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule14251flow0
    }
};
// content:"|D7 CD C6 9A 00 00|"; 
static ContentInfo rule14251content1 = 
{
    (uint8_t *) "|D7 CD C6 9A 00 00|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule14251option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule14251content1
    }
};
// content:"|22|8|05 22| distance|3A| 0"; 
static ContentInfo rule14251content2 = 
{
    (uint8_t *) "|38 05|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule14251option2 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule14251content2
    }
};

/* references for sid 14251 */
/* reference: cve "2008-3014"; */
static RuleReference rule14251ref1 = 
{
    "cve", /* type */
    "2008-3014" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS08-052"; */
static RuleReference rule14251ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS08-052" /* value */
};

static RuleReference *rule14251refs[] =
{
    &rule14251ref1,
    &rule14251ref2,
    NULL
};

/* metadata for sid 14251 */
/* metadata:service http, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule14251service1 = 
{
    "service http"
};


//static RuleMetaData rule14251policy1 = 
//{
//    "policy balanced-ips drop"
//};
//
//static RuleMetaData rule14251policy2 = 
//{
//    "policy security-ips drop"
//};

static RuleMetaData rule14251policy3 = 
{
    "policy max-detect-ips drop"
};

static RuleMetaData *rule14251metadata[] =
{
    &rule14251service1,
//    &rule14251policy1,
//    &rule14251policy2,
    &rule14251policy3,
    NULL
};

RuleOption *rule14251options[] =
{
    &rule14251option0,
    &rule14251option1,
    &rule14251option2,
    NULL
};

Rule rule14251 = {
   
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
       14251, /* sigid */
       10, /* revision */
   
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS Microsoft GDI malformed metarecord buffer overflow attempt",     /* message */
       rule14251refs /* ptr to references */
       ,rule14251metadata
   },
   rule14251options, /* ptr to rule options */
   &rule14251eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule14251eval(void *p) {
   const uint8_t *cursor_normal = 0;
   const uint8_t *beg_of_payload, *end_of_payload;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   uint16_t polyCount;
   uint32_t recordSize;
   uint32_t estimatedSize;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
    
   // flow:established, to_client;
   if(checkFlow(p, rule14251options[0]->option_u.flowFlags) > 0 ) {
      // content:"|D7 CD C6 9A 00 00|";
      if(contentMatch(p, rule14251options[1]->option_u.content, &cursor_normal) > 0) {
         // content:"| 38 05|";
         if(contentMatch(p, rule14251options[2]->option_u.content, &cursor_normal) > 0) {

            if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
               return RULE_NOMATCH;


            // Because we have a six byte content check and a follow up two byte
            //    check at distance: 0, we don't need to underflow check here.
            // We're comparing the declared recordsize to an estimation of 
            //    the probable size of the construct.
            if(cursor_normal + 2 < end_of_payload) {
               polyCount = read_little_16(cursor_normal);

               recordSize = read_little_32(cursor_normal - 6);

               estimatedSize = polyCount * 5 + 3;
               if(recordSize < estimatedSize) {
                  return RULE_MATCH;
               }
            }
         }
      }
   }
   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule14251,
    NULL
};
*/

