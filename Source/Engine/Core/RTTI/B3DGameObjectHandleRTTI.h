//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "Scene/B3DGameObjectCollection.h"
#include "Scene/B3DGameObjectHandle.h"
#include "Scene/B3DGameObjectManager.h"
#include "Utility/B3DUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	/** Specializes delta handler that is only used for GameObjectHandle type. Allows the system to perform ad-hoc ID remapping while comparing handles. */
	class B3D_EXPORT GameObjectHandleDeltaHandler : public BinaryDeltaHandler
	{
	protected:
		SPtr<SerializedObject> GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly) override;
	};

	inline SPtr<SerializedObject> GameObjectHandleDeltaHandler::GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly)
	{
		if(B3D_ENSURE(original == nullptr && modified != nullptr))
			return nullptr;

		auto fnGetOrDecodeHandle = [&context](IReflectable* object, SPtr<GameObjectHandle>& outDecodedHandle) -> GameObjectHandle*
		{
			if(object->GetTypeId() == TID_SerializedObject)
			{
				SerializedObject* serializedObject = B3DRTTICast<SerializedObject>(object);
				if(!B3D_ENSURE(serializedObject->GetRootTypeId() == TID_GameObjectHandleBase))
					return nullptr;

				outDecodedHandle = B3DRTTICast<GameObjectHandle>(serializedObject->Decode(context));
				return outDecodedHandle.get();
			}

			return B3DRTTICast<GameObjectHandle>(object);
		};

		SPtr<GameObjectHandle> originalHandleShared;
		GameObjectHandle* const originalHandle = fnGetOrDecodeHandle(original, originalHandleShared);
		if(!B3D_ENSURE(originalHandle != nullptr))
			return nullptr;

		SPtr<GameObjectHandle> modifiedHandleShared;
		GameObjectHandle* const modifiedHandle = fnGetOrDecodeHandle(modified, modifiedHandleShared);
		if(!B3D_ENSURE(modifiedHandle != nullptr))
			return nullptr;

		UUID originalId = originalHandle->GetId();
		UUID modifiedId = modifiedHandle->GetId();

		if(auto* serializationContext = context.As<RTTIOperationEngineContext>())
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

	class B3D_EXPORT GameObjectHandleRTTI : public TRTTIType<GameObjectHandle, IReflectable, GameObjectHandleRTTI>
	{
		UUID mId;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mId, 0)
		B3D_RTTI_END_MEMBERS

	public:
		IDeltaHandler& GetDeltaHandler() const override
		{
			static GameObjectHandleDeltaHandler kDeltaHandler;
			return kDeltaHandler;
		}

		void OnOperationStarted(GameObjectHandle& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				if(object.mSharedHandleData != nullptr)
				{
					const UUID& id = object.mSharedHandleData->Id;

					if(auto* serializationContext = context.As<RTTIOperationEngineContext>())
					{
						if(auto found = serializationContext->GameObjectIdRemapping.find(id); found != serializationContext->GameObjectIdRemapping.end())
							mId = found->second;
						else
							mId = id;
					}
					else
						mId = id;
				}
				else
					mId = UUID::kEmpty;
			}
		}

		void OnOperationEnded(GameObjectHandle& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(object.mSharedHandleData != nullptr)
					object.mSharedHandleData->Id = mId;

				if(auto* serializationContext = context.As<RTTIOperationEngineContext>())
				{
					if(serializationContext->GameObjectCollection != nullptr)
						serializationContext->GameObjectCollection->RegisterUnresolvedHandle(object);
				}
			}
		}

		const String& GetRttiName()
		{
			static String name = "GameObjectHandle";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GameObjectHandleBase;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeSharedFromExisting<GameObjectHandle>(new(B3DAllocate<GameObjectHandle>()) GameObjectHandle());
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
