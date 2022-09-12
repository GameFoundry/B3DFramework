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

	class BS_CORE_EXPORT TechniqueRTTI : public RTTIType<Technique, IReflectable, TechniqueRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			//BS_RTTI_MEMBER_PLAIN(mRenderer, 1)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mPasses, 2)
			BS_RTTI_MEMBER_PLAIN_ARRAY(mTags, 3)
			BS_RTTI_MEMBER_PLAIN(mLanguage, 4)
			BS_RTTI_MEMBER_REFL(mVariation, 5)
		BS_END_RTTI_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Technique* technique = static_cast<Technique*>(obj);
			technique->Initialize();
		}

		const String& GetRTTIName() override
		{
			static String name = "Technique";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_Technique;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return Technique::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
