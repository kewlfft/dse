# DSE
AES encryption using keyfile

## Usage
```
dse <keyfile> <e | d> <source> <destination>
```
- Create a random-content key file:
```
dse <keyfile> [password]
```
If `password` is provided, the keyfile will be generated from it.
Key file size is 32 bytes.

- Output version number
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