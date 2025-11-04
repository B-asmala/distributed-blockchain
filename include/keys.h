#ifndef KEYS_H
#define KEYS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/bn.h>

#define PUBLIC_KEY_PATH "keys/node_%d/public.pem"
#define PRIVATE_KEY_PATH "keys/node_%d/private.pem"

void generate_keys(const char * public_key_path, const char * private_key_path); 
RSA *load_private_key(const char *path);
RSA *load_public_key(const char *path);

#endif
