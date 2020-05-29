/* Output of mkstrtable.awk.  DO NOT EDIT.  */

/* err-codes.h - List of error codes and their description.
   Copyright (C) 2003, 2004 g10 Code GmbH

   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with libgpg-error; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */


/* The purpose of this complex string table is to produce
   optimal code with a minimum of relocations.  */

static const char msgstr[] =
  gettext_noop ("Success") "\0"
  gettext_noop ("General error") "\0"
  gettext_noop ("Unknown packet") "\0"
  gettext_noop ("Unknown version in packet") "\0"
  gettext_noop ("Invalid public key algorithm") "\0"
  gettext_noop ("Invalid digest algorithm") "\0"
  gettext_noop ("Bad public key") "\0"
  gettext_noop ("Bad secret key") "\0"
  gettext_noop ("Bad signature") "\0"
  gettext_noop ("No public key") "\0"
  gettext_noop ("Checksum error") "\0"
  gettext_noop ("Bad passphrase") "\0"
  gettext_noop ("Invalid cipher algorithm") "\0"
  gettext_noop ("Keyring open") "\0"
  gettext_noop ("Invalid packet") "\0"
  gettext_noop ("Invalid armor") "\0"
  gettext_noop ("No user ID") "\0"
  gettext_noop ("No secret key") "\0"
  gettext_noop ("Wrong secret key used") "\0"
  gettext_noop ("Bad session key") "\0"
  gettext_noop ("Unknown compression algorithm") "\0"
  gettext_noop ("Number is not prime") "\0"
  gettext_noop ("Invalid encoding method") "\0"
  gettext_noop ("Invalid encryption scheme") "\0"
  gettext_noop ("Invalid signature scheme") "\0"
  gettext_noop ("Invalid attribute") "\0"
  gettext_noop ("No value") "\0"
  gettext_noop ("Not found") "\0"
  gettext_noop ("Value not found") "\0"
  gettext_noop ("Syntax error") "\0"
  gettext_noop ("Bad MPI value") "\0"
  gettext_noop ("Invalid passphrase") "\0"
  gettext_noop ("Invalid signature class") "\0"
  gettext_noop ("Resources exhausted") "\0"
  gettext_noop ("Invalid keyring") "\0"
  gettext_noop ("Trust DB error") "\0"
  gettext_noop ("Bad certificate") "\0"
  gettext_noop ("Invalid user ID") "\0"
  gettext_noop ("Unexpected error") "\0"
  gettext_noop ("Time conflict") "\0"
  gettext_noop ("Keyserver error") "\0"
  gettext_noop ("Wrong public key algorithm") "\0"
  gettext_noop ("Tribute to D. A.") "\0"
  gettext_noop ("Weak encryption key") "\0"
  gettext_noop ("Invalid key length") "\0"
  gettext_noop ("Invalid argument") "\0"
  gettext_noop ("Syntax error in URI") "\0"
  gettext_noop ("Invalid URI") "\0"
  gettext_noop ("Network error") "\0"
  gettext_noop ("Unknown host") "\0"
  gettext_noop ("Selftest failed") "\0"
  gettext_noop ("Data not encrypted") "\0"
  gettext_noop ("Data not processed") "\0"
  gettext_noop ("Unusable public key") "\0"
  gettext_noop ("Unusable secret key") "\0"
  gettext_noop ("Invalid value") "\0"
  gettext_noop ("Bad certificate chain") "\0"
  gettext_noop ("Missing certificate") "\0"
  gettext_noop ("No data") "\0"
  gettext_noop ("Bug") "\0"
  gettext_noop ("Not supported") "\0"
  gettext_noop ("Invalid operation code") "\0"
  gettext_noop ("Timeout") "\0"
  gettext_noop ("Internal error") "\0"
  gettext_noop ("EOF (gcrypt)") "\0"
  gettext_noop ("Invalid object") "\0"
  gettext_noop ("Provided object is too short") "\0"
  gettext_noop ("Provided object is too large") "\0"
  gettext_noop ("Missing item in object") "\0"
  gettext_noop ("Not implemented") "\0"
  gettext_noop ("Conflicting use") "\0"
  gettext_noop ("Invalid cipher mode") "\0"
  gettext_noop ("Invalid flag") "\0"
  gettext_noop ("Invalid handle") "\0"
  gettext_noop ("Result truncated") "\0"
  gettext_noop ("Incomplete line") "\0"
  gettext_noop ("Invalid response") "\0"
  gettext_noop ("No agent running") "\0"
  gettext_noop ("Agent error") "\0"
  gettext_noop ("Invalid data") "\0"
  gettext_noop ("Unspecific Assuan server fault") "\0"
  gettext_noop ("General Assuan error") "\0"
  gettext_noop ("Invalid session key") "\0"
  gettext_noop ("Invalid S-expression") "\0"
  gettext_noop ("Unsupported algorithm") "\0"
  gettext_noop ("No pinentry") "\0"
  gettext_noop ("pinentry error") "\0"
  gettext_noop ("Bad PIN") "\0"
  gettext_noop ("Invalid name") "\0"
  gettext_noop ("Bad data") "\0"
  gettext_noop ("Invalid parameter") "\0"
  gettext_noop ("Wrong card") "\0"
  gettext_noop ("No dirmngr") "\0"
  gettext_noop ("dirmngr error") "\0"
  gettext_noop ("Certificate revoked") "\0"
  gettext_noop ("No CRL known") "\0"
  gettext_noop ("CRL too old") "\0"
  gettext_noop ("Line too long") "\0"
  gettext_noop ("Not trusted") "\0"
  gettext_noop ("Operation cancelled") "\0"
  gettext_noop ("Bad CA certificate") "\0"
  gettext_noop ("Certificate expired") "\0"
  gettext_noop ("Certificate too young") "\0"
  gettext_noop ("Unsupported certificate") "\0"
  gettext_noop ("Unknown S-expression") "\0"
  gettext_noop ("Unsupported protection") "\0"
  gettext_noop ("Corrupted protection") "\0"
  gettext_noop ("Ambiguous name") "\0"
  gettext_noop ("Card error") "\0"
  gettext_noop ("Card reset required") "\0"
  gettext_noop ("Card removed") "\0"
  gettext_noop ("Invalid card") "\0"
  gettext_noop ("Card not present") "\0"
  gettext_noop ("No PKCS15 application") "\0"
  gettext_noop ("Not confirmed") "\0"
  gettext_noop ("Configuration error") "\0"
  gettext_noop ("No policy match") "\0"
  gettext_noop ("Invalid index") "\0"
  gettext_noop ("Invalid ID") "\0"
  gettext_noop ("No SmartCard daemon") "\0"
  gettext_noop ("SmartCard daemon error") "\0"
  gettext_noop ("Unsupported protocol") "\0"
  gettext_noop ("Bad PIN method") "\0"
  gettext_noop ("Card not initialized") "\0"
  gettext_noop ("Unsupported operation") "\0"
  gettext_noop ("Wrong key usage") "\0"
  gettext_noop ("Nothing found") "\0"
  gettext_noop ("Wrong blob type") "\0"
  gettext_noop ("Missing value") "\0"
  gettext_noop ("Hardware problem") "\0"
  gettext_noop ("PIN blocked") "\0"
  gettext_noop ("Conditions of use not satisfied") "\0"
  gettext_noop ("PINs are not synced") "\0"
  gettext_noop ("Invalid CRL") "\0"
  gettext_noop ("BER error") "\0"
  gettext_noop ("Invalid BER") "\0"
  gettext_noop ("Element not found") "\0"
  gettext_noop ("Identifier not found") "\0"
  gettext_noop ("Invalid tag") "\0"
  gettext_noop ("Invalid length") "\0"
  gettext_noop ("Invalid key info") "\0"
  gettext_noop ("Unexpected tag") "\0"
  gettext_noop ("Not DER encoded") "\0"
  gettext_noop ("No CMS object") "\0"
  gettext_noop ("Invalid CMS object") "\0"
  gettext_noop ("Unknown CMS object") "\0"
  gettext_noop ("Unsupported CMS object") "\0"
  gettext_noop ("Unsupported encoding") "\0"
  gettext_noop ("Unsupported CMS version") "\0"
  gettext_noop ("Unknown algorithm") "\0"
  gettext_noop ("Invalid crypto engine") "\0"
  gettext_noop ("Public key not trusted") "\0"
  gettext_noop ("Decryption failed") "\0"
  gettext_noop ("Key expired") "\0"
  gettext_noop ("Signature expired") "\0"
  gettext_noop ("Encoding problem") "\0"
  gettext_noop ("Invalid state") "\0"
  gettext_noop ("Duplicated value") "\0"
  gettext_noop ("Missing action") "\0"
  gettext_noop ("ASN.1 module not found") "\0"
  gettext_noop ("Invalid OID string") "\0"
  gettext_noop ("Invalid time") "\0"
  gettext_noop ("Invalid CRL object") "\0"
  gettext_noop ("Unsupported CRL version") "\0"
  gettext_noop ("Invalid certificate object") "\0"
  gettext_noop ("Unknown name") "\0"
  gettext_noop ("A locale function failed") "\0"
  gettext_noop ("Not locked") "\0"
  gettext_noop ("Protocol violation") "\0"
  gettext_noop ("Invalid MAC") "\0"
  gettext_noop ("Invalid request") "\0"
  gettext_noop ("Unknown extension") "\0"
  gettext_noop ("Unknown critical extension") "\0"
  gettext_noop ("Locked") "\0"
  gettext_noop ("Unknown option") "\0"
  gettext_noop ("Unknown command") "\0"
  gettext_noop ("Not operational") "\0"
  gettext_noop ("No passphrase given") "\0"
  gettext_noop ("No PIN given") "\0"
  gettext_noop ("Not enabled") "\0"
  gettext_noop ("No crypto engine") "\0"
  gettext_noop ("Missing key") "\0"
  gettext_noop ("Too many objects") "\0"
  gettext_noop ("Limit reached") "\0"
  gettext_noop ("Not initialized") "\0"
  gettext_noop ("Missing issuer certificate") "\0"
  gettext_noop ("No keyserver available") "\0"
  gettext_noop ("Invalid elliptic curve") "\0"
  gettext_noop ("Unknown elliptic curve") "\0"
  gettext_noop ("Duplicated key") "\0"
  gettext_noop ("Ambiguous result") "\0"
  gettext_noop ("No crypto context") "\0"
  gettext_noop ("Wrong crypto context") "\0"
  gettext_noop ("Bad crypto context") "\0"
  gettext_noop ("Conflict in the crypto context") "\0"
  gettext_noop ("Broken public key") "\0"
  gettext_noop ("Broken secret key") "\0"
  gettext_noop ("Invalid MAC algorithm") "\0"
  gettext_noop ("Operation fully cancelled") "\0"
  gettext_noop ("Operation not yet finished") "\0"
  gettext_noop ("Buffer too short") "\0"
  gettext_noop ("Invalid length specifier in S-expression") "\0"
  gettext_noop ("String too long in S-expression") "\0"
  gettext_noop ("Unmatched parentheses in S-expression") "\0"
  gettext_noop ("S-expression not canonical") "\0"
  gettext_noop ("Bad character in S-expression") "\0"
  gettext_noop ("Bad quotation in S-expression") "\0"
  gettext_noop ("Zero prefix in S-expression") "\0"
  gettext_noop ("Nested display hints in S-expression") "\0"
  gettext_noop ("Unmatched display hints") "\0"
  gettext_noop ("Unexpected reserved punctuation in S-expression") "\0"
  gettext_noop ("Bad hexadecimal character in S-expression") "\0"
  gettext_noop ("Odd hexadecimal numbers in S-expression") "\0"
  gettext_noop ("Bad octal character in S-expression") "\0"
  gettext_noop ("Database is corrupted") "\0"
  gettext_noop ("Server indicated a failure") "\0"
  gettext_noop ("No name") "\0"
  gettext_noop ("No key") "\0"
  gettext_noop ("Legacy key") "\0"
  gettext_noop ("Request too short") "\0"
  gettext_noop ("Request too long") "\0"
  gettext_noop ("Object is in termination state") "\0"
  gettext_noop ("No certificate chain") "\0"
  gettext_noop ("Certificate is too large") "\0"
  gettext_noop ("Invalid record") "\0"
  gettext_noop ("The MAC does not verify") "\0"
  gettext_noop ("Unexpected message") "\0"
  gettext_noop ("Compression or decompression failed") "\0"
  gettext_noop ("A counter would wrap") "\0"
  gettext_noop ("Fatal alert message received") "\0"
  gettext_noop ("No cipher algorithm") "\0"
  gettext_noop ("Missing client certificate") "\0"
  gettext_noop ("Close notification received") "\0"
  gettext_noop ("Ticket expired") "\0"
  gettext_noop ("Bad ticket") "\0"
  gettext_noop ("Unknown identity") "\0"
  gettext_noop ("Bad certificate message in handshake") "\0"
  gettext_noop ("Bad certificate request message in handshake") "\0"
  gettext_noop ("Bad certificate verify message in handshake") "\0"
  gettext_noop ("Bad change cipher messsage in handshake") "\0"
  gettext_noop ("Bad client hello message in handshake") "\0"
  gettext_noop ("Bad server hello message in handshake") "\0"
  gettext_noop ("Bad server hello done message in hanshake") "\0"
  gettext_noop ("Bad finished message in handshake") "\0"
  gettext_noop ("Bad server key exchange message in handshake") "\0"
  gettext_noop ("Bad client key exchange message in handshake") "\0"
  gettext_noop ("Bogus string") "\0"
  gettext_noop ("Forbidden") "\0"
  gettext_noop ("Key disabled") "\0"
  gettext_noop ("Not possible with a card based key") "\0"
  gettext_noop ("Invalid lock object") "\0"
  gettext_noop ("True") "\0"
  gettext_noop ("False") "\0"
  gettext_noop ("General IPC error") "\0"
  gettext_noop ("IPC accept call failed") "\0"
  gettext_noop ("IPC connect call failed") "\0"
  gettext_noop ("Invalid IPC response") "\0"
  gettext_noop ("Invalid value passed to IPC") "\0"
  gettext_noop ("Incomplete line passed to IPC") "\0"
  gettext_noop ("Line passed to IPC too long") "\0"
  gettext_noop ("Nested IPC commands") "\0"
  gettext_noop ("No data callback in IPC") "\0"
  gettext_noop ("No inquire callback in IPC") "\0"
  gettext_noop ("Not an IPC server") "\0"
  gettext_noop ("Not an IPC client") "\0"
  gettext_noop ("Problem starting IPC server") "\0"
  gettext_noop ("IPC read error") "\0"
  gettext_noop ("IPC write error") "\0"
  gettext_noop ("Too much data for IPC layer") "\0"
  gettext_noop ("Unexpected IPC command") "\0"
  gettext_noop ("Unknown IPC command") "\0"
  gettext_noop ("IPC syntax error") "\0"
  gettext_noop ("IPC call has been cancelled") "\0"
  gettext_noop ("No input source for IPC") "\0"
  gettext_noop ("No output source for IPC") "\0"
  gettext_noop ("IPC parameter error") "\0"
  gettext_noop ("Unknown IPC inquire") "\0"
  gettext_noop ("General LDAP error") "\0"
  gettext_noop ("General LDAP attribute error") "\0"
  gettext_noop ("General LDAP name error") "\0"
  gettext_noop ("General LDAP security error") "\0"
  gettext_noop ("General LDAP service error") "\0"
  gettext_noop ("General LDAP update error") "\0"
  gettext_noop ("Experimental LDAP error code") "\0"
  gettext_noop ("Private LDAP error code") "\0"
  gettext_noop ("Other general LDAP error") "\0"
  gettext_noop ("LDAP connecting failed (X)") "\0"
  gettext_noop ("LDAP referral limit exceeded") "\0"
  gettext_noop ("LDAP client loop") "\0"
  gettext_noop ("No LDAP results returned") "\0"
  gettext_noop ("LDAP control not found") "\0"
  gettext_noop ("Not supported by LDAP") "\0"
  gettext_noop ("LDAP connect error") "\0"
  gettext_noop ("Out of memory in LDAP") "\0"
  gettext_noop ("Bad parameter to an LDAP routine") "\0"
  gettext_noop ("User cancelled LDAP operation") "\0"
  gettext_noop ("Bad LDAP search filter") "\0"
  gettext_noop ("Unknown LDAP authentication method") "\0"
  gettext_noop ("Timeout in LDAP") "\0"
  gettext_noop ("LDAP decoding error") "\0"
  gettext_noop ("LDAP encoding error") "\0"
  gettext_noop ("LDAP local error") "\0"
  gettext_noop ("Cannot contact LDAP server") "\0"
  gettext_noop ("LDAP success") "\0"
  gettext_noop ("LDAP operations error") "\0"
  gettext_noop ("LDAP protocol error") "\0"
  gettext_noop ("Time limit exceeded in LDAP") "\0"
  gettext_noop ("Size limit exceeded in LDAP") "\0"
  gettext_noop ("LDAP compare false") "\0"
  gettext_noop ("LDAP compare true") "\0"
  gettext_noop ("LDAP authentication method not supported") "\0"
  gettext_noop ("Strong(er) LDAP authentication required") "\0"
  gettext_noop ("Partial LDAP results+referral received") "\0"
  gettext_noop ("LDAP referral") "\0"
  gettext_noop ("Administrative LDAP limit exceeded") "\0"
  gettext_noop ("Critical LDAP extension is unavailable") "\0"
  gettext_noop ("Confidentiality required by LDAP") "\0"
  gettext_noop ("LDAP SASL bind in progress") "\0"
  gettext_noop ("No such LDAP attribute") "\0"
  gettext_noop ("Undefined LDAP attribute type") "\0"
  gettext_noop ("Inappropriate matching in LDAP") "\0"
  gettext_noop ("Constraint violation in LDAP") "\0"
  gettext_noop ("LDAP type or value exists") "\0"
  gettext_noop ("Invalid syntax in LDAP") "\0"
  gettext_noop ("No such LDAP object") "\0"
  gettext_noop ("LDAP alias problem") "\0"
  gettext_noop ("Invalid DN syntax in LDAP") "\0"
  gettext_noop ("LDAP entry is a leaf") "\0"
  gettext_noop ("LDAP alias dereferencing problem") "\0"
  gettext_noop ("LDAP proxy authorization failure (X)") "\0"
  gettext_noop ("Inappropriate LDAP authentication") "\0"
  gettext_noop ("Invalid LDAP credentials") "\0"
  gettext_noop ("Insufficient access for LDAP") "\0"
  gettext_noop ("LDAP server is busy") "\0"
  gettext_noop ("LDAP server is unavailable") "\0"
  gettext_noop ("LDAP server is unwilling to perform") "\0"
  gettext_noop ("Loop detected by LDAP") "\0"
  gettext_noop ("LDAP naming violation") "\0"
  gettext_noop ("LDAP object class violation") "\0"
  gettext_noop ("LDAP operation not allowed on non-leaf") "\0"
  gettext_noop ("LDAP operation not allowed on RDN") "\0"
  gettext_noop ("Already exists (LDAP)") "\0"
  gettext_noop ("Cannot modify LDAP object class") "\0"
  gettext_noop ("LDAP results too large") "\0"
  gettext_noop ("LDAP operation affects multiple DSAs") "\0"
  gettext_noop ("Virtual LDAP list view error") "\0"
  gettext_noop ("Other LDAP error") "\0"
  gettext_noop ("Resources exhausted in LCUP") "\0"
  gettext_noop ("Security violation in LCUP") "\0"
  gettext_noop ("Invalid data in LCUP") "\0"
  gettext_noop ("Unsupported scheme in LCUP") "\0"
  gettext_noop ("Reload required in LCUP") "\0"
  gettext_noop ("LDAP cancelled") "\0"
  gettext_noop ("No LDAP operation to cancel") "\0"
  gettext_noop ("Too late to cancel LDAP") "\0"
  gettext_noop ("Cannot cancel LDAP") "\0"
  gettext_noop ("LDAP assertion failed") "\0"
  gettext_noop ("Proxied authorization denied by LDAP") "\0"
  gettext_noop ("User defined error code 1") "\0"
  gettext_noop ("User defined error code 2") "\0"
  gettext_noop ("User defined error code 3") "\0"
  gettext_noop ("User defined error code 4") "\0"
  gettext_noop ("User defined error code 5") "\0"
  gettext_noop ("User defined error code 6") "\0"
  gettext_noop ("User defined error code 7") "\0"
  gettext_noop ("User defined error code 8") "\0"
  gettext_noop ("User defined error code 9") "\0"
  gettext_noop ("User defined error code 10") "\0"
  gettext_noop ("User defined error code 11") "\0"
  gettext_noop ("User defined error code 12") "\0"
  gettext_noop ("User defined error code 13") "\0"
  gettext_noop ("User defined error code 14") "\0"
  gettext_noop ("User defined error code 15") "\0"
  gettext_noop ("User defined error code 16") "\0"
  gettext_noop ("System error w/o errno") "\0"
  gettext_noop ("Unknown system error") "\0"
  gettext_noop ("End of file") "\0"
  gettext_noop ("Unknown error code");

