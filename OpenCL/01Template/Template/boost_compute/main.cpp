#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <boost/compute.hpp>
#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

namespace compute = boost::compute;

uint64_t GetCurrentTimeMsec()
{
#ifdef _WIN32
	struct timeval tv;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tv.tv_sec = clock;
	tv.tv_usec = wtm.wMilliseconds * 1000;
	return ((uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000);
#endif
}

float generateFloat()
{
	srand((unsigned)time(NULL));//这里以当前时间为种子

	return(float)(rand());//根据需要设置除数
}
#define DATA_SIZE 2000000

int main()
{
	// get the default compute device
// 	compute::device gpu = compute::system::default_device();
	auto devices = compute::system::devices();
	for (auto i = 0; i < devices.size(); i++)
	{
		std::cout << "gpu_name: " << devices[i].name() << std::endl;
	}
	std::string gpu_name = devices[2].name();
	std::cout << "select gpu: " << gpu_name << std::endl;

	compute::device gpu = compute::system::find_device(gpu_name);
	// create a compute context and command queue
	compute::context ctx(gpu);
	compute::command_queue queue(ctx, gpu);// OpenCL 

	uint64_t start_time = 0;
	uint64_t start_time2 = 0;
	uint64_t end_time = 0;
	// generate random numbers on the host
	std::vector<float> host_vector(DATA_SIZE);
	std::generate(host_vector.begin(), host_vector.end(), rand);

	start_time = GetCurrentTimeMsec();
	// create vector on the device
	compute::vector<float> device_vector(DATA_SIZE, ctx);
	
	// copy data to the device
	compute::copy(
		host_vector.begin(), host_vector.end(), device_vector.begin(), queue
	);
	start_time2 = GetCurrentTimeMsec();
	// sort data on the device
	compute::sort(
		device_vector.begin(), device_vector.end(), queue
	);

	// copy data back to the host
	compute::copy(
		device_vector.begin(), device_vector.end(), host_vector.begin(), queue
	);
	end_time = GetCurrentTimeMsec();
	std::cout << "GPU Executed program time1: " << end_time - start_time << std::endl;
	std::cout << "GPU Executed program time2: " << end_time - start_time2 << std::endl;
	for (int i = 1000; i <= 2000; i++)
		printf("%f ", host_vector[i]);  //输出
	printf("\n");

	std::generate(host_vector.begin(), host_vector.end(), rand);
	start_time = GetCurrentTimeMsec();
	sort(host_vector.begin(), host_vector.end());
	end_time = GetCurrentTimeMsec();
	std::cout << "CPU Executed program time: " << end_time - start_time << std::endl;
	for (int i = 1000; i <= 2000; i++)
		printf("%f ", host_vector[i]);  //输出
	printf("\n");
	std::cout << "----Executed program succesfully.--------" << std::endl;
 	
	getchar();
	return 0;
}

/*
sort作为一个内置的排序方法，可以被vector等直接调用。

对于STL中的sort()算法：

当数据量大时，将会采用Quick Sort（快排），分段递归进行排序。
一旦分段后的数据量小于某个阈值，为了避免快排的递归带来过大的额外的开销，sort()算法就自动改为Insertion Sort（插入排序）。
如果递归的层次过深，还会改用Heap Sort（堆排序）。
简单来说，sort并非只是普通的快速排序，除了对普通的快排进行优化，它还结合了插入排序和堆排序。

根据不同的数量级以及不同的情况，能够自动选择合适的排序算法。
*/