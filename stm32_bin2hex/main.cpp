#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "hex.h"

#define LOGD(fmt, ...) printf("[DEBUG][%s]"   fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOGW(fmt, ...) printf("[WARNING][%s]" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[ERROR][%s]"   fmt "\n", __FUNCTION__, ##__VA_ARGS__)

static int32_t openFile(FILE** fp, const char* fileName)
{
    if(!fp || !fileName) {
        return -1;
    }
    *fp = fopen(fileName, "w+");
    if(*fp == NULL) {
        return -1;
    }
    return 0;
}
static int32_t closeFile(FILE** fp)
{
    if(!fp || !*fp) {
        return -1;
    }
    fclose(*fp);
    return 0;
}
static int32_t writeFile(FILE* fp, const char* data)
{
    // LOGD("hex data%s", data);
    fwrite(data, strlen(data), 1, fp);
    return 0;
}
static int32_t readFile(const char* fileName, uint8_t** buffer, uint32_t* len)
{
    FILE* fp = NULL;
    uint32_t n = 0;
    int32_t bRet = 0;
    if(!fileName || !strlen(fileName) || !buffer || !len) {
        LOGE("param error!");
        return -1;
    }
    *buffer = NULL;
    *len    = 0;

    fp = fopen(fileName, "r");
    if(!fp) {
        LOGE("open file %s error", fileName);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    *len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if(*len == 0) {
        LOGE("file %s size is null", fileName);
        bRet = -1;
        goto exit;
    }
    *buffer = (uint8_t *)malloc(sizeof(uint8_t) * (*len));
    if(!(*buffer)) {
        LOGE("malloc file %d buffer failed", (int32_t)(sizeof(uint8_t) * (*len)));
        bRet = -1;
        goto exit;
    }

    n = fread(*buffer, sizeof(unsigned char), *len, fp);
    if(n == 0) {
        LOGE("read file %s error", fileName);
        bRet = -1;
        goto exit;
    }
    if(n != (*len)) {
        LOGE("read file %s fail, fileLen %d, readLen %d", fileName, (int32_t)*len, (int32_t)n);
        bRet = -1;
        goto exit;
    }
    fclose(fp);
    return 0;
exit:
    if(fp) {
        fclose(fp);
    }
    if(*buffer) {
        free(*buffer);
    }
    *buffer = NULL;
    *len    = 0;
    return bRet;
}
static void createHexHead(uint32_t address, char* buffer, uint32_t len)
{
    hexUtils::hex_data_t hex;
    hex.recordType = hexUtils::HEX_RECORD_EXT_LINE_SEG_ADDR;
    hex.recordLen = 2;
    hex.loadOffset = 0;
    address >>= 16;
    hex.data[0] = (address & 0x0000FF00) >> 8;
    hex.data[1] = (address & 0x000000FF) >> 0;
    hexUtils::encodeHexData(&hex,  buffer, len);
}
static void createHexEndOfLine(char* buffer, uint32_t len)
{
    hexUtils::hex_data_t hex;
    hex.recordType = hexUtils::HEX_RECORD_ENDOFFILE;
    hex.recordLen = 0;
    hex.loadOffset = 0;
    hexUtils::encodeHexData(&hex, buffer, len);
}
static void createHexData(uint32_t offset, const uint8_t* data, uint32_t dataLen, char* outputBuffer, uint32_t outputBufferLen)
{
    hexUtils::hex_data_t hex;
    hex.recordType = hexUtils::HEX_RECORD_DATA;
    hex.recordLen = dataLen;
    hex.loadOffset = offset;
    memcpy(hex.data, data, dataLen);
    hexUtils::encodeHexData(&hex, outputBuffer, outputBufferLen);
}
int main(int argc, char** argv)
{
    if(argc <= 3) {
        printf("stm32_bin2hex [address] [bin file] [hex file]\n");
        return -1;
    }
    int binAddress = strtol(argv[1], NULL, 16);
    const char* binFile = argv[2];
    const char* hexFile = argv[3];
    uint8_t* fileBuffer = NULL;
    uint32_t fileLength = 0;
    FILE* outputFile = NULL; 
    char hexBuffer[256] = { 0 };
    LOGD("bin file:%s, address 0x%08x, output:%s", binFile, binAddress, hexFile);
    if(readFile(binFile, &fileBuffer, &fileLength)) {
        return -1;
    }
    if(openFile(&outputFile, hexFile) != 0) {
        return -1;
    }
    createHexHead(binAddress, hexBuffer, sizeof(hexBuffer));
    writeFile(outputFile, hexBuffer);
    for (uint32_t i = 0; ;)
    {
        if(i + 16 <= fileLength) {
            createHexData(i, &fileBuffer[i], 16, hexBuffer, sizeof(hexBuffer));
            writeFile(outputFile, hexBuffer);
        } else {
            createHexData(i, &fileBuffer[i], fileLength % 16, hexBuffer, sizeof(hexBuffer));
            writeFile(outputFile, hexBuffer);
            break;
        }
        i += 16;
        if(i == fileLength) {
            break;
        }
    }
    createHexEndOfLine(hexBuffer, sizeof(hexBuffer));
    writeFile(outputFile, hexBuffer);
    if(fileBuffer) {
        free(fileBuffer);
    }
    closeFile(&outputFile);
    return 0;
}
