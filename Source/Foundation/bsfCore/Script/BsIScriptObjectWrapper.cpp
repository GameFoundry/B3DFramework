//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Script/BsIScriptObjectWrapper.h"
#include "BsCoreApplication.h"
#include "BsIScriptExportable.h"

using namespace bs;

IScriptObjectWrapper::IScriptObjectWrapper(IScriptExportable* nativeObject)
	:mNativeObject(nativeObject)
{
	if(nativeObject)
		nativeObject->AssociateWithScriptObjectWrapper(this);
}

void IScriptObjectWrapper::NotifyScriptObjectDestroyed()
{
	IScriptExportable* nativeObject = GetNativeObject();
	if(B3D_ENSURE(nativeObject))
		nativeObject->ClearAssociatedScriptObjectWrapper();
}

void IScriptObjectWrapper::NotifyNativeObjectDestroyed()
{
	EnsureMainThread();

	mNativeObject = nullptr;
}
