//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsComponent.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "Scene/BsGameObjectCollection.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ComponentRTTI : public RTTIType<Component, GameObject, ComponentRTTI>
	{
	public:
		void OnDeserializationEnded(IReflectable* object, SerializationContext* context) override
		{
			Component* const component = static_cast<Component*>(object);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new components).
			if(component->mRTTIData.Empty())
				return;

			CoreSerializationContext* const serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			B3D_ASSERT(serializationContext != nullptr);

			if(component->mId.Empty() || serializationContext->GoState->GetUseNewUuiDs())
			{
				const UUID oldId = component->GetId();
				component->mId = UUIDGenerator::GenerateRandom();

				if(!oldId.Empty())
				{
					if(serializationContext->GameObjectCollection != nullptr)
						serializationContext->GameObjectCollection->RegisterUnresolvedHandleIdRemapping(oldId, component->GetId());
				}
			}

			GODeserializationData& deserializationData = AnyCastRef<GODeserializationData>(component->mRTTIData);

			// This shouldn't be null during normal deserialization but could be during some other operations, like applying
			// a binary diff.
			if(deserializationData.Ptr != nullptr)
			{
				// Register the newly created SO with the GameObjectManager and provide it with the original ID so that
				// deserialized handles pointing to this object can be resolved.
				SPtr<Component> componentShared = std::static_pointer_cast<Component>(deserializationData.Ptr);

				GameObjectHandleBase handle;

				if(serializationContext->GameObjectCollection != nullptr)
					handle = serializationContext->GameObjectCollection->RegisterAndInitializeObject(componentShared);
				else
					handle = GameObjectManager::Instance().RegisterObject(componentShared);

				serializationContext->GoState->RegisterObject(deserializationData.OriginalId, handle);
			}

			component->mRTTIData = nullptr;
		}

		const String& GetRttiName() override
		{
			static String name = "Component";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Component;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
