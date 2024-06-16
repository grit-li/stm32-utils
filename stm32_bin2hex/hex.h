#ifndef __STM32_BIN2HEX_H__
#define __STM32_BIN2HEX_H__

#include <cstdint>


class hexUtils
{
public:
    enum {
        HEX_RECORD_DATA                 = 0x00,
        HEX_RECORD_ENDOFFILE            = 0x01,
        HEX_RECORD_EXT_SEG_ADDR         = 0x02,
        HEX_RECORD_ST_SEG_ADDR          = 0x03,
        HEX_RECORD_EXT_LINE_SEG_ADDR    = 0x04,
        HEX_RECORD_ST_LINE_SEG_ADDR     = 0x05,
    };
    struct hex_data_t {
        uint8_t recordLen;
        uint16_t loadOffset;
        uint8_t  recordType;
        uint8_t  data[256];
    };
    static bool encodeHexData(const struct hex_data_t*, char* , uint32_t);
};

#endif