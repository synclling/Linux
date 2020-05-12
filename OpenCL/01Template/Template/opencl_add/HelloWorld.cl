__kernel void hello_kernel(__global const float *a,
	__global const float *b,
	__global float *result)
{
	
	long long sum = 0;
	for (int i = 0; i < 1000; i++)
	{
		sum += i;
	}
	int gid = get_global_id(0);
	result[gid] = a[gid] + b[gid] + (float)sum;
	/*
	int gid = get_global_id(0);
	result[gid] = a[gid] + b[gid];
	*/
}