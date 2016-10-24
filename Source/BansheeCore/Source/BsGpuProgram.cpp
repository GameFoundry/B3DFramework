//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuProgram.h"
#include "BsRenderAPICapabilities.h"
#include "BsRenderAPI.h"
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuProgramManager.h"
#include "BsGpuProgramRTTI.h"

namespace BansheeEngine
{
	GpuProgramProperties::GpuProgramProperties(const String& source, const String& entryPoint,
		GpuProgramType gptype, GpuProgramProfile profile)
		:mType(gptype), mEntryPoint(entryPoint), mProfile(profile), mSource(source)
	{ }
		
	GpuProgramCore::GpuProgramCore(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		:mNeedsAdjacencyInfo(desc.requiresAdjacency), mIsCompiled(false), mProperties(desc.source, desc.entryPoint, 
			desc.type, desc.profile)
	{
		mParametersDesc = bs_shared_ptr_new<GpuParamDesc>();
	}

	bool GpuProgramCore::isSupported() const
    {
		if (!isRequiredCapabilitiesSupported())
			return false;

		RenderAPICore* rapi = RenderAPICore::instancePtr();
		String profile = rapi->getCapabilities().gpuProgProfileToRSSpecificProfile(getProperties().getProfile());

		return rapi->getCapabilities().isShaderProfileSupported(profile);
    }

	bool GpuProgramCore::isRequiredCapabilitiesSupported() const
	{
		return true;
	}

	SPtr<GpuProgramCore> GpuProgramCore::create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
	{
		return GpuProgramCoreManager::instance().create(desc, deviceMask);
	}

	GpuProgram::GpuProgram(const GPU_PROGRAM_DESC& desc)
		: mNeedsAdjacencyInfo(desc.requiresAdjacency), mLanguage(desc.language)
		, mProperties(desc.source, desc.entryPoint, desc.type, desc.profile)
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

	SPtr<GpuProgramCore> GpuProgram::getCore() const
	{
		return std::static_pointer_cast<GpuProgramCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GpuProgram::createCore() const
	{
		GPU_PROGRAM_DESC desc;
		desc.source = mProperties.getSource();
		desc.entryPoint = mProperties.getEntryPoint();
		desc.language = mLanguage;
		desc.type = mProperties.getType();
		desc.profile = mProperties.getProfile();
		desc.requiresAdjacency = mNeedsAdjacencyInfo;

		return GpuProgramCoreManager::instance().createInternal(desc);
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
}