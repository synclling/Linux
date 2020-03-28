#include "BloomFilter.h"

static inline void CalcBloomFilterParam(uint32_t n, double p, uint32_t* pm, uint32_t* pk)
{
	/**
	 *  n - Number of items in the filter
	 *  p - Probability of false positives, float between 0 and 1 or a number indicating 1-in-p
	 *  m - Number of bits in the filter
	 *  k - Number of hash functions
	 *
	 *  f = ln(2) × ln(1/2) × m / n = (0.6185) ^ (m/n)
	 *  m = -1 * ln(p) × n / 0.6185
	 *  k = ln(2) × m / n = 0.6931 * m / n
	**/

	uint32_t m, k;

	// 计算指定假阳概率下需要的比特数
	m = (uint32_t)ceil(-1 * log(p) * n / 0.6185);
	m = (m - m % 64) + 64;                  // 8字节对齐

	// 计算哈希函数个数
	k = (uint32_t)(0.6931 * m / n);
	k++;

	*pm = m;
	*pk = k;
}

inline int InitBloomFilter(BaseBloomFilter* pstBloomFilter, uint32_t dwSeed, uint32_t dwMaxItems, double dProbFalse)
{
	if (pstBloomFilter == NULL)
		return -1;

	if (dProbFalse <= 0 || dProbFalse >= 1)
		return -2;

	if (pstBloomFilter->pstFilter != NULL)
	{
		free(pstBloomFilter->pstFilter);
	}
	if (pstBloomFilter->pdwHashPos != NULL)
	{
		free(pstBloomFilter->pdwHashPos);
	}

	memset(pstBloomFilter, 0, sizeof(BaseBloomFilter));

	pstBloomFilter->dwMaxItems = dwMaxItems;
	pstBloomFilter->dProbFalse = dProbFalse;
	pstBloomFilter->dwSeed = dwSeed;

	CalcBloomFilterParam(pstBloomFilter->dwMaxItems, pstBloomFilter->dProbFalse, 
						&pstBloomFilter->dwFilterBits, &pstBloomFilter->dwHashFuncs);

	pstBloomFilter->dwFilterSize = pstBloomFilter->dwFilterBits / 8;
	pstBloomFilter->pstFilter = (unsigned char*)malloc(pstBloomFilter->dwFilterSize);
	if (pstBloomFilter->pstFilter == NULL)
	{
		return -100;
	}
	// 初始化BloomFilter的内存
	memset(pstBloomFilter->pstFilter, 0, pstBloomFilter->dwFilterSize);


	// 哈希结果数组
	pstBloomFilter->pdwHashPos = (uint32_t*)malloc(pstBloomFilter->dwHashFuncs * sizeof(uint32_t));
	if (pstBloomFilter->pdwHashPos == NULL)
	{
		return -200;
	}

	pstBloomFilter->cInitFlag = 1;

	printf(">>> Init BloomFilter(n=%u, p=%f, m=%u, k=%d), malloc() size=%.2fMB\n",
		pstBloomFilter->dwMaxItems, pstBloomFilter->dProbFalse, pstBloomFilter->dwFilterBits,
		pstBloomFilter->dwHashFuncs, (double)pstBloomFilter->dwFilterSize / 1024 / 1024);

	return 0;
}

inline int FreeBloomFilter(BaseBloomFilter* pstBloomFilter)
{
	if (pstBloomFilter == NULL)
		return -1;

	pstBloomFilter->cInitFlag = 0;
	pstBloomFilter->dwCount = 0;

	free(pstBloomFilter->pstFilter);
	pstBloomFilter->pstFilter = NULL;
	free(pstBloomFilter->pdwHashPos);
	pstBloomFilter->pdwHashPos = NULL;
	return 0;
}

inline int ResetBloomFilter(BaseBloomFilter* pstBloomFilter)
{
	if (pstBloomFilter == NULL)
		return -1;

	pstBloomFilter->cInitFlag = 0;
	pstBloomFilter->dwCount = 0;
	return 0;
}

