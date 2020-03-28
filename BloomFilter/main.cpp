#include "BloomFilter.h"

int main()
{
	printf(" test bloomfilter\n");

	// 1. 定义BaseBloomFilter
	static BaseBloomFilter stBloomFilter = { 0 };

	// 2. 初始化stBloomFilter
	InitBloomFilter(&stBloomFilter, 0, 100000, 0.00001);

	// 3. 向BloomFilter中新增数值
	char url[128] = { 0 };
	for (int i = 0; i < 10000; i++)
	{
		sprintf(url, "https://0voice.com/%d.html", i);
		if (0 == BloomFilter_Add(&stBloomFilter, (const void*)url, strlen(url)))
		{
			// printf("add %s success", url);
		}
		else
		{
			printf("add %s failed", url);
		}
		memset(url, 0, sizeof(url));
	}

	// 4. check url exist or not
	const char* str = "https://0voice.com/0.html";
	if (0 == BloomFilter_Check(&stBloomFilter, (const void*)str, strlen(str)))
	{
		printf("https://0voice.com/0.html exist\n");
	}

	const char* str2 = "https://0voice.com/10001.html";
	if (0 != BloomFilter_Check(&stBloomFilter, (const void*)str2, strlen(str2)))
	{
		printf("https://0voice.com/10001.html not exist\n");
	}

	// 5. free bloomfilter
	FreeBloomFilter(&stBloomFilter);

	return 0;
}