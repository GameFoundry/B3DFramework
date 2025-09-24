//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteVectorPath.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"
#include "BsScriptTSize2.generated.h"
#include "BsScriptSpriteVectorPathCreateInformation.generated.h"

namespace b3d
{
	ScriptSpriteVectorPath::ScriptSpriteVectorPath(const TResourceHandle<SpriteVectorPath>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSpriteVectorPath::~ScriptSpriteVectorPath()
	{
		UnregisterEvents();
	}

	void ScriptSpriteVectorPath::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteVectorPath::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSpriteVectorPath::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSpriteVectorPath::InternalCreate0);

	}

	MonoObject* ScriptSpriteVectorPath::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptSpriteVectorPath::InternalGetRef(ScriptSpriteVectorPath* self)
	{
		return self->GetOrCreateResourceReference();
	}

	void ScriptSpriteVectorPath::InternalCreate(MonoObject* scriptObject, MonoObject* vectorPath, TSize2<int32_t>* defaultSize)
	{
		TResourceHandle<VectorPath> tmpvectorPath;
		ScriptRRefBase* scriptObjectWrappervectorPath;
		scriptObjectWrappervectorPath = ScriptRRefBase::GetScriptObjectWrapper(vectorPath);
		if(scriptObjectWrappervectorPath != nullptr)
			tmpvectorPath = B3DStaticResourceCast<VectorPath>(scriptObjectWrappervectorPath->GetNativeObject());
		TResourceHandle<SpriteVectorPath> nativeObject = SpriteVectorPath::Create(tmpvectorPath, *defaultSize);
		ScriptObjectWrapper::Create<ScriptSpriteVectorPath>(nativeObject, scriptObject);
	}

	void ScriptSpriteVectorPath::InternalCreate0(MonoObject* scriptObject, __SpriteVectorPathCreateInformationInterop* createInformation)
	{
		SpriteVectorPathCreateInformation tmpcreateInformation;
		tmpcreateInformation = ScriptSpriteVectorPathCreateInformation::FromInterop(*createInformation);
		TResourceHandle<SpriteVectorPath> nativeObject = SpriteVectorPath::Create(tmpcreateInformation);
		ScriptObjectWrapper::Create<ScriptSpriteVectorPath>(nativeObject, scriptObject);
	}
}
