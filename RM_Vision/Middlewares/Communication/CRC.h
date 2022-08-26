#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 包含头文件 ----------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

/* 类型定义 ------------------------------------------------------------------*/

/* 宏定义 --------------------------------------------------------------------*/

/* 扩展变量 ------------------------------------------------------------------*/

/* 函数声明 ------------------------------------------------------------------*/
uint8_t verify_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength);
uint8_t verify_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength);

void append_crc8_check_sum(uint8_t* pchMessage, uint16_t dwLength);
void append_crc16_check_sum(uint8_t* pchMessage, uint32_t dwLength);


#ifdef __cplusplus
}
#endif

#endif  // __CRC_H__

