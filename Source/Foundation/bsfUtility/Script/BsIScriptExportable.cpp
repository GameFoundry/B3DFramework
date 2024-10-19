//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Script/BsIScriptExportable.h"
#include "BsIScriptObjectWrapper.h"
#include "Debug/BsDebug.h"

using namespace bs;

IScriptExportable::IScriptExportable(const IScriptExportable& other)
{
	// Do nothing
}

IScriptExportable::IScriptExportable(IScriptExportable&& other)
{
	mScriptObjectWrapper = std::exchange(other.mScriptObjectWrapper, nullptr);
}

IScriptExportable& IScriptExportable::operator=(const IScriptExportable& other)
{
	if(this == &other)
		return *this;

	mScriptObjectWrapper = nullptr;
	return *this;
}

IScriptExportable& IScriptExportable::operator=(IScriptExportable&& other)
{
	mScriptObjectWrapper = std::exchange(other.mScriptObjectWrapper, nullptr);
	return *this;
}

IScriptExportable::~IScriptExportable()
{
	if(mScriptObjectWrapper != nullptr)
		mScriptObjectWrapper->NotifyNativeObjectDestroyed();
}

void IScriptExportable::AssociateWithScriptObjectWrapper(IScriptObjectWrapper* wrapper)
{
	if(!B3D_ENSURE(mScriptObjectWrapper == nullptr))
		return;

	mScriptObjectWrapper = wrapper;
}

