//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStringIDRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "Material/B3DTechnique.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT TechniqueRTTI : public TRTTIType<Technique, IReflectable, TechniqueRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mPasses, 0)
			B3D_RTTI_MEMBER(mLanguage, 1)
			B3D_RTTI_MEMBER(mVariationParameters, 2)
			B3D_RTTI_MEMBER(mHasPassData, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(Technique& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Technique";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Technique;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Technique::CreateEmpty();
		}
	};

	class B3D_EXPORT TechniqueRenderProxyRTTI : public TRTTIType<render::Technique, IReflectable, TechniqueRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mPasses, 0)
			B3D_RTTI_MEMBER(mLanguage, 1)
			B3D_RTTI_MEMBER(mVariationParameters, 2)
			B3D_RTTI_MEMBER(mHasPassData, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(render::Technique& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "TechniqueRenderProxy";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_TechniqueRenderProxy;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return render::Technique::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
