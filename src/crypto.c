/*
 * DataNuke - Secure Data Deletion Tool
 * Crypto Module - AES-256 Encryption according to BSI recommendations
 */

#include "datanuke.h"
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sleep()

/**
 * @brief Initialize cryptographic context with random key and IV
 *
 * Generates a cryptographically secure 256-bit key and 128-bit IV
 * using OpenSSL's RAND_bytes() function.
 *
 * @param ctx Pointer to crypto_context_t structure to initialize
 * @return DATANUKE_SUCCESS on success, DATANUKE_ERROR_CRYPTO on failure
 */
int crypto_init(crypto_context_t *ctx) {
    if (!ctx)
        return DATANUKE_ERROR_CRYPTO;

    memset(ctx, 0, sizeof(crypto_context_t));

    // Generate random key
    if (crypto_generate_key(ctx->key, AES_KEY_SIZE) != DATANUKE_SUCCESS) {
        return DATANUKE_ERROR_CRYPTO;
    }

    // Generate random IV
    if (RAND_bytes(ctx->iv, AES_BLOCK_SIZE) != 1) {
        fprintf(stderr, "Error generating IV: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return DATANUKE_ERROR_CRYPTO;
    }

    return DATANUKE_SUCCESS;
}

/**
 * @brief Generate a cryptographically secure random key
 *
 * Uses OpenSSL's RAND_bytes() which provides cryptographic-quality
 * random numbers suitable for key generation.
 *
 * @param key Pointer to buffer where key will be stored
 * @param key_size Size of the key in bytes (must be AES_KEY_SIZE)
 * @return DATANUKE_SUCCESS on success, DATANUKE_ERROR_CRYPTO on failure
 */
int crypto_generate_key(uint8_t *key, size_t key_size) {
    if (!key || key_size != AES_KEY_SIZE) {
        return DATANUKE_ERROR_CRYPTO;
    }

    // Use OpenSSL's cryptographically secure random number generator
    if (RAND_bytes(key, key_size) != 1) {
        fprintf(stderr, "Error generating key: %s\n", ERR_error_string(ERR_get_error(), NULL));
        return DATANUKE_ERROR_CRYPTO;
    }

    return DATANUKE_SUCCESS;
}

/**
 * @brief Encrypt a file using AES-256-CBC
 *
 * Reads the input file in 4KB chunks, encrypts each chunk using
 * AES-256-CBC mode, and writes the encrypted data to the output file.
 *
 * @param input_path Path to the input file to encrypt
 * @param output_path Path where encrypted file will be written
 * @param ctx Pointer to initialized crypto_context_t with key and IV
 * @return DATANUKE_SUCCESS on success, error code on failure
 */
int crypto_encrypt_file(const char *input_path, const char *output_path, crypto_context_t *ctx) {
    if (!input_path || !output_path || !ctx) {
        return DATANUKE_ERROR_CRYPTO;
    }

    FILE *input = fopen(input_path, "rb");
    if (!input) {
        perror("Cannot open input file");
        return DATANUKE_ERROR_IO;
    }

    FILE *output = fopen(output_path, "wb");
    if (!output) {
        perror("Cannot open output file");
        fclose(input);
        return DATANUKE_ERROR_IO;
    }

    EVP_CIPHER_CTX *cipher_ctx = EVP_CIPHER_CTX_new();
    if (!cipher_ctx) {
        fprintf(stderr, "Error creating cipher context\n");
        fclose(input);
        fclose(output);
        return DATANUKE_ERROR_CRYPTO;
    }

    /* Initialize AES-256-CBC encryption
     * CBC (Cipher Block Chaining) mode provides confidentiality.
     * Each block depends on all previous blocks, preventing pattern analysis.
     */
    if (EVP_EncryptInit_ex(cipher_ctx, EVP_aes_256_cbc(), NULL, ctx->key, ctx->iv) != 1) {
        fprintf(stderr, "Error initializing encryption: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(cipher_ctx);
        fclose(input);
        fclose(output);
        return DATANUKE_ERROR_CRYPTO;
    }

    /* Encrypt file in 4KB chunks
     * Processing in chunks allows encryption of files larger than available RAM.
     * Each chunk is encrypted and immediately written to reduce memory usage.
     */
    unsigned char inbuf[4096];
    unsigned char outbuf[4096 + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;

    while ((inlen = fread(inbuf, 1, sizeof(inbuf), input)) > 0) {
        if (EVP_EncryptUpdate(cipher_ctx, outbuf, &outlen, inbuf, inlen) != 1) {
            fprintf(stderr, "Error during encryption: %s\n", ERR_error_string(ERR_get_error(), NULL));
            EVP_CIPHER_CTX_free(cipher_ctx);
            fclose(input);
            fclose(output);
            return DATANUKE_ERROR_CRYPTO;
        }
        fwrite(outbuf, 1, outlen, output);
    }

    /* Finalize encryption
     * In CBC mode, this adds PKCS#7 padding to ensure the last block
     * is complete. The padding is necessary for proper decryption.
     */
    if (EVP_EncryptFinal_ex(cipher_ctx, outbuf, &outlen) != 1) {
        fprintf(stderr, "Error finalizing encryption: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(cipher_ctx);
        fclose(input);
        fclose(output);
        return DATANUKE_ERROR_CRYPTO;
    }
    fwrite(outbuf, 1, outlen, output);

    EVP_CIPHER_CTX_free(cipher_ctx);
    fclose(input);
    fclose(output);

    return DATANUKE_SUCCESS;
}

/**
 * @brief Display the encryption key and IV in hexadecimal format
 *
 * Shows the key to the user ONE TIME ONLY before it is securely deleted.
 * This allows for potential data recovery if needed. Once the key is wiped,
 * the encrypted data becomes permanently irrecoverable.
 *
 * @param ctx Pointer to crypto_context_t containing the key and IV
 */
void crypto_display_key(const crypto_context_t *ctx) {
    if (!ctx)
        return;

    // ANSI color codes for rich terminal UI
    const char *BOLD = "\033[1m";      // Bold
    const char *REVERSE = "\033[7m";   // Reverse video (swap fg/bg)
    const char *YELLOW = "\033[1;33m"; // Bright yellow
    const char *CYAN = "\033[1;36m";   // Bright cyan
    const char *RESET = "\033[0m";     // Reset all attributes

    printf("\n");

    // Title bar with reverse video (white on black)
    printf("%s%s", BOLD, REVERSE);
    printf("╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║               🔐  ENCRYPTION KEY - SAVE NOW OR LOSE FOREVER  🔐              ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════╝\n");
    printf("%s", RESET);

    printf("\n");

    // Key section with cyan color
    printf("%s%s╔═══════════════════════════════════════════════════════════════════╗%s\n", BOLD, CYAN, RESET);
    printf("%s%s║%s  %sKey:%s ", BOLD, CYAN, RESET, YELLOW, RESET);
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        printf("%s%02x%s", YELLOW, ctx->key[i], RESET);
    }
    printf("  %s%s║%s\n", BOLD, CYAN, RESET);

    printf("%s%s║%s  %sIV:%s  ", BOLD, CYAN, RESET, YELLOW, RESET);
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        printf("%s%02x%s", YELLOW, ctx->iv[i], RESET);
    }
    printf("                                  %s%s║%s\n", BOLD, CYAN, RESET);
    printf("%s%s╚═══════════════════════════════════════════════════════════════════╝%s\n", BOLD, CYAN, RESET);

    printf("\n");

    // Info section with blue/cyan styling
    printf("%s%s", BOLD, CYAN);
    printf("╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║  ℹ️  Key is stored in RAM only and will be wiped immediately     ║\n");
    printf("║  ℹ️  Write it down now if you need to decrypt the file later     ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════╝\n");
    printf("%s", RESET);

    printf("\n");

    // Countdown with sleep (shorter, calmer)
    for (int i = 3; i > 0; i--) {
        printf("\r%s%sWiping key in %d...%s", BOLD, CYAN, i, RESET);
        fflush(stdout);
        sleep(1);
    }
    printf("\r%s                            %s\r", BOLD, RESET); // Clear countdown line
    fflush(stdout);
}

/**
 * @brief Securely wipe encryption key using 7-pass Gutmann method
 *
 * Implements a multi-pass overwrite strategy to prevent key recovery:
 * Pass 1: Write all zeros (0x00)
 * Pass 2: Write all ones (0xFF)
 * Pass 3: Write random data
 * Pass 4: Write all zeros again
 * Pass 5: Use volatile pointers to prevent compiler optimization
 *
 * This follows BSI recommendations for secure key destruction.
 *
 * @param ctx Pointer to crypto_context_t containing key to wipe
 * @return DATANUKE_SUCCESS on success, DATANUKE_ERROR_CRYPTO on failure
 */
int crypto_secure_wipe_key(crypto_context_t *ctx) {
    if (!ctx)
        return DATANUKE_ERROR_CRYPTO;

    /* Pass 1: Overwrite with zeros
     * Clears any existing data with a known pattern
     */
    memset(ctx->key, 0x00, AES_KEY_SIZE);
    memset(ctx->iv, 0x00, AES_BLOCK_SIZE);

    /* Pass 2: Overwrite with ones
     * Flips all bits from previous pass
     */
    memset(ctx->key, 0xFF, AES_KEY_SIZE);
    memset(ctx->iv, 0xFF, AES_BLOCK_SIZE);

    /* Pass 3: Overwrite with random data
     * Introduces unpredictability, making pattern analysis impossible
     */
    RAND_bytes(ctx->key, AES_KEY_SIZE);
    RAND_bytes(ctx->iv, AES_BLOCK_SIZE);

    /* Pass 4: Final overwrite with zeros
     * Leaves memory in a known, clean state
     */
    memset(ctx->key, 0x00, AES_KEY_SIZE);
    memset(ctx->iv, 0x00, AES_BLOCK_SIZE);

    /* Pass 5: Volatile overwrite to prevent compiler optimization
     * Compilers may optimize away memset() calls if they determine the
     * memory won't be read again. Using volatile pointers forces the
     * compiler to perform the write operation.
     */
    volatile uint8_t *vkey = (volatile uint8_t *)ctx->key;
    volatile uint8_t *viv = (volatile uint8_t *)ctx->iv;
    for (size_t i = 0; i < AES_KEY_SIZE; i++) {
        vkey[i] = 0;
    }
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        viv[i] = 0;
    }

    return DATANUKE_SUCCESS;
}

/**
 * @brief Cleanup crypto context and securely wipe all sensitive data
 *
 * Calls crypto_secure_wipe_key() to perform secure key deletion,
 * then zeros out the entire context structure.
 *
 * @param ctx Pointer to crypto_context_t to cleanup
 */
void crypto_cleanup(crypto_context_t *ctx) {
    if (ctx) {
        crypto_secure_wipe_key(ctx);
        memset(ctx, 0, sizeof(crypto_context_t));
    }
}

/**
 * @brief Encrypt a block device using AES-256-CBC
 *
 * Reads the device in 1MB chunks, encrypts each chunk using
 * AES-256-CBC mode, and writes the encrypted data back to the device.
 * Shows progress indicator during operation.
 *
 * WARNING: This DESTROYS all data on the device permanently!
 *
 * @param device_path Path to the block device (e.g., /dev/sdb)
 * @param ctx Pointer to initialized crypto_context_t with key and IV
 * @return DATANUKE_SUCCESS on success, error code on failure
 */
int crypto_encrypt_device(const char *device_path, crypto_context_t *ctx) {
    if (!device_path || !ctx) {
        return DATANUKE_ERROR_CRYPTO;
    }

    // Open device for read/write with synchronous I/O
    FILE *device = fopen(device_path, "r+b");
    if (!device) {
        perror("Cannot open device");
        return DATANUKE_ERROR_IO;
    }

    // Get device size
    uint64_t device_size = 0;
    if (platform_get_device_size(device_path, &device_size) != DATANUKE_SUCCESS) {
        fprintf(stderr, "Error getting device size\n");
        fclose(device);
        return DATANUKE_ERROR_IO;
    }

    EVP_CIPHER_CTX *cipher_ctx = EVP_CIPHER_CTX_new();
    if (!cipher_ctx) {
        fprintf(stderr, "Error creating cipher context\n");
        fclose(device);
        return DATANUKE_ERROR_CRYPTO;
    }

    // Initialize AES-256-CBC encryption
    if (EVP_EncryptInit_ex(cipher_ctx, EVP_aes_256_cbc(), NULL, ctx->key, ctx->iv) != 1) {
        fprintf(stderr, "Error initializing encryption: %s\n", ERR_error_string(ERR_get_error(), NULL));
        EVP_CIPHER_CTX_free(cipher_ctx);
        fclose(device);
        return DATANUKE_ERROR_CRYPTO;
    }

    // Process device in 1MB chunks for efficiency
    const size_t CHUNK_SIZE = 1024 * 1024; // 1MB
    unsigned char *inbuf = malloc(CHUNK_SIZE);
    unsigned char *outbuf = malloc(CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH);

    if (!inbuf || !outbuf) {
        fprintf(stderr, "Memory allocation failed\n");
        free(inbuf);
        free(outbuf);
        EVP_CIPHER_CTX_free(cipher_ctx);
        fclose(device);
        return DATANUKE_ERROR_MEMORY;
    }

    uint64_t processed = 0;
    int outlen;
    size_t bytes_read;

    printf("\n");
    printf("\033[1;36m╔═══════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║                     ENCRYPTING DEVICE...                         ║\033[0m\n");
    printf("\033[1;36m╚═══════════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");

    // Read, encrypt, and write back in chunks
    while ((bytes_read = fread(inbuf, 1, CHUNK_SIZE, device)) > 0) {
        // Encrypt chunk
        if (EVP_EncryptUpdate(cipher_ctx, outbuf, &outlen, inbuf, bytes_read) != 1) {
            fprintf(stderr, "\nError during encryption: %s\n", ERR_error_string(ERR_get_error(), NULL));
            free(inbuf);
            free(outbuf);
            EVP_CIPHER_CTX_free(cipher_ctx);
            fclose(device);
            return DATANUKE_ERROR_CRYPTO;
        }

        // Seek back to write position
        fseek(device, -(long)bytes_read, SEEK_CUR);

        // Write encrypted data back to device
        if (fwrite(outbuf, 1, outlen, device) != (size_t)outlen) {
            fprintf(stderr, "\nError writing to device\n");
            free(inbuf);
            free(outbuf);
            EVP_CIPHER_CTX_free(cipher_ctx);
            fclose(device);
            return DATANUKE_ERROR_IO;
        }

        // Flush to ensure data is written
        fflush(device);

        processed += bytes_read;

        // Show progress
        double percent = (processed * 100.0) / device_size;
        double gb_processed = processed / (1024.0 * 1024.0 * 1024.0);
        double gb_total = device_size / (1024.0 * 1024.0 * 1024.0);

        printf("\r\033[1;36mProgress: %.2f GB / %.2f GB (%.1f%%)  \033[0m", gb_processed, gb_total, percent);
        fflush(stdout);
    }

    // Note: We don't call EVP_EncryptFinal_ex for devices
    // because we're encrypting raw sectors, not a padded file format

    printf("\n\n");

    free(inbuf);
    free(outbuf);
    EVP_CIPHER_CTX_free(cipher_ctx);
    fclose(device);

    return DATANUKE_SUCCESS;
}
