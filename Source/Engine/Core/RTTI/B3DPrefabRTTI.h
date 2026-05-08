//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "Scene/B3DPrefab.h"
#include "Scene/B3DSceneObject.h"
#include "Utility/B3DUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PrefabRTTI : public TRTTIType<Prefab, Resource, PrefabRTTI>
	{
		TShared<SceneObject> mRootSceneObject;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mRootSceneObject, 0)
			B3D_RTTI_MEMBER(mPrefabVersion, 1)
			// B3D_RTTI_MEMBER_PLAIN(mNextLinkId, 2)
			B3D_RTTI_MEMBER(mUUID, 3)
			//B3D_RTTI_MEMBER(mIsScene, 4)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(Prefab& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				RTTIOperationEngineContext* const serializationContext = context.As<RTTIOperationEngineContext>();
				if(serializationContext != nullptr)
					serializationContext->GameObjectCollection = object.mGameObjectCollection;
			}

			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mRootSceneObject = object.mRoot.GetShared();
			}
		}

		void OnOperationEnded(Prefab& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.mRoot = mRootSceneObject->GetHandle();

				if(!operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
					object.Initialize();
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Prefab";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Prefab;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return Prefab::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
