//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11HLSLProgramFactory.h"
#include "BsD3D11GpuProgram.h"

namespace bs
{
    const String D3D11HLSLProgramFactory::LANGUAGE_NAME = "hlsl";

    D3D11HLSLProgramFactory::D3D11HLSLProgramFactory()
    {
    }

    D3D11HLSLProgramFactory::~D3D11HLSLProgramFactory()
    {
    }

    const String& D3D11HLSLProgramFactory::getLanguage(void) const
    {
        return LANGUAGE_NAME;
    }

	SPtr<GpuProgramCore> D3D11HLSLProgramFactory::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
    {
		SPtr<GpuProgramCore> gpuProg;

		switch (desc.type)
		{
		case GPT_VERTEX_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuVertexProgramCore>(new (bs_alloc<D3D11GpuVertexProgramCore>())
				D3D11GpuVertexProgramCore(desc, deviceMask));
			break;
		case GPT_FRAGMENT_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuFragmentProgramCore>(new (bs_alloc<D3D11GpuFragmentProgramCore>())
				D3D11GpuFragmentProgramCore(desc, deviceMask));
			break;
		case GPT_HULL_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuHullProgramCore>(new (bs_alloc<D3D11GpuHullProgramCore>())
				D3D11GpuHullProgramCore(desc, deviceMask));
			break;
		case GPT_DOMAIN_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuDomainProgramCore>(new (bs_alloc<D3D11GpuDomainProgramCore>())
				D3D11GpuDomainProgramCore(desc, deviceMask));
			break;
		case GPT_GEOMETRY_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuGeometryProgramCore>(new (bs_alloc<D3D11GpuGeometryProgramCore>())
				D3D11GpuGeometryProgramCore(desc, deviceMask));
			break;
		case GPT_COMPUTE_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuComputeProgramCore>(new (bs_alloc<D3D11GpuComputeProgramCore>())
				D3D11GpuComputeProgramCore(desc, deviceMask));
			break;
		}

		if (gpuProg != nullptr)
			gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
    }

	SPtr<GpuProgramCore> D3D11HLSLProgramFactory::create(GpuProgramType type, GpuDeviceFlags deviceMask)
	{
		SPtr<GpuProgramCore> gpuProg;

		GPU_PROGRAM_DESC desc;
		desc.type = type;

		switch (type)
		{
		case GPT_VERTEX_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuVertexProgramCore>(new (bs_alloc<D3D11GpuVertexProgramCore>())
				D3D11GpuVertexProgramCore(desc, deviceMask));
			break;
		case GPT_FRAGMENT_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuFragmentProgramCore>(new (bs_alloc<D3D11GpuFragmentProgramCore>())
				D3D11GpuFragmentProgramCore(desc, deviceMask));
			break;
		case GPT_HULL_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuHullProgramCore>(new (bs_alloc<D3D11GpuHullProgramCore>())
				D3D11GpuHullProgramCore(desc, deviceMask));
			break;
		case GPT_DOMAIN_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuDomainProgramCore>(new (bs_alloc<D3D11GpuDomainProgramCore>())
				D3D11GpuDomainProgramCore(desc, deviceMask));
			break;
		case GPT_GEOMETRY_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuGeometryProgramCore>(new (bs_alloc<D3D11GpuGeometryProgramCore>())
				D3D11GpuGeometryProgramCore(desc, deviceMask));
			break;
		case GPT_COMPUTE_PROGRAM:
			gpuProg = bs_shared_ptr<D3D11GpuComputeProgramCore>(new (bs_alloc<D3D11GpuComputeProgramCore>())
				D3D11GpuComputeProgramCore(desc, deviceMask));
			break;
		}

		if (gpuProg != nullptr)
			gpuProg->_setThisPtr(gpuProg);

		return gpuProg;
	}
}