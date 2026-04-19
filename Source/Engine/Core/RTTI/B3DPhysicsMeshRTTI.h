//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Physics/B3DPhysicsMesh.h"
#include "Physics/B3DPhysics.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PhysicsMeshRTTI : public TRTTIType<PhysicsMesh, Resource, PhysicsMeshRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mImplementation, 0)
			B3D_RTTI_MEMBER(mType, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(PhysicsMesh& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "PhysicsMesh";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PhysicsMesh;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return PhysicsMesh::CreateEmpty();
		}
	};

	class B3D_EXPORT PhysicsMeshImplementationRTTI : public TRTTIType<IPhysicsMeshImplementation, IReflectable, PhysicsMeshImplementationRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "PhysicsMesh";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PhysicsMeshImplementation;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
