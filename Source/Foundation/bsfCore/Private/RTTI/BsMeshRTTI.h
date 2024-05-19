//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "BsCoreApplication.h"
#include "Mesh/BsMesh.h"
#include "Animation/BsSkeleton.h"
#include "Animation/BsMorphShapes.h"
#include "CoreObject/BsRenderThread.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class MeshRTTI : public RTTIType<Mesh, MeshBase, MeshRTTI>
	{
		SPtr<MeshData> mMeshData;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mVertexDescription, 0)
			B3D_RTTI_MEMBER(mIndexType, 1)
			B3D_RTTI_MEMBER(mUsage, 2)
			B3D_RTTI_GENERATED_MEMBER(mMeshData, 3)
			B3D_RTTI_MEMBER(mSkeleton, 4)
			B3D_RTTI_MEMBER(mMorphShapes, 5)
		B3D_RTTI_END_MEMBERS

	public:
		void OnSerializationStarted(IReflectable* object, RTTIOperationContext* context) override
		{
			Mesh* const mesh = static_cast<Mesh*>(object);
			mMeshData = mesh->AllocBuffer();

			mesh->ReadData(mMeshData);
			GetRenderThread().PostCommand([] {}, "MeshRTTI::GetMeshData", true, mesh->GetName());
		}

		void OnDeserializationEnded(IReflectable* object, RTTIOperationContext* context) override
		{
			Mesh* const mesh = static_cast<Mesh*>(object);
			mesh->mCPUData = mMeshData;
			mesh->Initialize();
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Mesh::CreateEmptyShared();
		}

		const String& GetRttiName() override
		{
			static String name = "Mesh";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Mesh;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
