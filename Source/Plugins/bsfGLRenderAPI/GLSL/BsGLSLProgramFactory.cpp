//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLProgramFactory.h"
#include "GLSL/BsGLSLGpuProgram.h"

namespace bs
{
	namespace ct
	{
		SPtr<GpuProgram> GLSLProgramFactory::Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		{
			GLSLGpuProgram* prog = new(bs_alloc<GLSLGpuProgram>()) GLSLGpuProgram(desc, deviceMask);

			SPtr<GLSLGpuProgram> gpuProg = bs_shared_ptr<GLSLGpuProgram>(prog);
			gpuProg->SetThisPtrInternal(gpuProg);

			return gpuProg;
		}

		SPtr<GpuProgram> GLSLProgramFactory::Create(GpuProgramType type, GpuDeviceFlags deviceMask)
		{
			GPU_PROGRAM_DESC desc;
			desc.Type = type;

			GLSLGpuProgram* prog = new(bs_alloc<GLSLGpuProgram>()) GLSLGpuProgram(desc, deviceMask);

			SPtr<GLSLGpuProgram> gpuProg = bs_shared_ptr<GLSLGpuProgram>(prog);
			gpuProg->SetThisPtrInternal(gpuProg);

			return gpuProg;
		}

		SPtr<GpuProgramBytecode> GLSLProgramFactory::CompileBytecode(const GPU_PROGRAM_DESC& desc)
		{
			// Note: No bytecode format for GLSL
			SPtr<GpuProgramBytecode> bytecode = bs_shared_ptr_new<GpuProgramBytecode>();
			bytecode->CompilerId = OPENGL_COMPILER_ID;

			return bytecode;
		}
	} // namespace ct
} // namespace bs
