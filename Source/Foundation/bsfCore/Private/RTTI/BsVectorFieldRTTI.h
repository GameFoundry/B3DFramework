//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsMathRTTI.h"
#include "Particles/BsVectorField.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT VectorFieldRTTI : public RTTIType<VectorField, Resource, VectorFieldRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_NAMED(countX, mDesc.CountX, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(countY, mDesc.CountY, 1)
			B3D_RTTI_MEMBER_PLAIN_NAMED(countZ, mDesc.CountZ, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(bounds, mDesc.Bounds, 3)
			B3D_RTTI_MEMBER_REFLPTR(mTexture, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "VectorField";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_VectorField;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return VectorField::CreateEmptyInternal();
		}

	protected:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			VectorField* vectorField = static_cast<VectorField*>(obj);
			vectorField->Initialize();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
