# Snort-code-and-rule-set
Accelerating Aho-Corasick Algorithm using Odd-Even Sub Patterns to improve Snort Intrusion Detection System

Abstract—Snort is an open source network intrusion detection system (IDS) that detects if a specific information associated with to a malicious attack is contained in each network packet. In this paper, we propose an Odd-Even AC (OE-AC) structure built from odd and even subpatterns generated from the patterns used in the original AC algorithm to improve the search performance of Snort. We use a two-phase search architecture. In the first phase, only odd characters of input text are used to the next state in the  AC state machine. In the second phase, a full match is performed to determine if sub input text matches patterns. The experiment results show that under different Snort rule files, the search performance based on OE-AC is improved 1.11 to 1.80 times compared with original AC, and OE-AC approximates about 92% to 101% of the original AC memory.

Keywords: Pattern matching; Intrusion detection system; Aho-Corasick; Snort
