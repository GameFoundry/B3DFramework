//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "Resources/BsPlainText.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT PlainTextRTTI : public RTTIType<PlainText, Resource, PlainTextRTTI>
	{
	private:
		WString& GetString(PlainText* obj) { return obj->mString; }

		void SetString(PlainText* obj, WString& val) { obj->mString = val; }

	public:
		PlainTextRTTI()
		{
			AddPlainField("mString", 0, &PlainTextRTTI::GetString, &PlainTextRTTI::SetString);
		}

		const String& GetRttiName()
		{
			static String name = "PlainText";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_PlainText;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return PlainText::CreatePtrInternal(L""); // Initial string doesn't matter, it'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
