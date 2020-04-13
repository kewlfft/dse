/*=======================================================================
DSE                                               first rev: 2004.09.17

Copyright (c) 2004 Dariusz Stanislawek
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================

DSE is a file encryption program.

Cipher: AES (Rijndael)
Mode: CFB-128
Key: 256 bits
IV: 16 bytes
Max File Size: unlimited
Cipher File Structure: [IV = 16 random bytes] + [cipherdata]

Website
-------
http://www.ozemail.com.au/~nulifetv/freezip/freeware/
=======================================================================

Kewl <xrjy@nygb.rh.bet(rot13)> for the linux adaptations
=======================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "rijndael-alg-fst.h"

#define KEY_SIZE 32 // bytes
#define IV_SIZE 16
#define IO_SIZE (64 * 1024) // 64k buffer size for sequential I/O

//typedef unsigned char u8;
//typedef unsigned int u32;


void gen_iv(u8 *buf, int size)
{
    while(--size >= 0) buf[size] += rand();
}

char vers[] = "1.32";
char msg1[] = "The data is invalid: %s\n";
char msg2[] = "The file already exists: %s\n";


/*=====================================================================*/


int crypt(char *keyfile, int encrypt, char *src, char *dst)
{
    int Nr; /* key-length-dependent number of rounds */
    u32 rk[4*(MAXNR + 1)]; /* key schedule */
    FILE *fkey, *fsrc, *fdst;
    u8 iv[IV_SIZE], cipherKey[KEY_SIZE], filebuf[IO_SIZE], block[16], *in, *out;
    int i, numBlocks, sread = 0, sreadtot = 0, status = 1, round = 0;

    if((fdst = fopen(dst, "r")) != NULL) // check if file exists
    {
        printf(msg2, dst);
        fclose(fdst);
        return 1;
    }

    if((fkey = fopen(keyfile, "rb")) == NULL)
    {
        perror(keyfile);
        return 1;
    }

    if((fsrc = fopen(src, "rb")) == NULL)
    {
        perror(src);
        goto quit;
    }

    if((fdst = fopen(dst, "wb")) == NULL)
    {
        perror(dst);
        goto quit;
    }

    if(KEY_SIZE != fread(cipherKey, 1, KEY_SIZE + 1, fkey))
    {
        printf(msg1, keyfile);
        goto quit;
    }

    if(encrypt)
    {
        memcpy(filebuf, src, strlen(src)+1); // PRNG init 1
        for(i = (IO_SIZE / 4) - 1; i >= 0; i--) sread += ((int*)filebuf)[i]; // PRNG init 2
        srand(sread ^ time(NULL)); // PRNG init 3
        gen_iv(iv, IV_SIZE); // PRNG
        if(IV_SIZE != fwrite(iv, 1, IV_SIZE, fdst))
        {
            printf(msg1, dst);
            goto quit;
        }
    }
    else
    {
        if(IV_SIZE != fread(iv, 1, IV_SIZE, fsrc))
        {
            printf(msg1, src);
            goto quit;
        }
    }

    Nr = rijndaelKeySetupEnc(rk, cipherKey, KEY_SIZE * 8);

    while((sread = fread(filebuf, 1, IO_SIZE, fsrc)) > 0)
    {
        sreadtot += sread;
        in = iv;
        out = filebuf;
        numBlocks = sread / 16;
        if(sread % 16) numBlocks++;
        for(i = numBlocks; i > 0; i--) // AES/CFB-128
        {
            rijndaelEncrypt(rk, Nr, in, block);
            if(!encrypt) memcpy(iv, out, 16);
            ((u32*)out)[0] ^= ((u32*)block)[0];
            ((u32*)out)[1] ^= ((u32*)block)[1];
            ((u32*)out)[2] ^= ((u32*)block)[2];
            ((u32*)out)[3] ^= ((u32*)block)[3];
            if(encrypt) in = out;
            out += 16;
        }
        if(encrypt) memcpy(iv, in, 16);
        if(sread != fwrite(filebuf, 1, sread, fdst))
        {
            printf(msg1, dst);
            goto quit;
        }
        round++;
        if(!(round % 16)) printf("."); // progress indicator every 16 rounds
    }
    printf("Done %s %u Bytes\n", dst, sreadtot);
    status = 0; // SUCCESS

quit:
    fclose(fkey);
    if(fsrc) fclose(fsrc);
    if(fdst)
    {
        fclose(fdst);
        if(status) remove(dst);
    }
    if(!encrypt) memset(filebuf, 0, sizeof(filebuf)); // sensitive data memory cleanup
    memset(cipherKey, 0, sizeof(cipherKey));
    rijndaelKeySetupEnc(rk, cipherKey, KEY_SIZE * 8);
    return status;
}


