//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsRenderAPICapabilities.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsGpuParams.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsGpuProgramManager.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

namespace bs
{
	GpuProgramBytecode::~GpuProgramBytecode()
	{
		if(instructions.data)
			bs_free(instructions.data);
	}

	RTTITypeBase* GpuProgramBytecode::getRTTIStatic()
	{
		return GpuProgramBytecodeRTTI::Instance();
	}

	RTTITypeBase* GpuProgramBytecode::getRTTI() const
	{
		return GpuProgramBytecode::GetRTTIStatic();
	}

	GpuProgram::GpuProgram(const GPU_PROGRAM_DESC& desc)
		: MNeedsAdjacencyInfo(desc.requiresAdjacency), mLanguage(desc.language), mType(desc.type)
		, MEntryPoint(desc.entryPoint), mSource(desc.source)
	{

	}

	bool GpuProgram::IsCompiled() const
	{
		return GetCore()->isCompiled();
	}

	String GpuProgram::GetCompileErrorMessage() const
	{
		return GetCore()->getCompileErrorMessage();
	}

	SPtr<GpuParamDesc> GpuProgram::GetParamDesc() const
	{
		return GetCore()->getParamDesc();
	}

	SPtr<ct::GpuProgram> GpuProgram::GetCore() const
	{
		return std::static_pointer_cast<ct::GpuProgram>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GpuProgram::CreateCore() const
	{
		GPU_PROGRAM_DESC desc;
		desc.source = mSource;
		desc.entryPoint = mEntryPoint;
		desc.language = mLanguage;
		desc.type = mType;
		desc.requiresAdjacency = mNeedsAdjacencyInfo;
		desc.bytecode = mBytecode;

		return ct::GpuProgramManager::instance().createInternal(desc);
	}

	SPtr<GpuProgram> GpuProgram::Create(const GPU_PROGRAM_DESC& desc)
	{
		return GpuProgramManager::Instance().create(desc);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* GpuProgram::getRTTIStatic()
	{
		return GpuProgramRTTI::Instance();
	}

	RTTITypeBase* GpuProgram::getRTTI() const
	{
		return GpuProgram::GetRTTIStatic();
	}

	namespace ct
	{
	GpuProgram::GpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		: MNeedsAdjacencyInfo(desc.requiresAdjacency), mType(desc.type), mEntryPoint(desc.entryPoint), mSource(desc.source)
		, MBytecode(desc.bytecode)
	{
		mParametersDesc = bs_shared_ptr_new<GpuParamDesc>();
	}

	GpuProgram::~GpuProgram()
	{ }

	bool GpuProgram::IsSupported() const
	{
		return true;
	}

	SPtr<GpuProgram> GpuProgram::Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return GpuProgramManager::Instance().create(desc, deviceMask);
	}

	SPtr<GpuProgramBytecode> GpuProgram::CompileBytecode(const GPU_PROGRAM_DESC& desc)
	{
		return GpuProgramManager::Instance().compileBytecode(desc);
	}
	}
}
