//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "Scene/B3DScene.h"
#include "Scene/B3DSceneObject.h"
#include "Utility/B3DUtility.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SceneRTTI : public TRTTIType<Scene, Resource, SceneRTTI>
	{
		SPtr<SceneObject> mRootSceneObject;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mRootSceneObject, 0)
			B3D_RTTI_MEMBER(mUUID, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(Scene& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
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

		void OnOperationEnded(Scene& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
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
			static String name = "Scene";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Scene;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Scene::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
