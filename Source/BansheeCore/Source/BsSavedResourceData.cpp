//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSavedResourceData.h"
#include "BsSavedResourceDataRTTI.h"

namespace bs
{
	SavedResourceData::SavedResourceData()
		:mAllowAsync(true), mCompressionMethod(0)
	{ }

	SavedResourceData::SavedResourceData(const Vector<String>& dependencies, bool allowAsync, UINT32 compressionMethod)
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