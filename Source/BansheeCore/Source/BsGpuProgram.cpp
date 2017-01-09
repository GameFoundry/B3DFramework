//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuProgram.h"
#include "BsRenderAPICapabilities.h"
#include "BsRenderAPI.h"
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuProgramManager.h"
#include "BsGpuProgramRTTI.h"

namespace bs
{
	GpuProgramProperties::GpuProgramProperties(const String& source, const String& entryPoint, GpuProgramType gptype)
		:mType(gptype), mEntryPoint(entryPoint), mSource(source)
	{ }
		
	GpuProgram::GpuProgram(const GPU_PROGRAM_DESC& desc)
		: mNeedsAdjacencyInfo(desc.requiresAdjacency), mLanguage(desc.language)
		, mProperties(desc.source, desc.entryPoint, desc.type)
    {

    }

	bool GpuProgram::isCompiled() const
	{
		return getCore()->isCompiled();
	}

	String GpuProgram::getCompileErrorMessage() const
	{
		return getCore()->getCompileErrorMessage();
	}

	SPtr<GpuParamDesc> GpuProgram::getParamDesc() const
	{
		return getCore()->getParamDesc();
	}

	SPtr<ct::GpuProgramCore> GpuProgram::getCore() const
	{
		return std::static_pointer_cast<ct::GpuProgramCore>(mCoreSpecific);
	}

	SPtr<ct::CoreObjectCore> GpuProgram::createCore() const
	{
		GPU_PROGRAM_DESC desc;
		desc.source = mProperties.getSource();
		desc.entryPoint = mProperties.getEntryPoint();
		desc.language = mLanguage;
		desc.type = mProperties.getType();
		desc.requiresAdjacency = mNeedsAdjacencyInfo;

		return ct::GpuProgramManager::instance().createInternal(desc);
	}

	SPtr<GpuProgram> GpuProgram::create(const GPU_PROGRAM_DESC& desc)
	{
		return GpuProgramManager::instance().create(desc);
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* GpuProgram::getRTTIStatic()
	{
		return GpuProgramRTTI::instance();
	}

	RTTITypeBase* GpuProgram::getRTTI() const
	{
		return GpuProgram::getRTTIStatic();
	}

	namespace ct
	{
	GpuProgramCore::GpuProgramCore(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		:mNeedsAdjacencyInfo(desc.requiresAdjacency), mIsCompiled(false), mProperties(desc.source, desc.entryPoint, 
			desc.type)
	{
		mParametersDesc = bs_shared_ptr_new<GpuParamDesc>();
	}

	bool GpuProgramCore::isSupported() const
    {
		if (!isRequiredCapabilitiesSupported())
			return false;

		return true;
    }

	bool GpuProgramCore::isRequiredCapabilitiesSupported() const
	{
		return true;
	}

	SPtr<GpuProgramCore> GpuProgramCore::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return GpuProgramManager::instance().create(desc, deviceMask);
	}
	}
}