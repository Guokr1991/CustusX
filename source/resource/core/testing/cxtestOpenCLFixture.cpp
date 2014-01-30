#include <cxtestOpenCLFixture.h>

#include "sscMessageManager.h"
#include "cxtestUtilities.h"

namespace cxtest
{

OpenCLFixture::OpenCLFixture()
{
	cx::messageManager()->initialize();
}

OpenCLFixture::~OpenCLFixture()
{
	this->waitForOpenCLToShutDown();
	cx::messageManager()->shutdown();
}

bool OpenCLFixture::runTestKernel(cl_device_type type)
{
	cx::OpenCL::ocl* opencl = cx::OpenCL::init(type);

	const char* source = "__kernel void test_kernel(void){int i = 0; i+=i;}";
	bool success = true;
	try
	{
		cl::Program program = cx::OpenCL::createProgram(opencl->context, source, strlen(source));
		cx::OpenCL::build(program, "");
		cl::Kernel kernel = cx::OpenCL::createKernel(program, "test_kernel");
		size_t numberOfWorkingItems;
		kernel.getWorkGroupInfo(opencl->device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &numberOfWorkingItems);
		int numberOfWorkGroups = 2;
		size_t totalNumberOfItems = numberOfWorkGroups*numberOfWorkingItems; //is the mental picture correct???
		opencl->cmd_queue.enqueueNDRangeKernel(kernel, 0, totalNumberOfItems, numberOfWorkingItems, NULL, NULL);
		opencl->cmd_queue.finish();
	}
	catch(cl::Error error)
	{
		cx::messageManager()->sendError("Could not execute the kernel. Reason: "+QString(error.what()));
		success = false;
	}

	cx::OpenCL::release(opencl);

	return success;
}

void OpenCLFixture::waitForOpenCLToShutDown()
{
	//TODO make sure OpenCL threads are finished before shutting down the messageManager
	Utilities::sleep_sec(1); //TODO make better solution...
}


} /* namespace cx */
