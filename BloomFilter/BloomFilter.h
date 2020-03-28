#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define __MGAIC_CODE__		(0x01464C42)

#define BYTE_BITS           (8)
#define MIX_UINT64(v)		((uint32_t)((v>>32)^(v)))
#define SETBIT(filter, n)   (filter->pstFilter[n / BYTE_BITS] |= (1 << (n % BYTE_BITS)))
#define GETBIT(filter, n)   (filter->pstFilter[n / BYTE_BITS] & (1 << (n % BYTE_BITS)))


typedef struct BaseBloomFilter
{
	uint8_t			cInitFlag;		// 初始化标志，为0时的第一次Add()会对stFilter[]做初始化
	uint8_t			cResv[3];

	uint32_t		dwMaxItems;		// n - BloomFilter的最大元素个数 (输入量)
	double			dProbFalse;		// p - 假阳概率，比如万分之一:0.00001 (输入量)
	uint32_t		dwFilterBits;	// m - BloomFilter的比特位数
	uint32_t		dwHashFuncs;	// k - BloomFilter的哈希函数个数

	uint32_t		dwSeed;			// MurmurHash的种子偏移量
	uint32_t		dwCount;		// Add()的计数，超过MAX_BLOOMFILTER_N则返回失败

	uint32_t		dwFilterSize;	// BloomFilter的总大小 dwFilterBits / 8
	unsigned char*	pstFilter;		// 存储BloomFilter的指针
	uint32_t*		pdwHashPos;		// 哈希结果数组
} BaseBloomFilter;

typedef struct BloomFileHead
{
	uint32_t	dwMagicCode;
	uint32_t	dwSeed;
	uint32_t	dwCount;

	uint32_t	dwMaxItems;
	double		dProbFalse;
	uint32_t	dwFilterBits;
	uint32_t	dwHashFuncs;

	uint32_t	dwResv[6];
	uint32_t	dwFileCrc;
	uint32_t	dwFilterSize;
} BloomFileHead;


// 计算BloomFilter的参数m,k
static inline void CalcBloomFilterParam(uint32_t n, double p, uint32_t* pm, uint32_t* pk);

// 根据目标精度和数据个数，初始化BloomFilter结构
inline int InitBloomFilter(BaseBloomFilter* pstBloomFilter, uint32_t dwSeed, uint32_t dwMaxItems, double dProbFalse);

// 释放BloomFilter
inline int FreeBloomFilter(BaseBloomFilter* pstBloomFilter);

// 重置BloomFilter
// 注意: Reset()函数不会立即初始化stFilter，而是当一次Add()时去memset
inline int ResetBloomFilter(BaseBloomFilter* pstBloomFilter);

// 和ResetBloomFilter不同，调用后立即memset内存
inline int RealResetBloomFilter(BaseBloomFilter* pstBloomfilter);


// MurmurHash2, 64-bit versions, by Austin Appleby
// https://sites.google.com/site/murmurhash/
uint64_t MurmurHash2_x64(const void * key, int len, uint32_t seed);


// 双重散列封装
void BloomHash(BaseBloomFilter* pstBloomFilter, const void* key, int len);

// 向BloomFilter中新增一个元素
// 成功返回0，当添加数据超过限制值时返回1提示用户
int BloomFilter_Add(BaseBloomFilter* pstBloomFilter, const void* key, int len);

// 检查一个元素是否在bloomfilter中
// 返回：0-存在，1-不存在，负数表示失败
int BloomFilter_Check(BaseBloomFilter* pstBloomFilter, const void* key, int len);


/* 文件相关封装 */
// 将生成好的BloomFilter写入文件
inline int SaveBloomFilterToFile(BaseBloomFilter* pstBloomFilter, char* szFileName);

// 从文件读取生成好的BloomFilter
inline int LoadBloomFilterFromFile(BaseBloomFilter* pstBloomFilter, char *szFileName);