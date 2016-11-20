//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsSavedResourceData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT SavedResourceDataRTTI : public RTTIType <SavedResourceData, IReflectable, SavedResourceDataRTTI>
	{
	private:
		String& getDependency(SavedResourceData* obj, UINT32 arrayIdx) { return obj->mDependencies[arrayIdx]; }
		void setDependency(SavedResourceData* obj, UINT32 arrayIdx, String& val) { obj->mDependencies[arrayIdx] = val; }

		UINT32 getNumDependencies(SavedResourceData* obj) { return (UINT32)obj->mDependencies.size(); }
		void setNumDependencies(SavedResourceData* obj, UINT32 numEntries) { obj->mDependencies = Vector<String>(numEntries); }

		bool& getAllowAsyncLoading(SavedResourceData* obj) { return obj->mAllowAsync; }
		void setAllowAsyncLoading(SavedResourceData* obj, bool& value) { obj->mAllowAsync = value; }

	public:
		SavedResourceDataRTTI()
		{
			addPlainArrayField("mDependencies", 0, &SavedResourceDataRTTI::getDependency, &SavedResourceDataRTTI::getNumDependencies,
				&SavedResourceDataRTTI::setDependency, &SavedResourceDataRTTI::setNumDependencies);
			addPlainField("mAllowAsync", 1, &SavedResourceDataRTTI::getAllowAsyncLoading, &SavedResourceDataRTTI::setAllowAsyncLoading);
		}

		const String& getRTTIName() override
		{
			static String name = "ResourceDependencies";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ResourceDependencies;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SavedResourceData>();
		}
	};

	/** @} */
	/** @endcond */
}