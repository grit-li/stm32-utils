#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

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
static void printBuffer(const uint8_t* buffer, uint32_t len)
{
	uint32_t printSize = 0;

	while(printSize + 16 < len) {
		LOGD("%08xh: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ;", \
			printSize, \
			buffer[printSize +  0],	\
			buffer[printSize +  1], \
			buffer[printSize +  2], \
		        buffer[printSize +  3], \
			buffer[printSize +  4], \
			buffer[printSize +  5], \
			buffer[printSize +  6], \
			buffer[printSize +  7], \
			buffer[printSize +  8], \
			buffer[printSize +  9], \
			buffer[printSize + 10], \
			buffer[printSize + 11], \
			buffer[printSize + 12], \
			buffer[printSize + 13], \
			buffer[printSize + 14], \
			buffer[printSize + 15]);
		printSize += 16;
	}
	uint8_t tmp[16 * 3 + 1] = { 0 };
	for(uint32_t i = 0; i < len % 16; i++) {
		snprintf((char *)(tmp + i * 3), sizeof(tmp) - i * 3, "%02X ", buffer[printSize + i]);	
	}
	memset(tmp + (len % 16) * 3, ' ', sizeof(tmp) - (len % 16) * 3 - 1);
	LOGD("%08xh: %s;", printSize, tmp);
}
static const char* hexStreamToString(const uint8_t* buffer, uint32_t len)
{
	#define HEXSTRINGLEN  511
	static char tmp[HEXSTRINGLEN + 1] = { 0 };
	memset(tmp, 0, HEXSTRINGLEN);
	if(len > HEXSTRINGLEN) {
		LOGW("hex stream size %d > %d", len, HEXSTRINGLEN);
		len = HEXSTRINGLEN;
	}
	for(uint32_t i = 0; i < len; i++) {
		tmp[i] = buffer[i]; 
	}
	return tmp;
}
static void printHexData(const struct hex_data_t* hex)
{
	LOGD("mask: 0x%02x, len: %d, offset: 0x%08x, type: %d, checknum: 0x%02x", hex->recordMark, hex->recordLen, hex->loadOffset, hex->recordType, hex->chksum);
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
static bool hexCheckSum(const uint8_t* data, const struct hex_data_t* hex)
{
	bool bRet = false;
	uint8_t sum = 0;

	#define TI2(h) (OI(data[h]) << 4 | OI(data[h + 1]))
	sum += TI2(0);
	sum += TI2(2);
	sum += TI2(4);
	sum += TI2(6);
	
	for(uint32_t i = 0, j = 0; i < hex->recordLen; i++) {
		sum += TI2(j + 8);
		j += 2;
		
	}
	sum = ~sum + 1;
	if(sum == hex->chksum) {
		bRet = true;
	} else {
		LOGW("checksum error %02x, %02x", sum, hex->chksum);
	}
	return bRet;
}
static uint32_t hexDataToAddress(const uint8_t* data, uint8_t len)
{
	uint32_t bRet = 0;
	for(uint32_t i = 0; i < len; i++) {
		bRet = bRet << 8 | data[i];
	}
	return bRet;
}
static void parseHexData(const struct hex_data_t* hex)
{
	switch(hex->recordType) {
	//	case 0:
	//	case 1:
		case 2: LOGD("(0x%08x)extended segment address", hexDataToAddress(hex->data, hex->recordLen) << 4); break;
		case 3:	LOGD("(0x%08x)start segment address", hexDataToAddress(hex->data, hex->recordLen)); break;
		case 4: LOGD("(0x%08x)extended linear segment address", hexDataToAddress(hex->data, hex->recordLen) << 16); break;
		case 5: LOGD("(0x%08x)start linear segment address", hexDataToAddress(hex->data, hex->recordLen)); break;
		default:
			break;
	}
}
static void parseHexFile(const uint8_t* buffer, uint32_t len)
{
	struct hex_data_t hex;
	bool hexStartFlag = false;
	uint8_t tmp[512] = { 0 };
	uint32_t index = 0;
	uint32_t pos = 0;
	for(uint32_t i = 0; i < len; i++) {
		if(buffer[i] == ':') {
			hexStartFlag = true;
			memset(&hex, 0, sizeof(struct hex_data_t));
			memset(tmp, 0, sizeof(tmp));
			index = 0;
			pos = 0;
		}
		if(buffer[i] == 0x0A && buffer[i - 1] == 0x0D) {
			hexStartFlag = false;
			#define TI(h) (OI(tmp[h]) << 4 | OI(tmp[h + 1]))
			hex.recordMark = tmp[0];
			hex.recordLen = TI(1);
			hex.loadOffset = TI(3) << 8 | TI(5);
			hex.recordType = TI(7);
			pos = 9;
			for(uint32_t i = 0; i < hex.recordLen; i++) {
				hex.data[i] = TI(pos);
				pos += 2;	
			}
			hex.chksum = TI(pos);
			if(hexCheckSum(&tmp[1], &hex) == false) {
				printHexData(&hex);
				LOGD("%s", hexStreamToString(tmp, index));
			} else {
			// todo.
				parseHexData(&hex);
			}
		}
		if(hexStartFlag) {
			tmp[index++] = buffer[i];
		}
	}
}
int main(int argc, char** argv)
{
	if(argc <= 1) {
		printf("hexinfo [HEX FILE]\n");
		return -1;
	}
	const char* hexFile = argv[1];
	uint8_t* fileBuffer = NULL;
	uint32_t fileLength = 0;
	LOGD("hex file:%s", hexFile);
	if(readHexFile(hexFile, &fileBuffer, &fileLength)) {
		return -1;
	}
	//printBuffer(fileBuffer, fileLength);
	parseHexFile(fileBuffer, fileLength);
	if(fileBuffer) {
		free(fileBuffer);
	}
	return 0;
}
