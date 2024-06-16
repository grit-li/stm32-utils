#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>

#define LOGD(fmt, ...) printf("[DEBUG][%s]"   fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOGW(fmt, ...) printf("[WARNING][%s]" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[ERROR][%s]"   fmt "\n", __FUNCTION__, ##__VA_ARGS__)

struct hex_data_t {
    uint8_t recordMark;
    uint8_t recordLen;
    uint16_t loadOffset;
    uint8_t  recordType;
    uint8_t  data[256];
    uint8_t  chksum;
};
struct hex_data_array_t
{
    uint8_t* fileBuffer;
    uint32_t fileLength;
};
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
static int32_t readHexFile(const char* fileName, uint8_t** buffer, uint32_t* len)
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
static uint8_t OI(uint8_t c)
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
static void copyHexFile(FILE* fp, const struct hex_data_array_t* data)
{
    uint8_t recordType = 0xFF;
    bool hexStartFlag = false;
    uint8_t tmp[512] = { 0 };
    uint32_t index = 0;
    for(uint32_t i = 0; i < data->fileLength; i++) {
        if(data->fileBuffer[i] == ':') {
            hexStartFlag = true;
            recordType = 0xFF;
            memset(tmp, 0, sizeof(tmp));
            index = 0;
        }
        if(hexStartFlag) {
            tmp[index++] = data->fileBuffer[i];
        }
        if(data->fileBuffer[i] == 0x0A && data->fileBuffer[i - 1] == 0x0D) {
            hexStartFlag = false;
            #define TI(h) (OI(tmp[h]) << 4 | OI(tmp[h + 1]))
            recordType = TI(7);
            switch(recordType) {
                case 0: writeFile(fp, (const char *)tmp); break;
                case 1: break;
                case 2: writeFile(fp, (const char *)tmp); break;
                case 3: writeFile(fp, (const char *)tmp); break;
                case 4: writeFile(fp, (const char *)tmp); break;
                case 5: break;
                default:
                    break;
            }
        }
    }
}
int main(int argc, char** argv)
{
    if(argc <= 2) {
        printf("stm32_hexmerge [OUTPUT FILE] [HEX FILE]...\n");
        return -1;
    }
    FILE* outputFile = NULL;
    char* hexFile = NULL;
    struct hex_data_array_t hexData;
    std::vector<struct hex_data_array_t> hexArray;
    hexArray.clear();
    if(openFile(&outputFile, argv[1]) != 0) {
        return -1;
    }
    for(uint32_t i = 2; i < argc; i++) {
        hexFile = argv[i];
        LOGD("hex file:%s", hexFile);
        if(readHexFile(hexFile, &hexData.fileBuffer, &hexData.fileLength)) {
            LOGW("error hex file!");
            continue;
        }
        hexArray.push_back(hexData);
    }
    
    for(uint32_t i = 0; i < hexArray.size(); i++) {
        copyHexFile(outputFile, &hexArray[i]);
        if(hexArray[i].fileBuffer) {
            free(hexArray[i].fileBuffer);
        }
    }
    const char* hexEndOfLine = ":00000001FF\x0D\x0A";
    writeFile(outputFile, hexEndOfLine);
    closeFile(&outputFile);
    hexArray.clear();
    return 0;
}
