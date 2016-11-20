//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSavedResourceData.h"
#include "BsSavedResourceDataRTTI.h"

namespace bs
{
	SavedResourceData::SavedResourceData()
		:mAllowAsync(true)
	{ }

	SavedResourceData::SavedResourceData(const Vector<String>& dependencies, bool allowAsync)
		:mDependencies(dependencies), mAllowAsync(allowAsync)
	{

	}

	RTTITypeBase* SavedResourceData::getRTTIStatic()
	{
		return SavedResourceDataRTTI::instance();
	}

	RTTITypeBase* SavedResourceData::getRTTI() const
	{
		return SavedResourceData::getRTTIStatic();
	}
}