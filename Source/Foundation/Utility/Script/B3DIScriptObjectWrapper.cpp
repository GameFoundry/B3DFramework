//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Script/B3DIScriptObjectWrapper.h"
#include "B3DApplication.h"
#include "B3DIScriptExportable.h"

using namespace b3d;

IScriptObjectWrapper::IScriptObjectWrapper(IScriptExportable* nativeObject)
	:mNativeObject(nativeObject)
{
	if(nativeObject)
		nativeObject->AssociateWithScriptObjectWrapper(this);
}

void IScriptObjectWrapper::NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload)
{
	IScriptExportable* nativeObject = GetNativeObject();
	if(nativeObject != nullptr)
		nativeObject->ClearAssociatedScriptObjectWrapper();
}

void IScriptObjectWrapper::NotifyNativeObjectDestroyed()
{
	EnsureMainThread();

	mNativeObject = nullptr;
}
