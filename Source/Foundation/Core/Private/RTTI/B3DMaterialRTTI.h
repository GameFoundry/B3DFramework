//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DCorePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Material/B3DMaterial.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT MaterialRTTI : public TRTTIType<Material, Resource, MaterialRTTI>
	{
		SPtr<MaterialParams> mMaterialParameters;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mShader, 0)
			B3D_RTTI_GENERATED_MEMBER(mMaterialParameters, 2)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(Material& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				if(!operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
					object.Initialize();

				if(!mMaterialParameters)
					return;

				object.InitializeTechniques();

				if(object.GetNumTechniques() > 0)
					object.SetParams(mMaterialParameters);
			}
		}

		void OnOperationStarted(Material& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mMaterialParameters = object.mParams;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Material";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Material;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Material::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
