#ifndef __STM32_IBOOT_CRC_H__
#define __STM32_IBOOT_CRC_H__

#include <stdint.h>

uint32_t crc32(uint32_t, const unsigned char *, unsigned int);
uint32_t crc32_wd(uint32_t, const unsigned char *, unsigned int, unsigned int);
uint32_t crc32_no_comp(uint32_t, const unsigned char *, unsigned int);

#endif /* __STM32_IBOOT_CRC_H__ */
