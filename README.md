# DSE
AES encryption using keyfile

## Usage
- Encrypt or decrypt the source file into the destination file using a key file:
```
dse <keyfile> <e | d> <source> <destination>
```
- Create a random-content key file:
```
dse <keyfile> [password]
```
If `password` is provided, which is optional, the keyfile will be generated from it.

The generated key file has a size of 32 bytes.

- Output version number:
```
dse <-V | --version>
```

## Example
- Encryption:
```
dse my.key e data data.enc
```
- Decryption:
```
dse my.key d data.enc data
```