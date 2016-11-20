//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLSLProgramFactory.h"
#include "BsGLSLGpuProgram.h"

namespace bs 
{
    const String GLSLProgramFactory::LANGUAGE_NAME = "glsl";

    const String& GLSLProgramFactory::getLanguage() const
    {
        return LANGUAGE_NAME;
    }

	SPtr<GpuProgramCore> GLSLProgramFactory::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
    {
		GLSLGpuProgramCore* prog = new (bs_alloc<GLSLGpuProgramCore>()) GLSLGpuProgramCore(desc, deviceMask);

		SPtr<GLSLGpuProgramCore> gpuProg = bs_shared_ptr<GLSLGpuProgramCore>(prog);
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
    }

	SPtr<GpuProgramCore> GLSLProgramFactory::create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		GPU_PROGRAM_DESC desc;
		desc.type = type;

		GLSLGpuProgramCore* prog = new (bs_alloc<GLSLGpuProgramCore>()) GLSLGpuProgramCore(desc, deviceMask);

		SPtr<GLSLGpuProgramCore> gpuProg = bs_shared_ptr<GLSLGpuProgramCore>(prog);
		gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}
}
