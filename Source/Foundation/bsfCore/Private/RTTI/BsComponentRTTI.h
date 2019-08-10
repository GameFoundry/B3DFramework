//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsComponent.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ComponentRTTI : public RTTIType<Component, GameObject, ComponentRTTI>
	{
	public:
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Component* comp = static_cast<Component*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new components).
			if (comp->mRTTIData.empty())
				return;

			BS_ASSERT(context != nullptr && rtti_is_of_type<CoreSerializationContext>(context));
			auto coreContext = static_cast<CoreSerializationContext*>(context);

			GODeserializationData& deserializationData = any_cast_ref<GODeserializationData>(comp->mRTTIData);

			// This shouldn't be null during normal deserialization but could be during some other operations, like applying
			// a binary diff.
			if (deserializationData.ptr != nullptr)
			{
				// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
				// deserialized handles pointing to this object can be resolved.
				SPtr<Component> compPtr = std::static_pointer_cast<Component>(deserializationData.ptr);

				GameObjectHandleBase handle = GameObjectManager::instance().registerObject(compPtr);
				coreContext->goState->registerObject(deserializationData.originalId, handle);
			}

			if(comp->mUUID.empty() || coreContext->goState->getUseNewUUIDs())
				comp->mUUID = UUIDGenerator::generateRandom();
			
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
