//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsPlainText.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT PlainTextRTTI : public RTTIType <PlainText, Resource, PlainTextRTTI>
	{
	private:
		WString& getString(PlainText* obj) { return obj->mString; }
		void setString(PlainText* obj, WString& val) { obj->mString = val; }
	public:
		PlainTextRTTI()
		{
			addPlainField("mString", 0, &PlainTextRTTI::getString, &PlainTextRTTI::setString);
		}

		const String& getRTTIName() override
		{
			static String name = "PlainText";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PlainText;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return PlainText::_createPtr(L""); // Initial string doesn't matter, it'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
}