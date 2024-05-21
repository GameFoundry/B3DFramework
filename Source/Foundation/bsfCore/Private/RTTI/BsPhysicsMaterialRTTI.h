//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Physics/BsPhysicsMaterial.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT PhysicsMaterialRTTI : public RTTIType<PhysicsMaterial, Resource, PhysicsMaterialRTTI>
	{
	private:
		float& GetStaticFriction(PhysicsMaterial* obj)
		{
			return mStaticFriction;
		}

		void SetStaticFriction(PhysicsMaterial* obj, float& size) { obj->SetStaticFriction(size); }

		float& GetDynamicFriction(PhysicsMaterial* obj)
		{
			return mDynamicFriction;
		}

		void SetDynamicFriction(PhysicsMaterial* obj, float& size) { obj->SetDynamicFriction(size); }

		float& GetRestitutionCoefficient(PhysicsMaterial* obj)
		{
			return mRestitutionCoefficient;
		}

		void SetRestitutionCoefficient(PhysicsMaterial* obj, float& size) { obj->SetRestitutionCoefficient(size); }

	public:
		PhysicsMaterialRTTI()
		{
			AddPlainField("staticFriction", 0, &PhysicsMaterialRTTI::GetStaticFriction, &PhysicsMaterialRTTI::SetStaticFriction);
			AddPlainField("dynamicFriction", 1, &PhysicsMaterialRTTI::GetDynamicFriction, &PhysicsMaterialRTTI::SetDynamicFriction);
			AddPlainField("restitutionCoefficient", 2, &PhysicsMaterialRTTI::GetRestitutionCoefficient, &PhysicsMaterialRTTI::SetRestitutionCoefficient);
		}

		void OnOperationStarted(PhysicsMaterial& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mStaticFriction = object.GetStaticFriction();
				mDynamicFriction = object.GetDynamicFriction();
				mRestitutionCoefficient = object.GetRestitutionCoefficient();
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

	private:
		float mStaticFriction;
		float mDynamicFriction;
		float mRestitutionCoefficient;
	};

	/** @} */
	/** @endcond */
} // namespace bs
