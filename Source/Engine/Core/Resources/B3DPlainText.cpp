//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DPlainText.h"
#include "Resources/B3DResources.h"
#include "RTTI/B3DPlainTextRTTI.h"

using namespace b3d;

PlainText::PlainText(const WString& data)
	: Resource(false), mString(data)
{
}

HPlainText PlainText::Create(const WString& data)
{
	return B3DStaticResourceCast<PlainText>(GetResources().CreateResourceHandle(CreateShared(data)));
}

TShared<PlainText> PlainText::CreateShared(const WString& data)
{
	TShared<PlainText> plainTextPtr = B3DMakeSharedFromExisting<PlainText>(
		new(B3DAllocate<PlainText>()) PlainText(data));
	plainTextPtr->SetShared(plainTextPtr);
	plainTextPtr->Initialize();

	return plainTextPtr;
}

RTTIType* PlainText::GetRttiStatic()
{
	return PlainTextRTTI::Instance();
}

RTTIType* PlainText::GetRtti() const
{
	return PlainText::GetRttiStatic();
}
