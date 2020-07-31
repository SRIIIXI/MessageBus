/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Base64.h"
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

const char encodingtable[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								'w', 'x', 'y', 'z', '0', '1', '2', '3',
								'4', '5', '6', '7', '8', '9', '+', '/' };

const int modulustable[] = { 0, 2, 1 };


char *base64_encode(const unsigned char *data, unsigned long inputlength, char *encodedString, unsigned long *outputlength)
{
	*outputlength = 4 * ((inputlength + 2) / 3);

	encodedString = (char*)calloc(1, (size_t)(*outputlength) + 1);

	if (encodedString == NULL)
	{
		outputlength = 0;
        return NULL;
	}

	for (unsigned int i = 0, j = 0; i < inputlength;)
	{

		uint32_t octet_a = i < inputlength ? (unsigned char)data[i++] : 0;
		uint32_t octet_b = i < inputlength ? (unsigned char)data[i++] : 0;
		uint32_t octet_c = i < inputlength ? (unsigned char)data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encodedString[j++] = encodingtable[(triple >> 3 * 6) & 0x3F];
		encodedString[j++] = encodingtable[(triple >> 2 * 6) & 0x3F];
		encodedString[j++] = encodingtable[(triple >> 1 * 6) & 0x3F];
		encodedString[j++] = encodingtable[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < modulustable[inputlength % 3]; i++)
	{
		encodedString[*outputlength - 1 - i] = '=';
	}

    return encodedString;
}


unsigned char *base64_decode(const char *encodedString, unsigned long inputlength, unsigned char *decodedData, unsigned long *outputlength)
{
	char decodingtable[256] = { 0 };

	for (int i = 0; i < 64; i++)
	{
		decodingtable[(unsigned char)encodingtable[i]] = i;
	}

    if (inputlength % 4 != 0) return NULL;

	*outputlength = inputlength / 4 * 3;

	if (encodedString[inputlength - 1] == '=') (*outputlength)--;
	if (encodedString[inputlength - 2] == '=') (*outputlength)--;

	decodedData = (unsigned char*)calloc(1, *outputlength);

	if (decodedData == NULL)
	{
		*outputlength = 0;
        return NULL;
	}

	for (unsigned int i = 0, j = 0; i < inputlength;)
	{
		uint32_t sextet_a = encodedString[i] == '=' ? 0 & i++ : decodingtable[encodedString[i++]];
		uint32_t sextet_b = encodedString[i] == '=' ? 0 & i++ : decodingtable[encodedString[i++]];
		uint32_t sextet_c = encodedString[i] == '=' ? 0 & i++ : decodingtable[encodedString[i++]];
		uint32_t sextet_d = encodedString[i] == '=' ? 0 & i++ : decodingtable[encodedString[i++]];

		uint32_t triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

		if (j < *outputlength) decodedData[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *outputlength) decodedData[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *outputlength) decodedData[j++] = (triple >> 0 * 8) & 0xFF;
	}

    return decodedData;
}
