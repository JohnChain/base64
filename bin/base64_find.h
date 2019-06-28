//
//  base64 encoding and decoding with C++.
//  Version: 1.01.00
//

#ifndef BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A
#define BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A

#include <string>

void base64_encode_find(const char*, size_t len, char* out, size_t* outLen, int flag);
int base64_decode_find(const char*, size_t in_len, char* out, size_t* outLen, int flag);

#endif /* BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A */
