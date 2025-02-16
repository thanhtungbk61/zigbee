    /***********************************************************
    * Base64 library                                           *
    * @author Ahmed Elzoughby                                  *
    * @date July 23, 2017                                      *
    * Purpose: encode and decode base64 format                 *
    ***********************************************************/

#ifndef BASE46_H
#define BASE46_H

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>


/***********************************************
Encodes ASCCI string into base64 format string
@param plain ASCII string to be encoded
@return encoded base64 format string
***********************************************/
char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);


/***********************************************
decodes base64 format string into ASCCI string
@param plain encoded base64 format string
@return ASCII string to be encoded
***********************************************/
char* base64_decode(char* cipher, int *len);


#endif //BASE46_H
