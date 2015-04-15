
#ifndef __SPRITZ_H__
#define __SPRITZ_H__

#include <stdint.h>

void spritz_encrypt(unsigned char __xdata *msg, uint8_t msglen,
                   const unsigned char __xdata *nonce, uint8_t noncelen,
                   const unsigned char __xdata *key, uint8_t keylen);

void spritz_decrypt(unsigned char __xdata *c, uint8_t clen,
                   const unsigned char __xdata *nonce, uint8_t noncelen,
                   const unsigned char __xdata *key, uint8_t keylen);

void spritz_auth(unsigned char __xdata *out, uint8_t outlen,
                const unsigned char __xdata *msg, uint8_t msglen,
                const unsigned char __xdata *key, uint8_t keylen);
#endif
