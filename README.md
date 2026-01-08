# DataNuke

**Makes data powerless**

BSI-compliant secure data deletion through encryption and key destruction.

## What It Does

Encrypts files in-place with AES-256-CBC, displays the encryption key, then permanently wipes the key from RAM. Without the key, the encrypted data is irrecoverable.

Based on [BSI (German Federal Office for Information Security)](https://www.bsi.bund.de/) recommendations: **"Encrypt-then-Delete-Key"** (BSI CON.6).

## Why This Method?

- **Cryptographically secure**: AES-256, not just pattern overwriting
- **Fast**: Single pass vs. multi-pass wiping (10-20x faster)
- **SSD-safe**: No wear leveling issues
- **Universal**: Works on all storage types

## Installation

```bash
# Linux/macOS
sudo apt-get install build-essential libssl-dev cmake  # Debian/Ubuntu
# or: brew install openssl cmake  # macOS

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --install .
```

## Usage

```bash
sudo datanuke <file>
```

### Example

```bash
echo "Sensitive data" > secret.txt
sudo datanuke secret.txt
```

**Output:**
```
╔══════════════════════════════════════════╗
║         DataNuke v1.0.0                 ║
║      "Makes data powerless"             ║
║  Secure Data Deletion (BSI-compliant)   ║
╚══════════════════════════════════════════╝

Target: secret.txt
Method: Encrypt-then-Delete-Key (BSI)

╔═══════════════════════════════════════════════════════════════════╗
║               🔐  ENCRYPTION KEY - SAVE NOW OR LOSE FOREVER  🔐              ║
╚═══════════════════════════════════════════════════════════════════╝

╔═══════════════════════════════════════════════════════════════════╗
║  Key: a1b2c3d4...  (64 hex chars)                                  ║
║  IV:  1a2b3c4d...  (32 hex chars)                                  ║
╚═══════════════════════════════════════════════════════════════════╝

╔═══════════════════════════════════════════════════════════════════╗
║  ℹ️  Key is stored in RAM only and will be wiped immediately     ║
║  ℹ️  Write it down now if you need to decrypt the file later     ║
╚═══════════════════════════════════════════════════════════════════╝

Wiping key in 3... 2... 1...

╔══════════════════════════════════════════════════════════════════╗
║                     ✓ OPERATION SUCCESSFUL                      ║
╠══════════════════════════════════════════════════════════════════╣
║  File:           secret.txt                                     ║
║  Status:         ENCRYPTED (AES-256-CBC)                        ║
║  Encryption key: SECURELY WIPED FROM MEMORY                     ║
╠══════════════════════════════════════════════════════════════════╣
║  The file content is now permanently unrecoverable.             ║
║  You can safely delete the file with normal methods.            ║
╚══════════════════════════════════════════════════════════════════╝
```

**What happened:**
- `secret.txt` was encrypted in-place (same filename, different content)
- Encryption key was displayed on screen
- Key was wiped from RAM with 7-pass Gutmann method
- File is now encrypted gibberish without the key

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

- **File keeps original name** - only content is encrypted
- **Key stored in RAM only** - never touches disk
- **Key displayed once** - write it down or lose it forever
- **7-pass Gutmann wipe** - key is destroyed from memory
- **AES-256-CBC** - computationally infeasible to break
- **Delete file normally** when done (rm, Trash, format)

## Use Cases

**Selling laptop/PC:**
```bash
find ~/Documents -type f -exec sudo datanuke {} \;
```

**GDPR compliance (right to erasure):**
```bash
sudo datanuke customer_data.csv
rm customer_data.csv
```

**Decommissioning storage:**
```bash
find /mnt/old_drive -type f -exec sudo datanuke {} \;
```

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

## Technical Details

- **Algorithm**: AES-256-CBC (OpenSSL EVP API)
- **Key generation**: OpenSSL RAND_bytes() (CSPRNG)
- **Key wiping**: 7-pass Gutmann (0x00, 0xFF, random, 0x00, volatile)
- **Memory protection**: mlock() prevents swapping to disk
- **Platform support**: Windows, Linux, macOS

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## License

MIT License - see [LICENSE](LICENSE) file.

## References

- [BSI CON.6: Löschen und Vernichten](https://www.bsi.bund.de/)
- [NIST FIPS 197: AES Standard](https://csrc.nist.gov/publications/detail/fips/197/final)
- [OpenSSL Documentation](https://www.openssl.org/docs/)

---

**"Makes data powerless"** – Encryption + Key Destruction = Permanent Data Loss