inline int RealResetBloomFilter(BaseBloomFilter* pstBloomFilter)
{
	if (pstBloomFilter == NULL)
		return -1;

	memset(pstBloomFilter->pstFilter, 0, pstBloomFilter->dwFilterSize);
	pstBloomFilter->cInitFlag = 1;
	pstBloomFilter->dwCount = 0;
	return 0;
}

uint64_t MurmurHash2_x64(const void * key, int len, uint32_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len / 8);

	while (data != end)
	{
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const uint8_t * data2 = (const uint8_t*)data;

	switch (len & 7)
	{
	case 7: h ^= ((uint64_t)data2[6]) << 48;
	case 6: h ^= ((uint64_t)data2[5]) << 40;
	case 5: h ^= ((uint64_t)data2[4]) << 32;
	case 4: h ^= ((uint64_t)data2[3]) << 24;
	case 3: h ^= ((uint64_t)data2[2]) << 16;
	case 2: h ^= ((uint64_t)data2[1]) << 8;
	case 1: h ^= ((uint64_t)data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

void BloomHash(BaseBloomFilter* pstBloomFilter, const void* key, int len)
{
	int i;
	uint32_t dwFilterBits = pstBloomFilter->dwFilterBits;

	uint64_t hash1 = MurmurHash2_x64(key, len, pstBloomFilter->dwSeed);
	uint64_t hash2 = MurmurHash2_x64(key, len, MIX_UINT64(hash1));

	for (i = 0; i < (int)pstBloomFilter->dwHashFuncs; ++i)
	{
		pstBloomFilter->pdwHashPos[i] = (hash1 + i * hash2) % dwFilterBits;
	}
}

int BloomFilter_Add(BaseBloomFilter *pstBloomFilter, const void* key, int len)
{
	if ((pstBloomFilter == NULL) || (key == NULL) || (len <= 0))
		return -1;

	int i;

	if (pstBloomFilter->cInitFlag != 1)
	{
		// Reset后没有初始化，使用前需要memset
		memset(pstBloomFilter->pstFilter, 0, pstBloomFilter->dwFilterSize);
		pstBloomFilter->cInitFlag = 1;
	}

	// hash key到bloomfilter中
	BloomHash(pstBloomFilter, key, len);
	for (i = 0; i < (int)pstBloomFilter->dwHashFuncs; i++)
	{
		SETBIT(pstBloomFilter, pstBloomFilter->pdwHashPos[i]);
	}

	// 增加count数
	pstBloomFilter->dwCount++;
	if (pstBloomFilter->dwCount <= pstBloomFilter->dwMaxItems)
		return 0;
	else
		return 1;       // 超过N最大值，可能出现准确率下降等情况
}

int BloomFilter_Check(BaseBloomFilter* pstBloomFilter, const void* key, int len)
{
	if ((pstBloomFilter == NULL) || (key == NULL) || (len <= 0))
		return -1;

	BloomHash(pstBloomFilter, key, len);

	int i;
	for (i = 0; i < (int)pstBloomFilter->dwHashFuncs; i++)
	{
		// 如果有任意bit不为1，说明key不在bloomfilter中
		// 注意: GETBIT()返回不是0|1，高位可能出现128之类的情况
		if (GETBIT(pstBloomFilter, pstBloomFilter->pdwHashPos[i]) == 0)
			return 1;
	}

	return 0;
}

inline int SaveBloomFilterToFile(BaseBloomFilter* pstBloomFilter, char* szFileName)
{
	if ((pstBloomFilter == NULL) || (szFileName == NULL))
		return -1;

	int iRet;
	FILE *pFile;
	static BloomFileHead stFileHeader = { 0 };

	pFile = fopen(szFileName, "wb");
	if (pFile == NULL)
	{
		perror("fopen");
		return -11;
	}

	// 先写入文件头
	stFileHeader.dwMagicCode = __MGAIC_CODE__;
	stFileHeader.dwSeed = pstBloomFilter->dwSeed;
	stFileHeader.dwCount = pstBloomFilter->dwCount;
	stFileHeader.dwMaxItems = pstBloomFilter->dwMaxItems;
	stFileHeader.dProbFalse = pstBloomFilter->dProbFalse;
	stFileHeader.dwFilterBits = pstBloomFilter->dwFilterBits;
	stFileHeader.dwHashFuncs = pstBloomFilter->dwHashFuncs;
	stFileHeader.dwFilterSize = pstBloomFilter->dwFilterSize;

	iRet = fwrite((const void*)&stFileHeader, sizeof(stFileHeader), 1, pFile);
	if (iRet != 1)
	{
		perror("fwrite(head)");
		return -21;
	}

	// 接着写入BloomFilter的内容
	iRet = fwrite(pstBloomFilter->pstFilter, 1, pstBloomFilter->dwFilterSize, pFile);
	if ((uint32_t)iRet != pstBloomFilter->dwFilterSize)
	{
		perror("fwrite(data)");
		return -31;
	}

	fclose(pFile);
	return 0;
}

inline int LoadBloomFilterFromFile(BaseBloomFilter* pstBloomFilter, char *szFileName)
{
	if ((pstBloomFilter == NULL) || (szFileName == NULL))
		return -1;

	int iRet;
	FILE *pFile;
	static BloomFileHead stFileHeader = { 0 };

	if (pstBloomFilter->pstFilter != NULL)
		free(pstBloomFilter->pstFilter);
	if (pstBloomFilter->pdwHashPos != NULL)
		free(pstBloomFilter->pdwHashPos);

	//
	pFile = fopen(szFileName, "rb");
	if (pFile == NULL)
	{
		perror("fopen");
		return -11;
	}

	// 读取并检查文件头
	iRet = fread((void*)&stFileHeader, sizeof(stFileHeader), 1, pFile);
	if (iRet != 1)
	{
		perror("fread(head)");
		return -21;
	}

	if ((stFileHeader.dwMagicCode != __MGAIC_CODE__)
		|| (stFileHeader.dwFilterBits != stFileHeader.dwFilterSize*BYTE_BITS))
		return -50;

	// 初始化传入的 BaseBloomFilter 结构
	pstBloomFilter->dwMaxItems = stFileHeader.dwMaxItems;
	pstBloomFilter->dProbFalse = stFileHeader.dProbFalse;
	pstBloomFilter->dwFilterBits = stFileHeader.dwFilterBits;
	pstBloomFilter->dwHashFuncs = stFileHeader.dwHashFuncs;
	pstBloomFilter->dwSeed = stFileHeader.dwSeed;
	pstBloomFilter->dwCount = stFileHeader.dwCount;
	pstBloomFilter->dwFilterSize = stFileHeader.dwFilterSize;

	pstBloomFilter->pstFilter = (unsigned char *)malloc(pstBloomFilter->dwFilterSize);
	if (NULL == pstBloomFilter->pstFilter)
		return -100;
	pstBloomFilter->pdwHashPos = (uint32_t*)malloc(pstBloomFilter->dwHashFuncs * sizeof(uint32_t));
	if (NULL == pstBloomFilter->pdwHashPos)
		return -200;


	// 将后面的Data部分读入 pstFilter
	iRet = fread((void*)(pstBloomFilter->pstFilter), 1, pstBloomFilter->dwFilterSize, pFile);
	if ((uint32_t)iRet != pstBloomFilter->dwFilterSize)
	{
		perror("fread(data)");
		return -31;
	}
	pstBloomFilter->cInitFlag = 1;

	printf(">>> Load BloomFilter(n=%u, p=%f, m=%u, k=%d), malloc() size=%.2fMB\n",
		pstBloomFilter->dwMaxItems, pstBloomFilter->dProbFalse, pstBloomFilter->dwFilterBits,
		pstBloomFilter->dwHashFuncs, (double)pstBloomFilter->dwFilterSize / 1024 / 1024);

	fclose(pFile);
	return 0;
}