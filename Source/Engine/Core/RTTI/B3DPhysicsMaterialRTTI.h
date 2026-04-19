//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Physics/B3DPhysicsMaterial.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PhysicsMaterialRTTI : public TRTTIType<PhysicsMaterial, Resource, PhysicsMaterialRTTI>
	{
		float mStaticFriction;
		float mDynamicFriction;
		float mRestitutionCoefficient;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mStaticFriction, 0)
			B3D_RTTI_GENERATED_MEMBER(mDynamicFriction, 1)
			B3D_RTTI_GENERATED_MEMBER(mRestitutionCoefficient, 2)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(PhysicsMaterial& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mStaticFriction = object.GetStaticFriction();
				mDynamicFriction = object.GetDynamicFriction();
				mRestitutionCoefficient = object.GetRestitutionCoefficient();
			}
		}

		void OnOperationEnded(PhysicsMaterial& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.SetStaticFriction(mStaticFriction);
				object.SetDynamicFriction(mDynamicFriction);
				object.SetRestitutionCoefficient(mRestitutionCoefficient);
			}
		}

		const String& GetRttiName()
		{
			static String name = "PhysicsMaterial";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PhysicsMaterial;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return PhysicsMaterial::CreatePtrInternal();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