static const int msgidx[] =
  {
    0,
    8,
    22,
    37,
    63,
    92,
    117,
    132,
    147,
    161,
    175,
    190,
    205,
    230,
    243,
    258,
    272,
    283,
    297,
    319,
    335,
    365,
    385,
    409,
    435,
    460,
    478,
    487,
    497,
    513,
    526,
    540,
    559,
    583,
    603,
    619,
    634,
    650,
    666,
    683,
    697,
    713,
    740,
    757,
    777,
    796,
    813,
    833,
    845,
    859,
    872,
    888,
    907,
    926,
    946,
    966,
    980,
    1002,
    1022,
    1030,
    1034,
    1048,
    1071,
    1079,
    1094,
    1107,
    1122,
    1151,
    1180,
    1203,
    1219,
    1235,
    1255,
    1268,
    1283,
    1300,
    1316,
    1333,
    1350,
    1362,
    1375,
    1406,
    1427,
    1447,
    1468,
    1490,
    1502,
    1517,
    1525,
    1538,
    1547,
    1565,
    1576,
    1587,
    1601,
    1621,
    1634,
    1646,
    1660,
    1672,
    1692,
    1711,
    1731,
    1753,
    1777,
    1798,
    1821,
    1842,
    1857,
    1868,
    1888,
    1901,
    1914,
    1931,
    1953,
    1967,
    1987,
    2003,
    2017,
    2028,
    2048,
    2071,
    2092,
    2107,
    2128,
    2150,
    2166,
    2180,
    2196,
    2210,
    2227,
    2239,
    2271,
    2291,
    2303,
    2313,
    2325,
    2343,
    2364,
    2376,
    2391,
    2408,
    2423,
    2439,
    2453,
    2472,
    2491,
    2514,
    2535,
    2559,
    2577,
    2599,
    2622,
    2640,
    2652,
    2670,
    2687,
    2701,
    2718,
    2733,
    2756,
    2775,
    2788,
    2807,
    2831,
    2858,
    2871,
    2896,
    2907,
    2926,
    2938,
    2954,
    2972,
    2999,
    3006,
    3021,
    3037,
    3053,
    3073,
    3086,
    3098,
    3115,
    3127,
    3144,
    3158,
    3174,
    3201,
    3224,
    3247,
    3270,
    3285,
    3302,
    3320,
    3341,
    3360,
    3391,
    3409,
    3427,
    3449,
    3475,
    3502,
    3519,
    3560,
    3592,
    3630,
    3657,
    3687,
    3717,
    3745,
    3782,
    3806,
    3854,
    3896,
    3936,
    3972,
    3994,
    4021,
    4029,
    4036,
    4047,
    4065,
    4082,
    4113,
    4134,
    4159,
    4174,
    4198,
    4217,
    4253,
    4274,
    4303,
    4323,
    4350,
    4378,
    4393,
    4404,
    4421,
    4458,
    4503,
    4547,
    4587,
    4625,
    4663,
    4705,
    4739,
    4784,
    4829,
    4842,
    4852,
    4865,
    4900,
    4920,
    4925,
    4931,
    4949,
    4972,
    4996,
    5017,
    5045,
    5075,
    5103,
    5123,
    5147,
    5174,
    5192,
    5210,
    5238,
    5253,
    5269,
    5297,
    5320,
    5340,
    5357,
    5385,
    5409,
    5434,
    5454,
    5474,
    5493,
    5522,
    5546,
    5574,
    5601,
    5627,
    5656,
    5680,
    5705,
    5732,
    5761,
    5778,
    5803,
    5826,
    5848,
    5867,
    5889,
    5922,
    5952,
    5975,
    6010,
    6026,
    6046,
    6066,
    6083,
    6110,
    6123,
    6145,
    6165,
    6193,
    6221,
    6240,
    6258,
    6299,
    6339,
    6378,
    6392,
    6427,
    6466,
    6499,
    6526,
    6549,
    6579,
    6610,
    6639,
    6665,
    6688,
    6708,
    6727,
    6753,
    6774,
    6807,
    6844,
    6878,
    6903,
    6932,
    6952,
    6979,
    7015,
    7037,
    7059,
    7087,
    7126,
    7160,
    7182,
    7214,
    7237,
    7274,
    7303,
    7320,
    7348,
    7375,
    7396,
    7423,
    7447,
    7462,
    7490,
    7514,
    7533,
    7555,
    7592,
    7618,
    7644,
    7670,
    7696,
    7722,
    7748,
    7774,
    7800,
    7826,
    7853,
    7880,
    7907,
    7934,
    7961,
    7988,
    8015,
    8038,
    8059,
    8071
  };

static GPG_ERR_INLINE int
msgidxof (int code)
{
  return (0 ? 0
  : ((code >= 0) && (code <= 213)) ? (code - 0)
  : ((code >= 218) && (code <= 271)) ? (code - 4)
  : ((code >= 273) && (code <= 281)) ? (code - 5)
  : ((code >= 721) && (code <= 729)) ? (code - 444)
  : ((code >= 750) && (code <= 752)) ? (code - 464)
  : ((code >= 754) && (code <= 782)) ? (code - 465)
  : ((code >= 784) && (code <= 789)) ? (code - 466)
  : ((code >= 800) && (code <= 804)) ? (code - 476)
  : ((code >= 815) && (code <= 822)) ? (code - 486)
  : ((code >= 832) && (code <= 839)) ? (code - 495)
  : ((code >= 844) && (code <= 844)) ? (code - 499)
  : ((code >= 848) && (code <= 848)) ? (code - 502)
  : ((code >= 881) && (code <= 891)) ? (code - 534)
  : ((code >= 1024) && (code <= 1039)) ? (code - 666)
  : ((code >= 16381) && (code <= 16383)) ? (code - 16007)
  : 16384 - 16007);
}
