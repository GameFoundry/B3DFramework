//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsPlainText.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsPlainTextRTTI.h"

using namespace bs;

PlainText::PlainText(const WString& data)
	: Resource(false), mString(data)
{
}

HPlainText PlainText::Create(const WString& data)
{
	return B3DStaticResourceCast<PlainText>(GetResources().CreateResourceHandle(CreatePtrInternal(data)));
}

SPtr<PlainText> PlainText::CreatePtrInternal(const WString& data)
{
	SPtr<PlainText> plainTextPtr = B3DMakeSharedFromExisting<PlainText>(
		new(B3DAllocate<PlainText>()) PlainText(data));
	plainTextPtr->SetShared(plainTextPtr);
	plainTextPtr->Initialize();

	return plainTextPtr;
}

RTTITypeBase* PlainText::GetRttiStatic()
{
	return PlainTextRTTI::Instance();
}

RTTITypeBase* PlainText::GetRtti() const
{
	return PlainText::GetRttiStatic();
}
