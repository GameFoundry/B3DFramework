//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsGpuProgram.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT GpuProgramRTTI : public RTTIType<GpuProgram, IReflectable, GpuProgramRTTI>
	{
	private:
		GpuProgramType& getType(GpuProgram* obj) { return obj->mProperties.mType; }
		void setType(GpuProgram* obj, GpuProgramType& val) { obj->mProperties.mType = val; }

		bool& getNeedsAjdInfo(GpuProgram* obj) { return obj->mNeedsAdjacencyInfo; }
		void setNeedsAjdInfo(GpuProgram* obj, bool& val) { obj->mNeedsAdjacencyInfo = val; }

		String& getEntryPoint(GpuProgram* obj) { return obj->mProperties.mEntryPoint; }
		void setEntryPoint(GpuProgram* obj, String& val) { obj->mProperties.mEntryPoint = val; }

		GpuProgramProfile& getProfile(GpuProgram* obj) { return obj->mProperties.mProfile; }
		void setProfile(GpuProgram* obj, GpuProgramProfile& val) { obj->mProperties.mProfile = val; }

		String& getSource(GpuProgram* obj) { return obj->mProperties.mSource; }
		void setSource(GpuProgram* obj, String& val) { obj->mProperties.mSource = val; }

		String& getLanguage(GpuProgram* obj) { return obj->mLanguage; }
		void setLanguage(GpuProgram* obj, String& val) { obj->mLanguage = val; }

	public:
		GpuProgramRTTI()
		{
			addPlainField("mType", 2, &GpuProgramRTTI::getType, &GpuProgramRTTI::setType);
			addPlainField("mNeedsAdjacencyInfo", 3, &GpuProgramRTTI::getNeedsAjdInfo, &GpuProgramRTTI::setNeedsAjdInfo);
			addPlainField("mEntryPoint", 4, &GpuProgramRTTI::getEntryPoint, &GpuProgramRTTI::setEntryPoint);
			addPlainField("mProfile", 5, &GpuProgramRTTI::getProfile, &GpuProgramRTTI::setProfile);
			addPlainField("mSource", 6, &GpuProgramRTTI::getSource, &GpuProgramRTTI::setSource);
			addPlainField("mLanguage", 7, &GpuProgramRTTI::getLanguage, &GpuProgramRTTI::setLanguage);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			GpuProgram* gpuProgram = static_cast<GpuProgram*>(obj);
			gpuProgram->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "GpuProgram";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GpuProgram;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GpuProgramManager::instance().createEmpty("", GPT_VERTEX_PROGRAM); // Params don't matter, they'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
}