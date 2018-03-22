//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLProgramFactory.h"
#include "GLSL/BsGLSLGpuProgram.h"

namespace bs { namespace ct
{
	SPtr<GpuProgram> GLSLProgramFactory::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		GLSLGpuProgram* prog = new (bs_alloc<GLSLGpuProgram>()) GLSLGpuProgram(desc, deviceMask);

		SPtr<GLSLGpuProgram> gpuProg = bs_shared_ptr<GLSLGpuProgram>(prog);
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}

	SPtr<GpuProgram> GLSLProgramFactory::create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		GPU_PROGRAM_DESC desc;
		desc.type = type;

		GLSLGpuProgram* prog = new (bs_alloc<GLSLGpuProgram>()) GLSLGpuProgram(desc, deviceMask);

		SPtr<GLSLGpuProgram> gpuProg = bs_shared_ptr<GLSLGpuProgram>(prog);
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}

	SPtr<GpuProgramBytecode> GLSLProgramFactory::compileBytecode(const GPU_PROGRAM_DESC& desc)
	{
		// Note: No bytecode format for GLSL, so we just write the original source. Note that a SPIR-V extensions for GLSL
		// exists, and that could be used eventually (although support might be limited, esp. on macOS and mobiles).
		SPtr<GpuProgramBytecode> bytecode = bs_shared_ptr_new<GpuProgramBytecode>();
		bytecode->compilerId = OPENGL_COMPILER_ID;

		bytecode->instructions.size = 0;
		bytecode->instructions.size += rttiGetElemSize(desc.type);
		bytecode->instructions.size += rttiGetElemSize(desc.entryPoint);
		bytecode->instructions.size += rttiGetElemSize(desc.language);
		bytecode->instructions.size += rttiGetElemSize(desc.source);
		bytecode->instructions.size += rttiGetElemSize(desc.requiresAdjacency);

		bytecode->instructions.data = (UINT8*)bs_alloc(bytecode->instructions.size);

		char* memory = (char*)bytecode->instructions.data;
		memory = rttiWriteElem(desc.type, memory);
		memory = rttiWriteElem(desc.entryPoint, memory);
		memory = rttiWriteElem(desc.language, memory);
		memory = rttiWriteElem(desc.source, memory);
		memory = rttiWriteElem(desc.requiresAdjacency, memory);

		return bytecode;
	}
}}