/*=====================================================================*/


int passgets(char *s, int n)
{
    int c;
    char *t;

    t = s;
    while(--n >= 0)
    {
        c = getchar();
        if(c < 32 || c > 126) break;
        *s++ = c;
    }
    *s = 0;
    return s != t;
}


int password(char *pass)
{
    char temp[33];

    memset(pass, 0, 32);
    printf("Password: ");
    if(passgets(pass, 32))
    {
        printf("\nVerify  : ");
        if(passgets(temp, 32) && !strcmp(pass, temp))
        {
            memset(temp, 0, 32);
            printf("\n");
            return 1;
        }
    }
    printf("\nError\n");
    return 0;
}


int gen_key(char *dst, int pass) // keyfile name, pass true to request password input
{
    int i, sum = 0, Nr;
    u32 rk[4*(MAXNR + 1)];
    FILE *fdst;
    u8 block[512];

    if((fdst = fopen(dst, "r")) != NULL) // check if keyfile already exists
    {
        printf(msg2, dst);
        fclose(fdst);
        return 1;
    }

    if(pass) // request password input
    {
        if(!password((char *)block)) return 1;
    }
    else memcpy(block, dst, strlen(dst)+1); // PRNG init 1: copy keyfile name

    if((fdst = fopen(dst, "wb")) == NULL) // open keyfile for wb
    {
        perror(dst);
        return 1;
    }

    Nr = rijndaelKeySetupEnc(rk, block, 32 * 8);
    if(!pass) // generate random key
    {
        for(i = (sizeof(block) / 4) - 1; i >= 0; i--) sum += ((int*)block)[i]; // PRNG init 2: add first 128 Bytes
        srand(sum ^ time(NULL)); // PRNG init 3: set random seed (sum XOR time)
        gen_iv(block, 16); // PRNG: random 16 bytes
    }
    rijndaelEncrypt(rk, Nr, block, block + 16);
    rijndaelEncrypt(rk, Nr, block + 16, block);
    fwrite(block, 1, 32, fdst); // save keyfile
    fclose(fdst);
    memset(block, 0, 32); // overwrite with 0
    printf("Keyfile saved: %s\n", dst);
    return 0;
}

int usage(int level)
{
    printf("usage:\n"
       "\tdse <keyfile> [password]\n"
       "\tdse <keyfile> <e | d> <source> <destination>\n"
       "\tdse <-V | --version>\n"
       "\tdse <-h | --help>\n");
    if (level>0)
    {
        printf("examples:\n"
           "\tdse mykey                  creates a random-content 32 bytes keyfile named mykey\n"
           "\tdse mykey mypsw            creates a 32 bytes keyfile from the password mypsw\n"
           "\tdse mykey e data data.enc  encryption\n"
           "\tdse mykey d data.enc data  decryption\n");
    }
    return 0;
}

/*=====================================================================*/


int main(int argc, char *argv[])
{
    if(argc == 2)
    {
        if (strcmp(argv[1], "-V") == 0 || strcmp(argv[1], "--version") == 0)
        {
            printf("dse %s\n", vers);
            return 0;
        }
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            usage(1);
            return 0;
        }        
        if (argv[1][0] == '-')
        {
            usage(0);
            return 1;
        }
    }
    // key generator
    if(argc == 2 || argc == 3) return gen_key(argv[1], argc == 3);

    // file encrypt or decrypt
    if(argc == 5)
    {
        *argv[2] = tolower(*argv[2]);
        switch (*argv[2])
        {
            case 'e':
            case 'd':
                return crypt(argv[1], *argv[2] == 'e', argv[3], argv[4]);
        }
    }

    usage(0);
    return 1;
}
