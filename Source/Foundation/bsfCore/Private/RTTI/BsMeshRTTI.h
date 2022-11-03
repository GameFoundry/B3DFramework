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
#include "CoreThread/BsCoreThread.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class MeshRTTI : public RTTIType<Mesh, MeshBase, MeshRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFLPTR(mVertexDesc, 0)
			B3D_RTTI_MEMBER_PLAIN(mIndexType, 1)
			B3D_RTTI_MEMBER_PLAIN(mUsage, 2)
			B3D_RTTI_MEMBER_REFLPTR(mSkeleton, 4)
			B3D_RTTI_MEMBER_REFLPTR(mMorphShapes, 5)
		B3D_RTTI_END_MEMBERS

		SPtr<MeshData> GetMeshData(Mesh* obj)
		{
			SPtr<MeshData> meshData = obj->AllocBuffer();

			obj->ReadData(meshData);
			GetCoreThread().SubmitAll(true);

			return meshData;
		}

		void SetMeshData(Mesh* obj, SPtr<MeshData> meshData)
		{
			obj->mCPUData = meshData;
		}

	public:
		MeshRTTI()
		{
			AddReflectablePtrField("mMeshData", 3, &MeshRTTI::GetMeshData, &MeshRTTI::SetMeshData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Mesh* mesh = static_cast<Mesh*>(obj);
			mesh->Initialize();
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Mesh::CreateEmpty();
		}

		const String& GetRttiName() override
		{
			static String name = "Mesh";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Mesh;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
