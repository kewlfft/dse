# DSE
AES encryption using keyfile

## Usage
dse keyfile e|d source destination
Create a random-content key file: dse keyfile
Create a key file from a password: dse keyfile p
Key file size is 32 bytes.

## Example
* Encryption example:  dse my.key e data.zip data.enc
* Decryption example:  dse my.key d data.enc data.zip