//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Material/B3DMaterial.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT MaterialRTTI : public TRTTIType<Material, Resource, MaterialRTTI>
	{
		SPtr<MaterialParameters> mMaterialParameters;

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

				object.InitializeVariations();

				if(object.GetVariationCount() > 0)
					object.SetParams(mMaterialParameters);
			}
		}

		void OnOperationStarted(Material& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mMaterialParameters = object.mParameters;
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
