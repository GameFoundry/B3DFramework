//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Scene/BsGameObjectCollection.h"
#include "Scene/BsGameObjectHandle.h"
#include "Scene/BsGameObjectManager.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	/** Specializes delta handler that is only used for GameObjectHandle type. Allows the system to perform ad-hoc ID remapping while comparing handles. */
	class B3D_UTILITY_EXPORT GameObjectHandleDeltaHandler : public BinaryDeltaHandler
	{
	protected:
		SPtr<SerializedObject> GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext* context, bool replicableOnly) override;
	};

	inline SPtr<SerializedObject> GameObjectHandleDeltaHandler::GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext* context, bool replicableOnly)
	{
		if(B3D_ENSURE(original == nullptr && modified != nullptr))
			return nullptr;

		auto fnGetOrDecodeHandle = [context](IReflectable* object, SPtr<GameObjectHandleBase>& outDecodedHandle) -> GameObjectHandleBase*
		{
			if(object->GetTypeId() == TID_SerializedObject)
			{
				SerializedObject* serializedObject = B3DRTTICast<SerializedObject>(object);
				if(!B3D_ENSURE(serializedObject->GetRootTypeId() == TID_GameObjectHandleBase))
					return nullptr;

				outDecodedHandle = B3DRTTICast<GameObjectHandleBase>(serializedObject->Decode(context));
				return outDecodedHandle.get();
			}

			return B3DRTTICast<GameObjectHandleBase>(object);
		};

		SPtr<GameObjectHandleBase> originalHandleShared;
		GameObjectHandleBase* const originalHandle = fnGetOrDecodeHandle(original, originalHandleShared);
		if(!B3D_ENSURE(originalHandle != nullptr))
			return nullptr;

		SPtr<GameObjectHandleBase> modifiedHandleShared;
		GameObjectHandleBase* const modifiedHandle = fnGetOrDecodeHandle(modified, modifiedHandleShared);
		if(!B3D_ENSURE(modifiedHandle != nullptr))
			return nullptr;

		UUID originalId = originalHandle->GetId();
		UUID modifiedId = modifiedHandle->GetId();

		if(auto* serializationContext = B3DRTTICast<RTTIOperationEngineContext>(context))
		{
			if(auto found = serializationContext->GameObjectIdRemapping.find(originalId); found != serializationContext->GameObjectIdRemapping.end())
				originalId = found->second;

			if(auto found = serializationContext->GameObjectIdRemapping.find(modifiedId); found != serializationContext->GameObjectIdRemapping.end())
				modifiedId = found->second;
		}

		if(originalId == modifiedId)
			return nullptr;

		return SerializedObject::Create(*modifiedHandle);
	}

	class B3D_CORE_EXPORT GameObjectHandleRTTI : public RTTIType<GameObjectHandleBase, IReflectable, GameObjectHandleRTTI>
	{
	private:
		UUID& GetId(GameObjectHandleBase* object)
		{
			if(!mRemappedId.Empty())
				return mRemappedId;

			if(object->mSharedHandleData == nullptr)
				return const_cast<UUID&>(UUID::kEmpty);

			return object->mSharedHandleData->Id;
		}

		void SetId(GameObjectHandleBase* object, UUID& value)
		{
			if(object->mSharedHandleData == nullptr)
				return;

			object->mSharedHandleData->Id = value;
		}

	public:
		GameObjectHandleRTTI()
		{
			//AddPlainField("mInstanceID", 0, &GameObjectHandleRTTI::GetInstanceId, &GameObjectHandleRTTI::SetInstanceId);
			AddPlainField("mId", 1, &GameObjectHandleRTTI::GetId, &GameObjectHandleRTTI::SetId);
		}

		IDeltaHandler& GetDeltaHandler() const override
		{
			static GameObjectHandleDeltaHandler kDeltaHandler;
			return kDeltaHandler;
		}

		void OnDeserializationEnded(IReflectable* object, RTTIOperationContext* context)
		{
			if(auto* serializationContext = B3DRTTICast<RTTIOperationEngineContext>(context))
			{
				GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(object);
				if(serializationContext->GameObjectCollection != nullptr)
					serializationContext->GameObjectCollection->RegisterUnresolvedHandle(*gameObjectHandle);
			}
		}

		void OnSerializationStarted(IReflectable* object, RTTIOperationContext* context) override
		{
			if(auto* serializationContext = B3DRTTICast<RTTIOperationEngineContext>(context))
			{
				GameObjectHandleBase* gameObjectHandle = static_cast<GameObjectHandleBase*>(object);
				if(auto found = serializationContext->GameObjectIdRemapping.find(gameObjectHandle->GetId()); found != serializationContext->GameObjectIdRemapping.end())
					mRemappedId = found->second;
			}
		}

		const String& GetRttiName()
		{
			static String name = "GameObjectHandleBase";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GameObjectHandleBase;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<GameObjectHandleBase>(new(B3DAllocate<GameObjectHandleBase>()) GameObjectHandleBase());
		}

		UUID mRemappedId;
	};

	/** @} */
	/** @endcond */
} // namespace bs
