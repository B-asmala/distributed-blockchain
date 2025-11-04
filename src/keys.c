#include "keys.h"



// Generate RSA keypair and save to PEM files
void generate_keys(const char * public_key_path, const char * private_key_path) {
    RSA *rsa = RSA_new();
    BIGNUM *bn = BN_new();
    BN_set_word(bn, RSA_F4);

    if (RSA_generate_key_ex(rsa, 2048, bn, NULL) != 1) {
        fprintf(stderr, "Key generation failed\n");
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    // Write private key
    FILE *fp = fopen(private_key_path, "w");
    if (!fp) { perror("private.pem"); exit(1); }
    PEM_write_RSAPrivateKey(fp, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(fp);

    // Write public key
    fp = fopen(public_key_path, "w");
    if (!fp) { perror("public.pem"); exit(1); }
    PEM_write_RSA_PUBKEY(fp, rsa);
    fclose(fp);

    //printf("Generated RSA keypair and saved to %s and %s\n", private_key_path, public_key_path);

    BN_free(bn);
    RSA_free(rsa);
}

// Load private key
RSA *load_private_key(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) { perror("open private key"); return NULL; }
    RSA *rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);
    return rsa;
}

// Load public key
RSA *load_public_key(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) { perror("open public key"); return NULL; }
    RSA *rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);
    return rsa;
}
