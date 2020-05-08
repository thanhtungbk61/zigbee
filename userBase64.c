    /***********************************************************
    * Base64 library implementation                            *
    * @author Ahmed Elzoughby                                  *
    * @date July 23, 2017                                      *
    ***********************************************************/

#include "userBase64.h"


char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base46_map[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base46_map[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base46_map[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base46_map[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    encoded_data[*output_length] = '\0';
    return encoded_data;
}
//char* base64_encode(char* plain, int len) {
//
//    char counts = 0;
//    char buffer[3];
//    char* cipher = malloc(len * 4 / 3 + 4);
//    int i = 0, c = 0;
//
//    for(i = 0; plain[i] < len; i++) {
//        buffer[counts++] = plain[i];
//        if(counts == 3) {
//            cipher[c++] = base46_map[buffer[0] >> 2];
//            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
//            cipher[c++] = base46_map[((buffer[1] & 0x0f) << 2) + (buffer[2] >> 6)];
//            cipher[c++] = base46_map[buffer[2] & 0x3f];
//            counts = 0;
//        }
//    }
//
//    if(counts > 0) {
//        cipher[c++] = base46_map[buffer[0] >> 2];
//        if(counts == 1) {
//            cipher[c++] = base46_map[(buffer[0] & 0x03) << 4];
//            cipher[c++] = '=';
//        } else {                      // if counts == 2
//            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
//            cipher[c++] = base46_map[(buffer[1] & 0x0f) << 2];
//        }
//        cipher[c++] = '=';
//    }
//
//    cipher[c] = '\0';   /* string padding character */
//    return cipher;
//}

char* base64_decode(char* cipher, int *len) {

    char counts = 0;
    char buffer[4];
    char* plain = malloc(strlen(cipher) * 3 / 4);
    int i = 0, p = 0;

    for(i = 0; cipher[i] != '\0'; i++) {
        char k;
        for(k = 0 ; k < 64 && base46_map[k] != cipher[i]; k++);
        buffer[counts++] = k;
        if(counts == 4) {
            plain[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
            if(buffer[2] != 64)
                plain[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
            if(buffer[3] != 64)
                plain[p++] = (buffer[2] << 6) + buffer[3];
            counts = 0;
        }
    }

    plain[p] = '\0';    /* string padding character */
    *len =p;
    return plain;
}
