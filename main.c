#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> // strstr
#include <time.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define LOG 1

int main(int argc, char* argv[])
{
	// Loads add_vectors.cl
	FILE* fp;
	fp = fopen("mcx_core.cl", "r");
	if (!fp) {
		fprintf(stderr, "Error loading kernel.\n");
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	size_t kernel_sz = ftell(fp);
	rewind(fp);

	char* kernel_str = (char*)malloc(kernel_sz);
	fread(kernel_str, 1, kernel_sz, fp);
	fclose(fp);

	// Query platforms and devices

	cl_platform_id 	platform;
	cl_device_id 	device;
	cl_uint 		num_devices, num_platforms;

	//cl_int err = clGetPlatformIDs(1, &platform, &num_platforms);
	cl_int err = clGetPlatformIDs(1, NULL, &num_platforms);
	printf("platforms: %d\n", num_platforms);

	cl_platform_id* platforms =(cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
	clGetPlatformIDs(num_platforms, platforms, NULL);

	// notes: select 2nd platform in my case
	int pid = 0;

	// platform name
	char *pltName=(char*) malloc(sizeof(char) * 256);

	for(unsigned int i=0; i<num_platforms; i++)
	{
		platform = platforms[i];

		clGetPlatformInfo(platform, CL_PLATFORM_NAME, 256, (void*)pltName, NULL);

		printf("platform : %d (%s)\n", pid, pltName);

		if(strstr(pltName,"NVIDIA") != NULL) {
			printf("select current platform\n");
			pid = i;
			break;
		}
	}

	platform = platforms[pid];
	printf("Test on platform : %d (%s)\n", pid, pltName);

	// get the gpu devices
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, NULL, &num_devices);
	printf("gpu devices : %d\n", num_devices);

	cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);

	// GPU Info
	int gid = 0;
	//int gid = 1;
	char *gpuName=(char*) malloc(sizeof(char) * 256);
	clGetDeviceInfo(devices[gid], CL_DEVICE_NAME, 256, (void*)gpuName,NULL);
	printf("select gpu : %d  (%s)\n", gid, gpuName);

	device = devices[gid];

	// Create OpenCL context
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

	// Create OpenCL command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device, 0, &err);

	// Create OpenCL program for kernel 
	cl_program program = clCreateProgramWithSource(context, 1,
			(const char **)&kernel_str, (const size_t *)&kernel_sz, &err);

	// Build OpenCL program
	char flags[10240];
	time_t t;
	srand((unsigned) time(&t));
	int someRandNum = rand() % 100 + 200; // 200 to 299
	sprintf(flags, "-cl-nv-maxrregcount=%d -cl-nv-verbose", someRandNum); // randomn number is added to avoid empty log 
	err = clBuildProgram(program, 1,  &device, flags, NULL, NULL);

#if LOG 
	{
		char *program_log;
		size_t log_size;
		err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		program_log = (char*)malloc(log_size+1);
		err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);

		program_log[log_size] = '\0';

		fprintf(stderr, "%s\n", program_log);
	}
#endif

	// Create OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "mcx_main_loop", &err);

	// Query binary (PTX file) size
	size_t bin_sz;
	err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &bin_sz, NULL);

	// Read binary (PTX file) to memory buffer
	unsigned char *bin = (unsigned char *)malloc(bin_sz);
	err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char *), &bin, NULL);

	// Save PTX to add_vectors_ocl.ptx
	fp = fopen("mcx_core.ptx", "wb");
	fwrite(bin, sizeof(char), bin_sz, fp);
	fclose(fp);
	free(bin);

	free(kernel_str);
	free(platforms);
	free(pltName);
	free(devices);
	free(gpuName);

	clFlush(command_queue);
	clFinish(command_queue);

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	return 0;
}
