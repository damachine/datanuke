# DataNuke

**Makes data powerless**

BSI-compliant secure data deletion through encryption and key destruction.

**POSIX-compliant** tool for Unix-like systems (Linux, macOS, BSD).

## What It Does

**Implements the BSI method: "Daten verschlüsseln und Schlüssel wegwerfen"** (Encrypt data and throw away the key)

Encrypts files or entire devices in-place with AES-256-CBC, displays the encryption key once, then permanently wipes the key from RAM. Without the key, the encrypted data is permanently irrecoverable.

Based on [BSI (Bundesamt für Sicherheit in der Informationstechnik)](https://www.bsi.bund.de/DE/Themen/Verbraucherinnen-und-Verbraucher/Informationen-und-Empfehlungen/Cyber-Sicherheitsempfehlungen/Daten-sichern-verschluesseln-und-loeschen/Daten-endgueltig-loeschen/daten-endgueltig-loeschen_node.html) recommendations: **"Encrypt-then-Delete-Key"** (BSI CON.6).

## Why This Method?

- **Cryptographically secure**: AES-256, not just pattern overwriting
- **Fast**: Single pass vs. multi-pass wiping (10-20x faster)
- **SSD-safe**: No wear leveling issues
- **Universal**: Works on all storage types

## Installation

```bash
# Dependencies
sudo apt-get install build-essential libssl-dev cmake  # Debian/Ubuntu
# or: brew install openssl cmake  # macOS

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

## Usage

```bash
# Encrypt a file
sudo datanuke <file>

# Encrypt a block device (entire drive/partition)
sudo datanuke <device>
```

**You can safely format, delete, reuse, or physically destroy the file/device.** 
**After encryption, the file/device is gibberish - worthless without the key.**

To complete secure deletion:
 1. Remove the encrypted file with normal methods (rm).
 2. Forget the key if you don't need the data.

### Example: File Encryption

```bash
echo "Sensitive data" > secret.txt
sudo datanuke secret.txt
```

**Output:**
```
DataNuke v1.0.0 - Secure Data Deletion (BSI-compliant)

Target: secret.txt
Type:   Regular File
Method: Encrypt-then-Delete-Key
---
ENCRYPTION KEY - SAVE NOW OR LOSE FOREVER

Key: 7ee6c8b5eb89d025e79fb6990d1ea0f78cbe9dd7070159023e94a39a68c399e6
IV:  0486802bd91a4596272e8051ceb42bd5

Key is stored in RAM only and will be wiped immediately.
Write it down now if you need to decrypt later. (both hex values below)
---
OPERATION SUCCESSFUL

Target:         secret.txt
Status:         ENCRYPTED (AES-256-CBC)
Encryption key: SECURELY WIPED FROM MEMORY

The file/device is now encrypted and permanently unrecoverable - worthless without the key.

To complete secure deletion process:
 1) You can safely remove the encrypted file with normal methods.
 2) Forget the key if you do not need to recover the data.
```

**What happened:**
- `secret.txt` was encrypted in-place (same filename, different content)
- Algorithm: AES-256-CBC with random 256-bit key and 128-bit IV
- Key generation: OpenSSL RAND_bytes() (cryptographically secure)
- Encryption key was displayed on screen (one-time only)
- Key was wiped from RAM with 7-pass Gutmann method (0x00, 0xFF, random, 0x00, volatile)
- Memory protection: POSIX mlock() prevented key from swapping to disk
- File is now encrypted gibberish without the key

### Example: Block Device Encryption

```bash
# Create a test partition or use existing device
sudo datanuke /dev/sdb1
```

**Output:**
```
DataNuke v1.0.0 - Secure Data Deletion (BSI-compliant)

Target: /dev/sdb1
Type:   Block Device
Method: Encrypt-then-Delete-Key

Device size: 8.00 GB (8589934592 bytes)

WARNING: This will DESTROY all data on /dev/sdb1!
Type YES to confirm: YES

Encrypting device...

Progress: 8.00 GB / 8.00 GB (100.0%)  

---
ENCRYPTION KEY - SAVE NOW OR LOSE FOREVER

Key: 9f2c1d8e7b6a5f4e3d2c1b0a9f8e7d6c5b4a3f2e1d0c9b8a7f6e5d4c3b2a1f0e
IV:  1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d

Key is stored in RAM only and will be wiped immediately.
Write it down now if you need to decrypt later. (both hex values below)
---
OPERATION SUCCESSFUL

Target:         /dev/sdb1
Status:         ENCRYPTED (AES-256-CBC)
Encryption key: SECURELY WIPED FROM MEMORY

The file/device is now encrypted and permanently unrecoverable - worthless without the key.

To complete secure deletion process:
 1) You can safely remove the encrypted file with normal methods.
 2) Forget the key if you do not need to recover the data.
