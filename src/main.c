/*
 * DataNuke - Secure Data Deletion Tool
 * According to BSI recommendations (Bundesamt für Sicherheit in der Informationstechnik)
 *
 * Method: Encrypt data with AES-256, then securely delete encryption key
 * This implements the BSI-recommended "Encrypt and throw away key" method
 */

#include "datanuke.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Display program usage information and available command-line options
 * @param program_name The name of the program executable
 */
void print_usage(const char *program_name) {
    printf("DataNuke v%s - Secure Data Deletion Tool\n", DATANUKE_VERSION);
    printf("\"Makes data powerless\"\n");
    printf("Based on BSI recommendations (Germany)\n\n");
    printf("Usage: %s <file|device>\n\n", program_name);
    printf("Description:\n");
    printf("  Encrypts files or entire block devices with AES-256-CBC.\n");
    printf("  After encryption, the key is securely destroyed.\n\n");
    printf("Examples:\n");
    printf("  %s secret.txt              # Encrypt file\n", program_name);
    printf("  %s /dev/sdb                # Encrypt entire drive (requires root)\n", program_name);
    printf("  %s /dev/sdb1               # Encrypt partition\n\n", program_name);
    printf("WARNING FOR DEVICES:\n");
    printf("  - Cannot encrypt mounted devices (umount first)\n");
    printf("  - Cannot encrypt device with running OS (use live system)\n");
    printf("  - This DESTROYS all data on the device permanently!\n\n");
    printf("WARNING: This tool permanently destroys data!\n");
    printf("         Make backups before use!\n");
}

/**
 * @brief Main entry point for DataNuke application
 *
 * Implements the BSI-recommended "Encrypt-then-Delete-Key" method:
 * 1. Encrypt file with AES-256-CBC
 * 2. Display encryption key (for optional recovery)
 * 3. Securely wipe key from memory
 * 4. Overwrite and delete original file
 * 5. Prompt user to delete .encrypted file
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    char *target_file = argv[1];

    // Check if target is a block device
    int is_device = platform_is_device(target_file);

    if (is_device < 0) {
        fprintf(stderr, "Error: Cannot access %s\n", target_file);
        return 1;
    }

    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║         DataNuke v%s                 ║\n", DATANUKE_VERSION);
    printf("║      \"Makes data powerless\"             ║\n");
    printf("║  Secure Data Deletion (BSI-compliant)   ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf("\n");
    printf("Target: %s\n", target_file);
    printf("Type:   %s\n", is_device ? "Block Device" : "Regular File");
    printf("Method: Encrypt-then-Delete-Key (BSI)\n\n");

    if (is_device) {
        uint64_t size;
        if (platform_get_device_size(target_file, &size) == DATANUKE_SUCCESS) {
            printf("Device size: %.2f GB (%llu bytes)\n\n", size / (1024.0 * 1024.0 * 1024.0),
                   (unsigned long long)size);
        }
        printf("\033[1;31m⚠️  WARNING: This will DESTROY all data on %s!\033[0m\n", target_file);
        printf("Type 'YES' to confirm (uppercase): ");
        char confirm[10];
        if (fgets(confirm, sizeof(confirm), stdin) == NULL || strncmp(confirm, "YES\n", 4) != 0) {
            printf("Aborted.\n");
            return 1;
        }
        printf("\n");
    }

    crypto_context_t ctx;
    if (crypto_init(&ctx) != DATANUKE_SUCCESS) {
        fprintf(stderr, "Failed to initialize cryptography\n");
        return 1;
    }

    // Lock key in memory to prevent swapping
    platform_lock_memory(&ctx, sizeof(ctx));

    int result;

    if (is_device) {
        // Encrypt entire block device
        result = crypto_encrypt_device(target_file, &ctx);

        if (result != DATANUKE_SUCCESS) {
            fprintf(stderr, "Device encryption failed\n");
            platform_unlock_memory(&ctx, sizeof(ctx));
            crypto_cleanup(&ctx);
            return 1;
        }
    } else {
        // Encrypt regular file
        char temp_path[512];
        char encrypted_path[512];
        snprintf(temp_path, sizeof(temp_path), "%s.tmp_encrypted", target_file);
        snprintf(encrypted_path, sizeof(encrypted_path), "%s", target_file);

        result = crypto_encrypt_file(target_file, temp_path, &ctx);

        if (result != DATANUKE_SUCCESS) {
            fprintf(stderr, "Encryption failed\n");
            platform_unlock_memory(&ctx, sizeof(ctx));
            crypto_cleanup(&ctx);
            return 1;
        }

        // Rename temp file to original name (overwrites original)
        if (remove(target_file) != 0 || rename(temp_path, encrypted_path) != 0) {
            fprintf(stderr, "Failed to replace original file with encrypted version\n");
            remove(temp_path);
            platform_unlock_memory(&ctx, sizeof(ctx));
            crypto_cleanup(&ctx);
            return 1;
        }
    }

    // Display key with countdown
    crypto_display_key(&ctx);

    // Wipe key from memory
    result = crypto_secure_wipe_key(&ctx);

    if (result != DATANUKE_SUCCESS) {
        fprintf(stderr, "Key wiping failed\n");
        platform_unlock_memory(&ctx, sizeof(ctx));
        crypto_cleanup(&ctx);
        return 1;
    }

    // Success summary
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║                     ✓ OPERATION SUCCESSFUL                      ║\n");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");
    printf("║  Target:         %s%-40s%s  ║\n", "\033[1;36m", target_file, "\033[0m");
    printf("║  Status:         %sENCRYPTED (AES-256-CBC)%s                      ║\n", "\033[1;32m", "\033[0m");
    printf("║  Encryption key: %sSECURELY WIPED FROM MEMORY%s                 ║\n", "\033[1;31m", "\033[0m");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");
    if (is_device) {
        printf("║  The device is now encrypted and permanently unrecoverable.     ║\n");
        printf("║  You can safely format, reuse, or physically destroy it.       ║\n");
    } else {
        printf("║  The file content is now permanently unrecoverable.             ║\n");
        printf("║  You can safely delete the file with normal methods.            ║\n");
    }
    printf("╚══════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    platform_unlock_memory(&ctx, sizeof(ctx));
    crypto_cleanup(&ctx);

    return 0;
}
