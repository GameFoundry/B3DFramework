//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Mesh/B3DMesh.h"
#include "Animation/B3DSkeleton.h"
#include "Animation/B3DMorphShapes.h"
#include "CoreObject/B3DRenderThread.h"
#include "RTTI/B3DFlagsRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class MeshRTTI : public TRTTIType<Mesh, MeshBase, MeshRTTI>
	{
		SPtr<MeshData> mMeshData;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mVertexDescription, 0)
			B3D_RTTI_MEMBER(mIndexType, 1)
			B3D_RTTI_MEMBER(mFlags, 2)
			B3D_RTTI_GENERATED_MEMBER(mMeshData, 3)
			B3D_RTTI_MEMBER(mSkeleton, 4)
			B3D_RTTI_MEMBER(mMorphShapes, 5)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(Mesh& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit) && operationType != RTTIOperationType::GatherReferences)
			{
				mMeshData = object.AllocBuffer();

				object.ReadData(mMeshData);
				GetRenderThread().PostCommand([] {}, "MeshRTTI::GetMeshData", true, object.GetName());
			}
		}

		void OnOperationEnded(Mesh& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
			{
				object.mCPUData = mMeshData;
				object.Initialize();
			}
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
} // namespace b3d
