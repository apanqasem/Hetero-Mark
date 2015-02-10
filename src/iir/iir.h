#ifndef IIR_H
#define IIR_H

#include <clUtil.h>

using namespace clHelper;

class IIR
{
	clRuntime *runtime;
	clFile    *file;

	cl_platform_id   platform;
	cl_device_id     device;
	cl_context       context;
	cl_command_queue cmdQueue;

	cl_program       program;
	cl_kernel        kernel;

public:
	IIR();
	~IIR();

	void InitKernel();
	void InitBuffer();

	void FreeKernel();
	void FreeBuffer();
	
};

#endif