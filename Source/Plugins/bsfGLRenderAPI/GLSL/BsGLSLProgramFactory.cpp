//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLProgramFactory.h"
#include "GLSL/BsGLSLGpuProgram.h"

using namespace bs;
using namespace bs::ct;

SPtr<ct::GpuProgram> GLSLProgramFactory::Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask)
{
	GLSLGpuProgram* prog = new(B3DAllocate<GLSLGpuProgram>()) GLSLGpuProgram(desc, deviceMask);

	SPtr<GLSLGpuProgram> gpuProg = B3DMakeSharedFromExisting<GLSLGpuProgram>(prog);
	gpuProg->SetThisPtrInternal(gpuProg);

	return gpuProg;
}

SPtr<ct::GpuProgram> GLSLProgramFactory::Create(GpuProgramType type, GpuDeviceFlags deviceMask)
{
	GpuProgramCreateInformation desc;
	desc.Type = type;

	GLSLGpuProgram* prog = new(B3DAllocate<GLSLGpuProgram>()) GLSLGpuProgram(desc, deviceMask);

	SPtr<GLSLGpuProgram> gpuProg = B3DMakeSharedFromExisting<GLSLGpuProgram>(prog);
	gpuProg->SetThisPtrInternal(gpuProg);

	return gpuProg;
}

SPtr<GpuProgramBytecode> GLSLProgramFactory::CompileBytecode(const GpuProgramCreateInformation& desc)
{
	// Note: No bytecode format for GLSL
	SPtr<GpuProgramBytecode> bytecode = B3DMakeShared<GpuProgramBytecode>();
	bytecode->CompilerId = kOpenglCompilerId;

	return bytecode;
}
