//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsSavedResourceData.h"
#include "Private/RTTI/BsSavedResourceDataRTTI.h"

namespace bs
{
	SavedResourceData::SavedResourceData(const Vector<UUID>& dependencies, bool allowAsync, UINT32 compressionMethod)
		:mDependencies(dependencies), mAllowAsync(allowAsync), mCompressionMethod(compressionMethod)
	{ }

	RTTITypeBase* SavedResourceData::getRTTIStatic()
	{
		return SavedResourceDataRTTI::instance();
	}

	RTTITypeBase* SavedResourceData::getRTTI() const
	{
		return SavedResourceData::getRTTIStatic();
	}
}