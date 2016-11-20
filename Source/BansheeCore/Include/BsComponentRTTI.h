//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsComponent.h"
#include "BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ComponentRTTI : public RTTIType<Component, GameObject, ComponentRTTI>
	{
	public:
		ComponentRTTI()
		{ }

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Component* comp = static_cast<Component*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new components).
			if (comp->mRTTIData.empty())
				return;

			GODeserializationData& deserializationData = any_cast_ref<GODeserializationData>(comp->mRTTIData);

			// This shouldn't be null during normal deserialization but could be during some other operations, like applying
			// a binary diff.
			if (deserializationData.ptr != nullptr)
			{
				// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
				// deserialized handles pointing to this object can be resolved.
				SPtr<Component> compPtr = std::static_pointer_cast<Component>(deserializationData.ptr);

				GameObjectManager::instance().registerObject(compPtr, deserializationData.originalId);
			}
			
			comp->mRTTIData = nullptr;
		}

		const String& getRTTIName() override
		{
			static String name = "Component";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Component;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}