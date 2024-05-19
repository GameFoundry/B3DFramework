//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStringIDRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "Material/BsTechnique.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT TechniqueRTTI : public RTTIType<Technique, IReflectable, TechniqueRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mPasses, 0)
			B3D_RTTI_MEMBER(mLanguage, 1)
			B3D_RTTI_MEMBER(mVariationParameters, 2)
			B3D_RTTI_MEMBER(mHasPassData, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override
		{
			Technique* technique = static_cast<Technique*>(obj);
			technique->Initialize();
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

	class B3D_CORE_EXPORT TechniqueRenderProxyRTTI : public RTTIType<ct::Technique, IReflectable, TechniqueRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mPasses, 0)
			B3D_RTTI_MEMBER(mLanguage, 1)
			B3D_RTTI_MEMBER(mVariationParameters, 2)
			B3D_RTTI_MEMBER(mHasPassData, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override
		{
			ct::Technique* technique = static_cast<ct::Technique*>(obj);
			technique->Initialize();
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
			return ct::Technique::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
