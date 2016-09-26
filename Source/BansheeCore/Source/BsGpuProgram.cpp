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
		
	GpuProgramCore::GpuProgramCore(const String& source, const String& entryPoint,
		GpuProgramType gptype, GpuProgramProfile profile, bool isAdjacencyInfoRequired)
		:mNeedsAdjacencyInfo(isAdjacencyInfoRequired), mIsCompiled(false), mProperties(source, entryPoint, gptype, profile)
	{
		mParametersDesc = bs_shared_ptr_new<GpuParamDesc>();
	}

	bool GpuProgramCore::isSupported() const
    {
		if (!isRequiredCapabilitiesSupported())
			return false;

		RenderAPICore* rs = BansheeEngine::RenderAPICore::instancePtr();
		String profile = rs->getCapabilities()->gpuProgProfileToRSSpecificProfile(getProperties().getProfile());

		return rs->getCapabilities()->isShaderProfileSupported(profile);
    }

	bool GpuProgramCore::isRequiredCapabilitiesSupported() const
	{
		return true;
	}

	SPtr<GpuParamsCore> GpuProgramCore::createParameters()
	{
		return GpuParamsCore::create(mParametersDesc);
	}

	SPtr<GpuProgramCore> GpuProgramCore::create(const String& source, const String& entryPoint, const String& language, GpuProgramType gptype,
		GpuProgramProfile profile, bool requiresAdjacency)
	{
		return GpuProgramCoreManager::instance().create(source, entryPoint, language, gptype, profile, requiresAdjacency);
	}

	GpuProgram::GpuProgram(const String& source, const String& entryPoint, const String& language,
		GpuProgramType gptype, GpuProgramProfile profile, bool isAdjacencyInfoRequired) 
		: mNeedsAdjacencyInfo(isAdjacencyInfoRequired), mLanguage(language)
		, mProperties(source, entryPoint, gptype, profile)
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

	SPtr<GpuParams> GpuProgram::createParameters()
	{
		return GpuParams::create(getCore()->getParamDesc());
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
		return GpuProgramCoreManager::instance().createInternal(mProperties.getSource(), mProperties.getEntryPoint(),
			mLanguage, mProperties.getType(), mProperties.getProfile(), mNeedsAdjacencyInfo);
	}

	SPtr<GpuProgram> GpuProgram::create(const String& source, const String& entryPoint, const String& language, GpuProgramType gptype,
		GpuProgramProfile profile, bool requiresAdjacency)
	{
		return GpuProgramManager::instance().create(source, entryPoint, language, gptype, profile, requiresAdjacency);
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