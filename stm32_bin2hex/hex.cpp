#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "hex.h"

static char hextoint(char c)
{
    if('0' <= c && c <= '9') {
        return c - '0';
    }
    if('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }
    if('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0;
}

static uint8_t hexCheckSum(const char* data, uint32_t len)
{
    int8_t bRet = 0;

    #define TI2(h) (hextoint(data[h]) << 4 | hextoint(data[h + 1]))
    bRet += TI2(0);
    bRet += TI2(2);
    bRet += TI2(4);
    bRet += TI2(6);
    
    for(uint32_t i = 0, j = 0; i < len; i++) {
        bRet += TI2(j + 8);
        j += 2;
        
    }
    bRet = ~bRet + 1;
    return bRet;
}

bool hexUtils::encodeHexData(const struct hex_data_t* hex, char* buffer, uint32_t len)
{
    if(!hex || !buffer || !len) {
        return false;
    }
    uint32_t n = 0;
    memset(buffer, 0, len);
    if((16 + hex->recordLen * 2) > len) {
        return false;
    }
    n = snprintf(buffer, len, ":%02X%04X%02X", hex->recordLen, hex->loadOffset, hex->recordType);
    for(uint32_t i = 0; i < hex->recordLen; i++) {
        n += snprintf(&buffer[n], len - n, "%02X", hex->data[i]);
    }
    snprintf(&buffer[n], len - n, "%02X\r\n", hexCheckSum(&buffer[1], n - 1));
    return true;
}
