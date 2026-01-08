/**
 * @file datanuke.h
 * @brief DataNuke - Secure Data Deletion Tool
 * @details Makes data powerless
 *
 * This header defines the public API for DataNuke, a BSI-compliant
 * secure data deletion tool that uses the "Encrypt-then-Delete-Key"
 * method to make data permanently irrecoverable.
 *
 * @version 1.0.0
 * @author DataNuke Contributors
 */

#ifndef DATANUKE_H
#define DATANUKE_H

#include <stddef.h>
#include <stdint.h>

/** @brief Version string for DataNuke */
#define DATANUKE_VERSION "1.0.0"

/** @brief AES-256 key size in bytes (256 bits) */
#define AES_KEY_SIZE 32

/** @brief AES block size in bytes (128 bits) */
#define AES_BLOCK_SIZE 16

/**
 * @defgroup ReturnCodes Return Codes
 * @brief Status codes returned by DataNuke functions
 * @{
 */

/** @brief Operation completed successfully */
#define DATANUKE_SUCCESS 0

/** @brief I/O error (file/device access failed) */
#define DATANUKE_ERROR_IO -1

/** @brief Cryptographic operation failed */
#define DATANUKE_ERROR_CRYPTO -2

/** @brief Memory allocation or locking failed */
#define DATANUKE_ERROR_MEMORY -3

/** @brief Platform-specific operation failed */
#define DATANUKE_ERROR_PLATFORM -4

/** @} */ // end of ReturnCodes

/**
 * @struct crypto_context_t
 * @brief Encryption context containing key, IV, and cipher state
 *
 * This structure holds all cryptographic material needed for
 * AES-256-CBC encryption. It MUST be securely wiped after use
 * using crypto_secure_wipe_key() to prevent key recovery.
 */
typedef struct {
    uint8_t key[AES_KEY_SIZE];  /**< 256-bit AES encryption key */
    uint8_t iv[AES_BLOCK_SIZE]; /**< 128-bit initialization vector */
    void *cipher_ctx;           /**< OpenSSL cipher context (internal) */
} crypto_context_t;

/**
 * @defgroup Crypto Cryptographic Functions
 * @brief AES-256 encryption and secure key management
 * @{
 */

/**
 * @brief Initialize crypto context with random key and IV
 * @param ctx Pointer to crypto_context_t to initialize
 * @return DATANUKE_SUCCESS or DATANUKE_ERROR_CRYPTO
 */
int crypto_init(crypto_context_t *ctx);

/**
 * @brief Generate cryptographically secure random key
 * @param key Buffer to store generated key
 * @param key_size Size of key in bytes (must be AES_KEY_SIZE)
 * @return DATANUKE_SUCCESS or DATANUKE_ERROR_CRYPTO
 */
int crypto_generate_key(uint8_t *key, size_t key_size);

/**
 * @brief Encrypt file using AES-256-CBC
 * @param input_path Path to input file
 * @param output_path Path to output encrypted file
 * @param ctx Initialized crypto context
 * @return DATANUKE_SUCCESS, DATANUKE_ERROR_IO, or DATANUKE_ERROR_CRYPTO
 */
int crypto_encrypt_file(const char *input_path, const char *output_path, crypto_context_t *ctx);

/**
 * @brief Display encryption key in hexadecimal (ONE TIME ONLY)
 * @param ctx Crypto context containing key to display
 */
void crypto_display_key(const crypto_context_t *ctx);

/**
 * @brief Securely wipe encryption key using 7-pass Gutmann method
 * @param ctx Crypto context containing key to wipe
 * @return DATANUKE_SUCCESS or DATANUKE_ERROR_CRYPTO
 */
int crypto_secure_wipe_key(crypto_context_t *ctx);

/**
 * @brief Cleanup and securely wipe entire crypto context
 * @param ctx Crypto context to cleanup
 */
void crypto_cleanup(crypto_context_t *ctx);

/** @} */ // end of Crypto

/**
 * @defgroup SecureDelete Secure Deletion Functions
 * @brief Multi-pass overwrite and secure file/device deletion
 * @{
 */

/**
 * @brief Securely delete a file using multi-pass overwrite
 * @param path Path to file to delete
 * @return DATANUKE_SUCCESS, DATANUKE_ERROR_IO, or DATANUKE_ERROR_CRYPTO
 */
int secure_delete_file(const char *path);

/**
 * @brief Securely overwrite and delete entire device/partition
 * @param device_path Path to device (e.g., /dev/sdb, \\\\.\\PhysicalDrive1)
 * @return DATANUKE_SUCCESS, DATANUKE_ERROR_IO, or DATANUKE_ERROR_CRYPTO
 */
int secure_delete_device(const char *device_path);

/**
 * @brief Perform multi-pass overwrite on file or device
 * @param path Path to file or device
 * @param passes Number of overwrite passes
 * @return DATANUKE_SUCCESS, DATANUKE_ERROR_IO, or DATANUKE_ERROR_CRYPTO
 */
int secure_overwrite(const char *path, size_t passes);

/** @} */ // end of SecureDelete

/**
 * @defgroup Platform Platform-Specific Functions
 * @brief Cross-platform abstractions for device access and memory locking
 * @{
 */

/**
 * @brief Get size of block device in bytes
 * @param device_path Path to device
 * @param size Pointer to store device size
 * @return DATANUKE_SUCCESS or DATANUKE_ERROR_PLATFORM
 */
int platform_get_device_size(const char *device_path, uint64_t *size);

/**
 * @brief Check if path points to a block device
 * @param path Path to check
 * @return 1 if device, 0 if regular file, -1 on error
 */
int platform_is_device(const char *path);

/**
 * @brief Lock memory pages to prevent swapping to disk
 * @param addr Starting address of memory region
 * @param len Length of memory region in bytes
 * @return DATANUKE_SUCCESS or DATANUKE_ERROR_MEMORY
 */
int platform_lock_memory(void *addr, size_t len);

/**
 * @brief Unlock previously locked memory pages
 * @param addr Starting address of memory region
 * @param len Length of memory region in bytes
 * @return DATANUKE_SUCCESS or DATANUKE_ERROR_MEMORY
 */
int platform_unlock_memory(void *addr, size_t len);

/** @} */ // end of Platform

#endif // DATANUKE_H
