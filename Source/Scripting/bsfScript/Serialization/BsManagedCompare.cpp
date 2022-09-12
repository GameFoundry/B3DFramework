//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedCompare.h"
#include "Serialization/BsManagedSerializableObject.h"

namespace bs
{
	bool ManagedCompare::Run(IReflectable& a, IReflectable& b)
	{
		auto managedA = static_cast<ManagedSerializableObject&>(a);
		auto managedB = static_cast<ManagedSerializableObject&>(b);

		return managedA.Equals(managedB);
	}
}
