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

	class BS_CORE_EXPORT MaterialRTTI : public RTTIType<Material, Resource, MaterialRTTI>
	{
	private:
		HShader& getShader(Material* obj) { return obj->mShader; }
		void setShader(Material* obj, HShader& val) { obj->mShader = val; }

		SPtr<MaterialParams> getMaterialParams(Material* obj) { return obj->mParams; }
		void setMaterialParams(Material* obj, SPtr<MaterialParams> value) { mMatParams = value; }

	public:
		MaterialRTTI()
		{
			addReflectableField("mShader", 0, &MaterialRTTI::getShader, &MaterialRTTI::setShader);
			addReflectablePtrField("mMaterialParams", 2, &MaterialRTTI::getMaterialParams, &MaterialRTTI::setMaterialParams);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override;

		const String& getRTTIName() override
		{
			static String name = "Material";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Material;
		}

		SPtr<IReflectable> newRTTIObject() override;

	private:
		SPtr<MaterialParams> mMatParams;
	};

	/** @} */
	/** @endcond */
}