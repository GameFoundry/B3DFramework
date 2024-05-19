//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Material/BsMaterial.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT MaterialRTTI : public RTTIType<Material, Resource, MaterialRTTI>
	{
	private:
		HShader& GetShader(Material* obj) { return obj->mShader; }

		void SetShader(Material* obj, HShader& val) { obj->mShader = val; }

		SPtr<MaterialParams> GetMaterialParams(Material* obj) { return obj->mParams; }

		void SetMaterialParams(Material* obj, SPtr<MaterialParams> value) { mMatParams = value; }

	public:
		MaterialRTTI()
		{
			AddReflectableField("mShader", 0, &MaterialRTTI::GetShader, &MaterialRTTI::SetShader);
			AddReflectablePtrField("mMaterialParams", 2, &MaterialRTTI::GetMaterialParams, &MaterialRTTI::SetMaterialParams);
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override;

		const String& GetRttiName() override
		{
			static String name = "Material";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Material;
		}

		SPtr<IReflectable> NewRttiObject();

	private:
		SPtr<MaterialParams> mMatParams;
	};

	/** @} */
	/** @endcond */
} // namespace bs
