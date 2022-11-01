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
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Component* comp = static_cast<Component*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new components).
			if(comp->mRTTIData.Empty())
				return;

			BS_ASSERT(context != nullptr && B3DRTTIIsOfType<CoreSerializationContext>(context));
			auto coreContext = static_cast<CoreSerializationContext*>(context);

			GODeserializationData& deserializationData = any_cast_ref<GODeserializationData>(comp->mRTTIData);

			// This shouldn't be null during normal deserialization but could be during some other operations, like applying
			// a binary diff.
			if(deserializationData.Ptr != nullptr)
			{
				// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
				// deserialized handles pointing to this object can be resolved.
				SPtr<Component> compPtr = std::static_pointer_cast<Component>(deserializationData.Ptr);

				GameObjectHandleBase handle = GameObjectManager::Instance().RegisterObject(compPtr);
				coreContext->GoState->RegisterObject(deserializationData.OriginalId, handle);
			}

			if(comp->mUUID.Empty() || coreContext->GoState->GetUseNewUuiDs())
				comp->mUUID = UUIDGenerator::GenerateRandom();

			comp->mRTTIData = nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "Component";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Component;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
