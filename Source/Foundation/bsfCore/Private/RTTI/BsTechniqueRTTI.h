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
			// B3D_RTTI_MEMBER_PLAIN(mRenderer, 1)
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(mPasses, 2)
			//B3D_RTTI_MEMBER_PLAIN_ARRAY(mTags, 3)
			B3D_RTTI_MEMBER_PLAIN(mLanguage, 4)
			B3D_RTTI_MEMBER_REFL(mVariationParameters, 5)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Technique* technique = static_cast<Technique*>(obj);
			technique->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Technique";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Technique;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Technique::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
