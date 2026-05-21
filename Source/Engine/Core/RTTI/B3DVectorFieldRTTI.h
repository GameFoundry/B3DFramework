//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DMathRTTI.h"
#include "Particles/B3DVectorField.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT VectorFieldRTTI : public TRTTIType<VectorField, Resource, VectorFieldRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_NAMED(countX, mDesc.CountX, 0)
			B3D_RTTI_MEMBER_NAMED(countY, mDesc.CountY, 1)
			B3D_RTTI_MEMBER_NAMED(countZ, mDesc.CountZ, 2)
			B3D_RTTI_MEMBER_NAMED(bounds, mDesc.Bounds, 3)
			B3D_RTTI_MEMBER(mTexture, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "VectorField";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_VectorField;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return VectorField::CreateEmpty();
		}

	protected:
		void OnOperationEnded(VectorField& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
