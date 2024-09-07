//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUITexture.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUITexture.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptGUITextureContents.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUITexture.h"

namespace bs
{
	ScriptGUITexture::ScriptGUITexture(MonoObject* managedInstance, GUITexture* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUITexture::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetImage", (void*)&ScriptGUITexture::InternalSetImage);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUITexture::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUITexture::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUITexture::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUITexture::InternalCreate2);

	}

	void ScriptGUITexture::InternalSetImage(ScriptGUITexture* self, MonoObject* image)
	{
		TResourceHandle<SpriteImage> tmpimage;
		ScriptRRefBase* scriptObjectWrapperimage;
		scriptObjectWrapperimage = ScriptRRefBase::GetScriptObjectWrapper(image);
		if(scriptObjectWrapperimage != nullptr)
			tmpimage = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperimage->GetNativeObject());
		static_cast<GUITexture*>(self->GetGuiElement())->SetImage(tmpimage);
	}

	void ScriptGUITexture::InternalCreate(MonoObject* managedInstance, __GUITextureContentsInterop* contents, MonoString* styleClass, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUITexture* nativeObject = GUITexture::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUITexture>())ScriptGUITexture(managedInstance, nativeObject);
	}

	void ScriptGUITexture::InternalCreate0(MonoObject* managedInstance, __GUITextureContentsInterop* contents, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUITexture* nativeObject = GUITexture::Create(tmpcontents, nativeArrayoptions);
		new (B3DAllocate<ScriptGUITexture>())ScriptGUITexture(managedInstance, nativeObject);
	}

	void ScriptGUITexture::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUITexture* nativeObject = GUITexture::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUITexture>())ScriptGUITexture(managedInstance, nativeObject);
	}

	void ScriptGUITexture::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUITexture* nativeObject = GUITexture::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUITexture>())ScriptGUITexture(managedInstance, nativeObject);
	}
}
