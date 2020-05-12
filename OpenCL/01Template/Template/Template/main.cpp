#include <iostream>
#include <string>
#include "CL\opencl.h"
using namespace std;
string getPlatformName(const cl_platform_id pid) {
	size_t param_value_size;
	clGetPlatformInfo(pid, CL_PLATFORM_NAME, 0, NULL, &param_value_size);
	char *param_value = new char[param_value_size];
	clGetPlatformInfo(pid, CL_PLATFORM_NAME, param_value_size, param_value, NULL);
	return param_value;
}
int main() {
	cl_uint num_platforms;
	clGetPlatformIDs(0, NULL, &num_platforms);
	cl_platform_id *platforms = new cl_platform_id[num_platforms];
	clGetPlatformIDs(num_platforms, platforms, NULL);
	for (cl_uint i = 0; i < num_platforms; i++) {
		string platname = getPlatformName(platforms[i]);
		cout << "<" << i << "> " << "Platform name is :" << platname << endl;
	}
	system("pause");
	return 0;
}