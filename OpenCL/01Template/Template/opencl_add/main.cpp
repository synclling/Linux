#include <iostream>
#include <fstream>
#include <sstream>
#include <CL/cl.h>

const int ARRAY_SIZE = 1000000;

#ifdef _WIN32
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

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


//一、 选择OpenCL平台并创建一个上下文
cl_context CreateContext1()
{
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;

	//选择可用的平台中的第一个
	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if (errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}

	//创建一个OpenCL上下文环境
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)firstPlatformId,
		0
	};
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
		NULL, NULL, &errNum);

	return context;
}

//一、 选择OpenCL平台并创建一个上下文
cl_context CreateContext()
{
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;

	clGetPlatformIDs(0, NULL, &numPlatforms);
	cl_platform_id *platforms = new cl_platform_id[numPlatforms];
// 	clGetPlatformIDs(numPlatforms, platforms, NULL);


	//选择可用的平台中
	errNum = clGetPlatformIDs(numPlatforms, platforms, NULL);;
	if (errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}

	//创建一个OpenCL上下文环境
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platforms[0],
		0
	};
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
		NULL, NULL, &errNum);

	return context;
}


//二、 创建设备并创建命令队列
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
	cl_int errNum;
	cl_device_id *devices;
	cl_command_queue commandQueue = NULL;
	size_t deviceBufferSize = -1;

	// 获取设备缓冲区大小
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
	std::cout << "clGetContextInfo deviceBufferSize = " << deviceBufferSize << "\n";
	if (deviceBufferSize <= 0)
	{
		std::cerr << "No devices available.";
		return NULL;
	}

	// 为设备分配缓存空间
	devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
	std::cout << "clGetContextInfo devices = " << deviceBufferSize / sizeof(cl_device_id) << "\n";
	//选取可用设备中的第一个
	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

	*device = devices[0];
	delete[] devices;
	return commandQueue;
}


// 三、创建和构建程序对象
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
	cl_int errNum;
	cl_program program;

	std::ifstream kernelFile(fileName, std::ios::in);
	if (!kernelFile.is_open())
	{
		std::cerr << "Failed to open file for reading: " << fileName << std::endl;
		return NULL;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	//创建程序对象
	program = clCreateProgramWithSource(context, 1,
		(const char**)&srcStr,
		NULL, NULL);
	//编译程序对象
	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != 0)
	{
		std::cerr << "clBuildProgram failed: " << errNum << std::endl;
		return NULL;
	}

	return program;
}

//创建和构建程序对象
bool CreateMemObjects(cl_context context, cl_mem memObjects[3],
	float *a, float *b)
{
	memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * ARRAY_SIZE, a, NULL);
	memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * ARRAY_SIZE, b, NULL);
	memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
		sizeof(float) * ARRAY_SIZE, NULL, NULL);
	return true;
}


