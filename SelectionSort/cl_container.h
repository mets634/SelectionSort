#pragma once

#include <CL\cl.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "_timer.h"

using namespace std;

#define TYPE int

const bool will_log = true;
const string func_name = "select_sort";


/*A class to run parallel sorting with.
The class uses OpenCL software while interfacing
with the GPU. Sorting only works on arrays with 
a binary number of inputs (2^N) and no repeating numbers.*/
class cl_container {
private:
	bool has_init;

	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue command_queue;
	cl_mem input;
	cl_mem output;
	cl_program program;
	cl_kernel kernel;

	string code;

	cl_int ret;
	_timer timer;

	/*Container specific functions*/

	void set_platform() {
		cl_uint ret_num_platforms;
		ret = clGetPlatformIDs(1, &platform, &ret_num_platforms);

		// log data
		stringstream s;
		s << "Set platform to ID " << platform << ". NUMBER_OF_PLATFORMS = " << ret_num_platforms << ". Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void set_device() {
		cl_uint ret_num_devices;
		ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &ret_num_devices);

		// log data
		stringstream s;
		s << "Set device to ID " << device << ". NUMBER_OF_DEVICES = " << ret_num_devices << ". Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void create_context() {
		context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);

		// log creation
		stringstream s;
		s << "Created context. Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void create_command_queue() {
		command_queue = clCreateCommandQueue(context, device, NULL, &ret);
		
		// log creation
		stringstream s;
		s << "Created command queue. Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void read_cl() {
		// open file
		fstream file("Select.cl");

		// get file size
		int size = file.tellg();
		file.seekg(ios::beg);

		// read CL code
		stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		code = buffer.str();

		// log compilation
		stringstream s;
		s << "Read and compiled CL-code. Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void create_program() {
		// turn string into char buffer
		int count = code.size();
		char *buffer = &code[0];

		// compile code
		program = clCreateProgramWithSource(context, 1, (const char**)&buffer, (const size_t *)&count, &ret);
		ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

		// log creation
		stringstream s;
		s << "Created program. Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void create_kernel() {
		kernel = clCreateKernel(program, func_name.c_str(), &ret);

		// log creation
		stringstream s;
		s << "Created kernel. Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void init() {
		_log("Beginning initialization...");
		set_platform();
		set_device();
		create_context();
		create_command_queue();
		read_cl();
		create_program();
		create_kernel();
		_log("Finished initialization");

		has_init = true;
	}

	/*Operation specific functions*/

	void create_memory(vector<TYPE> &input_data, vector<TYPE> &output_data) {
		const size_t count = input_data.size();
		stringstream s;

		input = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, count * sizeof(TYPE), input_data.data(), &ret);
		s << "Created input memory. count = " << count * sizeof(TYPE) << ". Return code = " << getErrorString(ret);

		output = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, count * sizeof(TYPE), output_data.data(), &ret);
		s << endl << "Created output memory. count = " << count * sizeof(TYPE) << ". Return code = " << getErrorString(ret);

		_log(s.str());
	}

	void get_results(vector<TYPE> &out) {
		const size_t count = out.size();

		// read result into out
		ret = clEnqueueReadBuffer(command_queue, output, CL_TRUE, 0, count * sizeof(TYPE), out.data(), 0, NULL, NULL);

		// log reading
		stringstream s;
		s << "Read from output GPU memory. Return code = " << getErrorString(ret);
		_log(s.str());
	}

	void set_args(const cl_uint count) {
		stringstream s;


		ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
		s << "Set argument1 to input memory. Return code = " << getErrorString(ret);

		ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
		s << endl << "Set argument2 to output memory. Return code = " << getErrorString(ret);

		ret = clSetKernelArg(kernel, 2, sizeof(cl_uint), &count);
		s << endl << "Set argument3 to count. Return code = " << getErrorString(ret);

		_log(s.str());
	}

	void run(const size_t count) {
		stringstream s1, s2;

		s1 << "Running on GPU with " << count << " global-items...";
		_log(s1.str());

		// run sorting
		timer.start();
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &count, NULL, NULL, NULL, NULL);
		clFinish(command_queue);
		timer.stop();

		s2 << endl << "Finished running. Elapsed time = " << timer.get_time() << " seconds. Return code = " << getErrorString(ret);
		_log(s2.str());
	}


	void _log(string msg) {
		if (will_log)
			cout << "LOG: " << msg << endl;
	}

	const char *getErrorString(cl_int error)
	{
		switch (error) {
			// run-time and JIT compiler errors
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case -16: return "CL_LINKER_NOT_AVAILABLE";
		case -17: return "CL_LINK_PROGRAM_FAILURE";
		case -18: return "CL_DEVICE_PARTITION_FAILED";
		case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

			// compile-time errors
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

			// extension errors
		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		default: return "Unknown OpenCL error";
		}
	}

public:
	vector<TYPE> sort(vector<TYPE> in) {
			const size_t count = in.size();

			vector<TYPE> out(in);

			// log sorting execution
			stringstream s1;
			s1 << "Sorting " << count << " numbers. Array data dump:" << endl;
			//for (auto k = in.begin(); k != in.end(); ++k)
				//s1 << *k << " ";
			_log(s1.str());

			create_memory(in, out);
			set_args(count);
			run(count);

			get_results(out);

			// log end of sorting
			stringstream s2;
			s2 << "Finished sorting. Array data dump: " << endl;
			//for (auto k = out.begin(); k != out.end(); ++k)
				//s2 << *k << " ";
			_log(s2.str());

			return out;
		}
	
	cl_container() {
		init();
	}
	~cl_container() {
		ret = clFlush(command_queue);
		ret = clFinish(command_queue);
		ret = clReleaseKernel(kernel);
		ret = clReleaseProgram(program);
		ret = clReleaseMemObject(input);
		ret = clReleaseMemObject(output);
		ret = clReleaseCommandQueue(command_queue);
		ret = clReleaseContext(context);
	}
};