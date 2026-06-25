//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStringIDRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT VariationPrecompiledDataRTTI : public TRTTIType<VariationPrecompiledData, IReflectable, VariationPrecompiledDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Language, 0)
			B3D_RTTI_MEMBER(VariationParameters, 1)
			B3D_RTTI_MEMBER_CONTAINER(Passes, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "VariationPrecompiledData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_VariationPrecompiledData;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<VariationPrecompiledData>();
		}
	};

	class B3D_EXPORT VariationRTTI : public TRTTIType<Variation, IReflectable, VariationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mPasses, 0)
			B3D_RTTI_MEMBER(mLanguage, 1)
			B3D_RTTI_MEMBER(mVariationParameters, 2)
			B3D_RTTI_MEMBER(mHasPassData, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(Variation& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Variation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Variation;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return Variation::CreateEmpty();
		}
	};

	class B3D_EXPORT VariationRenderProxyRTTI : public TRTTIType<render::Variation, IReflectable, VariationRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mPasses, 0)
			B3D_RTTI_MEMBER(mLanguage, 1)
			B3D_RTTI_MEMBER(mVariationParameters, 2)
			B3D_RTTI_MEMBER(mHasPassData, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(render::Variation& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "VariationRenderProxy";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_VariationRenderProxy;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return render::Variation::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
