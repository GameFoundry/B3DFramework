//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUITexture.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUITexture.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptGUIOption.generated.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptGUITexture.generated.h"
#include "B3DScriptGUITextureContents.generated.h"

namespace b3d
{
	ScriptGUITexture::ScriptGUITexture(GUITexture* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUITexture::~ScriptGUITexture()
	{
		UnregisterEvents();
	}

	void ScriptGUITexture::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetImage", (void*)&ScriptGUITexture::InternalSetImage);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUITexture::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUITexture::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUITexture::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUITexture::InternalCreate2);

	}

	MonoObject* ScriptGUITexture::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUITexture::InternalSetImage(ScriptGUITexture* self, MonoObject* image)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<SpriteImage> tmpimage;
		ScriptRRefBase* scriptObjectWrapperimage;
		scriptObjectWrapperimage = ScriptRRefBase::GetScriptObjectWrapper(image);
		if(scriptObjectWrapperimage != nullptr)
			tmpimage = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperimage->GetNativeObject());
		static_cast<GUITexture*>(self->GetNativeObject())->SetImage(tmpimage);
	}

	void ScriptGUITexture::InternalCreate(MonoObject* scriptObject, __GUITextureContentsInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUITextureContents tmpcontents;
		tmpcontents = ScriptGUITextureContents::FromInterop(*contents);
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUITexture* nativeObject = GUITexture::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUITexture>(nativeObject, scriptObject);
	}

	void ScriptGUITexture::InternalCreate0(MonoObject* scriptObject, __GUITextureContentsInterop* contents, MonoArray* options)
	{
		GUITextureContents tmpcontents;
		tmpcontents = ScriptGUITextureContents::FromInterop(*contents);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUITexture* nativeObject = GUITexture::Create(tmpcontents, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUITexture>(nativeObject, scriptObject);
	}

	void ScriptGUITexture::InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUITexture* nativeObject = GUITexture::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUITexture>(nativeObject, scriptObject);
	}

	void ScriptGUITexture::InternalCreate2(MonoObject* scriptObject, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUITexture* nativeObject = GUITexture::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUITexture>(nativeObject, scriptObject);
	}
}
