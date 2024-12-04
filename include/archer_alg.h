#ifndef _ARCHER_ALG_H_
#define _ARCHER_ALG_H_

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SM2_C1C2C3 16
#define SM2_C1C3C2 64

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EcPrivateKey {
    uint8_t d[32];
} EcPrivateKey;

typedef struct EcPublicKey {
    uint8_t x[32];
    uint8_t y[32];
} EcPublicKey;

typedef struct EcSignature {
    uint8_t r[32];
    uint8_t s[32];
} EcSignature;

typedef struct Hash32 {
    uint8_t h[32];
} Hash32;


// secp256k1 sign algorithm
void secp256k1_key_gen(EcPrivateKey *sk, EcPublicKey *pk);
void secp256k1_privateKey_to_publicKey(const EcPrivateKey *sk, EcPublicKey *pk);
void secp256k1_sign(const EcPrivateKey *sk, const uint8_t *msg, const size_t msg_len, EcSignature *sig);
/**
 * @return 1=success, 0=verify failed 
*/
int secp256k1_verify(const EcPublicKey *pk, const uint8_t *msg, const size_t msg_len, const EcSignature *sig);
int secp256k1_get_v(const EcSignature *sig);
void secp256k1_recover_publicKey(const EcSignature *sig, const uint8_t *msg, const size_t msg_len, EcPublicKey *pk);




// sm2 crypto
void sm2p256v1_key_gen(EcPrivateKey *sk, EcPublicKey *pk);
void sm2p256v1_encrypt(const EcPublicKey *pk, const uint8_t *msg, const size_t msg_len, const int mode, uint8_t **out, size_t *out_len);
/**
 * @return 1=success, 0=decrypt failed 
*/
int sm2p256v1_decrypt(const EcPrivateKey *sk, const uint8_t *cipher, const size_t cipher_len, const int mode, uint8_t **out, size_t *out_len);



// sm2 sign algorithm
void sm2p256v1_privateKey_to_publicKey(const EcPrivateKey *sk, EcPublicKey *pk);
void sm2p256v1_sign(const EcPrivateKey *sk, const uint8_t *msg, const size_t msg_len, EcSignature *sig);
/**
 * @return 1=success, 0=verify failed 
*/
int sm2p256v1_verify(const EcPublicKey *pk, const uint8_t *msg, const size_t msg_len, const EcSignature *sig);



// hash
void keccak256(const uint8_t *content, const size_t content_len, Hash32 *hash);
void sha256(const uint8_t *content, const size_t content_len, Hash32 *hash);
void sm3(const uint8_t *content, const size_t content_len, Hash32 *hash);



// sm4
void sm4_encrypt(const uint8_t user_key[16], const uint8_t *in, const size_t in_size, uint8_t **out, size_t *out_size);
/**
 * @return 1=success, 0=decrypt failed 
*/
int sm4_decrypt(const uint8_t user_key[16], const uint8_t *in, const size_t in_size, uint8_t **out, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif