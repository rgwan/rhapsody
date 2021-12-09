#include <stdio.h>
#include <openssl/des.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <string.h>

char ios_license_key[] = "Venusutm";
char ios_license_iv[] = "licenses";

char *base64_encode(const uint8_t *input, int length)
{
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length + 1] = 0;

    BIO_free_all(b64);

    return buff;
}
uint8_t *base64_decode(unsigned char *input, int length)
{
    BIO *b64 = NULL;
    BIO *bmem = NULL;
    uint8_t *buffer = (char *)malloc(length);
    memset(buffer, 0, length);

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf(input, length);
    bmem = BIO_push(b64, bmem);
    BIO_read(bmem, buffer, length);

    BIO_free_all(bmem);
    return buffer;
}

void decrypt_buf(char *in, char *out, int len)
{
    DES_cblock iv;
    DES_cblock key;
    DES_key_schedule ks;

    memcpy(&key, ios_license_key, 8);
    DES_set_key(&key, &ks);
    memcpy(&iv, ios_license_iv, 8);
    DES_ncbc_encrypt(in, out, len, &ks, &iv, 0);
}

//cc deraisecom.c -o deraisecom -lssl -lcrypto && ./deraisecom && xxd tmp

int main(int argc, char *argv[])
{
    char *hpass = argv[1]; //"5ZMfsRFY61KWtNkv0KQrUg0MLY8Klt8iyZr0ykghgayovi9a4VxqRtDnwqz26py";
                           // BJPqJJx+U9b8x11T1H9fzkwIKPpe2IYf0nvSw1WtQQhPqDyPOyJNZCdHbb7j6n+
                           // 4kk2i75d28Mrmhv+fyTgUYFUpsD+TK0baeaKJjDTZFNDSSs0YwuqYpkBli6cSsh
    char *lpass = argv[2]; //"fMhlEDRRuxWTPtyk3ELemzJoKgSSNsKLP216Gw0ttrIBr0s8dcZmyyWkBC2n3I/";

    char *hpass_p;
    char *lpass_p;

    char dec_hpass[48];
    char dec_lpass[48];
    char allpass[128];
    int i, len;

    if (argc == 1)
    {
        printf("Usage: %s <hpass> <lpass>\n", argv[0]);
        printf("Usage: %s <encpass>\n", argv[0]);
        return -1;
    }
    else if (argc == 3)
    {

        hpass_p = base64_decode(hpass, strlen(hpass));
        lpass_p = base64_decode(lpass, strlen(hpass));

        decrypt_buf(hpass_p, dec_hpass, 48);
        decrypt_buf(lpass_p, dec_lpass, 48);

        len = strlen(dec_hpass) + strlen(dec_lpass);
        int j = 0, k = 0;
        for (i = 0; i < len; i++)
            if ((i & 1) != 0)
                allpass[i] = dec_lpass[j++];
            else
                allpass[i] = dec_hpass[k++];

        allpass[i] = 0;
    }
    else
    {
        memset(allpass, 0x00, 128);
        strncpy(allpass, argv[1], 120);
        len = strlen(allpass);
        if (len == 63)
        {
            lpass_p = base64_decode(argv[1], len);
            decrypt_buf(lpass_p, allpass, 48);
            goto showpass; // 'zhjrqmwg!' I have no idea what the f*ck is.
        }
    }

    for (i = 0; i < len; i++)
        allpass[i] = allpass[i] - i - 5;
    allpass[i] = 0;
    
showpass:
    printf("password is '%s'\n", allpass);
    return 0;
}
