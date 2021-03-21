#pragma once
// #ifndef _ENCRYPT_HPP_
// #define _ENCRYPT_HPP_

#include "debug.h"
#include <cstdlib>

class CENCRYPT {
typedef unsigned int UINT_T;
typedef unsigned char UCHAR_T;
#define DELTA 0x26DD7BB3 //0x9e3779b9
public:
	static size_t Alignment(UCHAR_T* pDataBuffer, size_t realSize, size_t alignSize) {
		size_t size = realSize;
		//判断是否对齐
		size_t isAlign = realSize & (alignSize-1);
		if (isAlign){//if没有对齐，调整大小使其对齐
			size = realSize+alignSize-isAlign;
			for (size_t i = realSize; i < size; ++i) {
				pDataBuffer[i] = 0;
			}
		}

		for (size_t i = 0; i < size; i++) {
			COUT("%02x ", pDataBuffer[i]);
		}
		COUT("\n");

		return size;
	}

	static void EncryByXOR(UCHAR_T* pDataBuffer, UCHAR_T secretKey, size_t dataSize) {
		if(!pDataBuffer) {
			COUT("pBuffer is nullptr!\n");
			return;
		}

		size_t length = dataSize/sizeof(UCHAR_T);
		for (size_t i = 0; i < length; ++i, ++pDataBuffer) {
			*pDataBuffer ^= secretKey;
		}
	}

	static size_t EncryptByTEA(UCHAR_T* data, const char* key, size_t dataSize, bool type = true) {
		const UINT_T delta = DELTA;
		const size_t alignSize = sizeof(UINT_T)*2;
		UINT_T keyPart[4];
		UINT_T left;
		UINT_T right;
		UINT_T i, j;

		UINT_T sum = 0;

		if(type){
			dataSize = Alignment(data, dataSize, alignSize);
		}
		UINT_T encryptCount = dataSize/alignSize;//获取加密块的次数

		for (i = 0, j = 0; i < 4; i++, j += 4) {
			keyPart[i] = (key[j] << 24) | (key[j + 1] << 16) | (key[j + 2] << 8) | (key[j + 3]);//将**使其转换成4个uint型数据
		}

		for (i = 0, j = 0; i < encryptCount; i++, j += alignSize) {
			//初始化
			left = data[j] << 24 | data[j + 1] << 16 | data[j + 2] << 8 | data[j + 3];//获得四字节 作为left
			right = data[j + 4] << 24 | data[j + 5] << 16 | data[j + 6] << 8 | data[j + 7];//获得四字节 作为 right

			if(type) {//加密
				sum = 0;
				for (size_t k = 0; k < 32; k++) {//一般为32轮
					sum += delta;
					left += ((right << 4) + keyPart[0]) ^ (right + sum) ^ ((right >> 5) + keyPart[1]);//对左半部分进行操作
					right += ((left << 4) + keyPart[2]) ^ (left + sum) ^ ((left >> 5) + keyPart[3]);//对右半部分进行操作
				}
			} else {//解密
				sum = delta*32;
				for (size_t k = 0; k < 32; k++) {//一般为32轮
					right -= ((left << 4) + keyPart[2]) ^ (left + sum) ^ ((left >> 5) + keyPart[3]);//对右半部分进行操作
					left -= ((right << 4) + keyPart[0]) ^ (right + sum) ^ ((right >> 5) + keyPart[1]);//对左半部分进行操作
					sum -= delta;
				}
			}

			//加密后的结果
			data[j] = (left >> 24) & 0xff;
			data[j + 1] = (left >> 16) & 0xff;
			data[j + 2] = (left >> 8) & 0xff;
			data[j + 3] = left & 0xff;
			data[j + 4] = (right >> 24) & 0xff;
			data[j + 5] = (right >> 16) & 0xff;
			data[j + 6] = (right >> 8) & 0xff;
			data[j + 7] = right & 0xff;
		}

		//输出结果
		for (size_t i = 0; i < dataSize; i++) {
			COUT("%02x ", data[i]);
		}
		COUT("\n");

		return dataSize;
	}
};
// #endif