```

**What happened:**
- `/dev/sdb1` was encrypted in-place (raw sectors overwritten with encrypted data)
- Device is now gibberish - can be formatted, reused, or physically destroyed
- Without the key, data recovery is computationally infeasible

## Data Recovery

If you saved the key while it was displayed, decrypt with OpenSSL:

```bash
openssl enc -d -aes-256-cbc \
  -K <your_saved_key_hex> \
  -iv <your_saved_iv_hex> \
  -in secret.txt \
  -out secret_recovered.txt
```

**For permanent deletion:** Don't save the key.

## Security Notes

- **File is encrypted in-place** - same filename, encrypted content
- **Works with ANY file type** - text, images, videos, databases, archives, etc.
- **Key stored in RAM only** - never touches disk
- **Key displayed once** - write it down or lose it forever
- **7-pass Gutmann wipe** - key is destroyed from memory
- **AES-256-CBC** - computationally infeasible to break
- **Encrypted file is unreadable** without the key - permanently destroyed data

## Use Cases

**File Encryption - Selling laptop/PC:**
```bash
# Encrypt all files in Documents folder
find ~/Documents -type f -exec sudo datanuke {} \;

# Encrypt specific file types
find ~/Pictures -type f \( -name "*.jpg" -o -name "*.png" \) -exec sudo datanuke {} \;
```

**File Encryption - GDPR compliance (right to erasure):**
```bash
# Works with any file type: CSV, PDF, databases, etc.
sudo datanuke customer_data.csv
sudo datanuke invoices.pdf
sudo datanuke database.sqlite
rm customer_data.csv invoices.pdf database.sqlite
```

**Device Encryption - Wiping entire drive before sale/disposal:**
```bash
# ⚠️  USE LIVE SYSTEM (boot from USB) when wiping OS drive!
# ⚠️  Cannot wipe drive with running OS!

# Examples (requires root):
sudo datanuke /dev/sdb        # Entire drive
sudo datanuke /dev/sdb1       # Single partition
sudo datanuke /dev/nvme0n1    # NVMe drive
```

**Use cases for device encryption:**
- 📱 Selling, gifting, or trading in devices
- 🗑️ Disposing of old hard drives and SSDs
- 🔒 Irrevocable deletion of sensitive information
- 💼 GDPR compliance (Art. 17 - Right to erasure)

**⚠️  Device Encryption Warnings:**
- **Unmount before encrypting:** `sudo umount /dev/sdb1`
- **Use live system** if target contains running OS (boot from Ubuntu Live USB)
- **All data destroyed permanently** - no recovery possible
- **"YES" confirmation required** - tool prevents accidental operations
- **After encryption:** Device becomes unreadable gibberish - can be safely formatted, reused, or physically destroyed

## How It Works

```
File: "Sensitive Data..."
  ↓ AES-256-CBC encryption with random key
File: 0x7a3f89c2... (encrypted)
  ↓ Display key on screen
Key: [user can save it]
  ↓ 7-pass Gutmann wipe
Key: [destroyed from RAM]
  ↓ Normal file deletion
Result: Encrypted file, no key = data is powerless
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.
BSI Method: "Daten verschlüsseln und Schlüssel wegwerfen"

DataNuke implements the official BSI (Bundesamt für Sicherheit in der Informationstechnik) recommendation for secure data deletion:

> **"Wenn Sie die Daten auf dem Datenträger oder Gerät verschlüsselt haben, reicht es aus, alle Schlüssel sicher zu löschen. Diese Methode bietet – sofern der Schlüssel tatsächlich gelöscht und nicht nur als gelöscht markiert wurde – einen zuverlässigen Schutz gegen eine unbefugte Wiederherstellung."**

Translation: *"If you have encrypted the data on the storage medium or device, it is sufficient to securely delete all keys. This method provides – provided the key is actually deleted and not just marked as deleted – reliable protection against unauthorized recovery."*

This is exactly what DataNuke does:
1. ✅ Encrypts data with AES-256-CBC
2. ✅ Displays key once (optional save for recovery)
3. ✅ **Securely deletes key** (7-pass Gutmann wipe, volatile memory)
4. ✅ Key is never written to disk (POSIX mlock())

## References

- [BSI: Daten endgültig löschen](https://www.bsi.bund.de/DE/Themen/Verbraucherinnen-und-Verbraucher/Informationen-und-Empfehlungen/Cyber-Sicherheitsempfehlungen/Daten-sichern-verschluesseln-und-loeschen/Daten-endgueltig-loeschen/daten-endgueltig-loeschen_node.html)## License

MIT License - see [LICENSE](LICENSE) file.

## References

- [BSI CON.6: Löschen und Vernichten](https://www.bsi.bund.de/)
- [NIST FIPS 197: AES Standard](https://csrc.nist.gov/publications/detail/fips/197/final)
- [OpenSSL Documentation](https://www.openssl.org/docs/)

---

**"Makes data powerless"** – Encryption + Key Destruction = Permanent Data Loss