// 释放OpenCL资源
void Cleanup(cl_context context, cl_command_queue commandQueue,
	cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
	for (int i = 0; i < 3; i++)
	{
		if (memObjects[i] != 0)
			clReleaseMemObject(memObjects[i]);
	}
	if (commandQueue != 0)
		clReleaseCommandQueue(commandQueue);

	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

	if (context != 0)
		clReleaseContext(context);
}
float do_for_test(int count)
{
	long long sum = 0;
	for (int i = 0; i < count; i++)
	{
		sum += i;
	}
	return (float)sum;
}
int main(int argc, char** argv)
{
	cl_context context = 0;
	cl_command_queue commandQueue = 0;
	cl_program program = 0;
	cl_device_id device = 0;
	cl_kernel kernel = 0;
	cl_mem memObjects[3] = { 0, 0, 0 };
	cl_int errNum;

	// 一、选择OpenCL平台并创建一个上下文
	context = CreateContext();		// GPU平台 context device queue buffer kernel 

	// 二、 创建设备并创建命令队列
	commandQueue = CreateCommandQueue(context, &device);

	// 三 创建和构建程序对象
	program = CreateProgram(context, device, "HelloWorld.cl");

	// 四、 创建OpenCL内核并分配内存空间
	kernel = clCreateKernel(program, "hello_kernel", NULL);

	//创建要处理的数据
	float *result = new float[ARRAY_SIZE];
	float *a = new float[ARRAY_SIZE];
	float *b = new float[ARRAY_SIZE];

	float *cpu_result = new float[ARRAY_SIZE];
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		cpu_result[i] = 0;
		result[i] = 0;
		a[i] = (float)i;
		b[i] = (float)(ARRAY_SIZE - i);
	}

	uint64_t start_time = GetCurrentTimeMsec();
	//创建内存对象
	if (!CreateMemObjects(context, memObjects, a, b))
	{
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	// 五、 设置内核数据并执行内核
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);

	size_t globalWorkSize[1] = { ARRAY_SIZE };	// 总共项
	size_t localWorkSize[1] = { 1 };
	// 执行内核
	errNum = clEnqueueNDRangeKernel(commandQueue,//一个有效的命令队列，内核将被入队并在command_queue所关联的设备上执行
		kernel,//一个有效的内核对象。kernel 所关联的OpenCL上下文必须和command_queue所关联的保持一致
		1, //是维数，用来指定全局工作项和工作组中的工作项。work_dim必须大于0，且小于等3. 
		NULL,//在当前必须是NULL,将来修订OpenCL时，此参数可能用来指定一组work_dim无符号值作为偏移量，用来计算工作项的全局ID,而不必非得知道从偏移位置(0,0,…,0)起始的全局ID. 
		globalWorkSize,//向一组work_dim无符号值，描述work_dim维度中用来执行内核函数的全局工作项的数目。全局工作项的数目这样计算：global_work_size[0]*…*global_work_size[work_dim - 1]。 
		localWorkSize,//描述组成一个工作组(会执行kernel)的工作项的数目.工作组长工作项的总数可以这样算得local_work_size[0]*…*local_work_size[work_dim - 1]
		0,
		NULL,
		NULL);
	if (errNum != 0) 
	{
		std::cerr << "clEnqueueNDRangeKernel failed: " << errNum << std::endl;
	}
	uint64_t end_time = GetCurrentTimeMsec(); // 只是GPU执行完
	std::cout << "GPU Executed program time1: " << end_time - start_time << std::endl;
	// 六、 读取执行结果并释放OpenCL资源
	errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE,
		0, ARRAY_SIZE * sizeof(float), result,
		0, NULL, NULL);
	end_time = GetCurrentTimeMsec();
	std::cout << "GPU Executed program time2: " << end_time - start_time << std::endl;
	std::cout << std::fixed;//不用科学计数法
	std::cout.precision(0); //保留一位小数
	
	
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
// 		std::cout << result[i] << " ";
	}

	start_time = GetCurrentTimeMsec();
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		cpu_result[i] = a[i] + b[i] ;
		cpu_result[i] += do_for_test(1000);

	}
	end_time = GetCurrentTimeMsec();
	std::cout << "CPU Executed program time: " << end_time - start_time << std::endl;
	std::cout << std::endl;
	std::cout << std::fixed;//不用科学计数法
	std::cout.precision(0); //保留一位小数

	std::cout << "result[0]:		" << result[0] << std::endl;
	std::cout << "result[ARRAY_SIZE-1]: " << result[ARRAY_SIZE - 1] << std::endl;

	std::cout << "cpu_result[0]: " << cpu_result[0] << std::endl;
	std::cout << "cpu_result[1]: " << cpu_result[1] << std::endl;
	std::cout << "Executed program succesfully." << std::endl;
	
	getchar();
	Cleanup(context, commandQueue, program, kernel, memObjects);

	delete[] result;
	delete[] a;
	delete[] b;
	delete[] cpu_result;

	return 0;
}
