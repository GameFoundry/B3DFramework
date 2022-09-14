//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsPlainText.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsPlainTextRTTI.h"

namespace bs
{
	PlainText::PlainText(const WString& data)
		:Resource(false), mString(data)
	{

	}

	HPlainText PlainText::create(const WString& data)
	{
		return static_resource_cast<PlainText>(gResources().CreateResourceHandleInternal(CreatePtrInternal(data)));
	}

	SPtr<PlainText> PlainText::CreatePtrInternal(const WString& data)
	{
		SPtr<PlainText> plainTextPtr = bs_core_ptr<PlainText>(
			new (bs_alloc<PlainText>()) PlainText(data));
		plainTextPtr->SetThisPtrInternal(plainTextPtr);
		plainTextPtr->initialize();

		return plainTextPtr;
	}

	RTTITypeBase* PlainText::getRTTIStatic()
	{
		return PlainTextRTTI::instance();
	}

	RTTITypeBase* PlainText::getRTTI() const
	{
		return PlainText::getRTTIStatic();
	}
}
