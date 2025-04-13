//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIRenderTexture.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIRenderTexture.h"
#include "GUI/BsGUIOptions.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptRenderTexture.generated.h"

using namespace bs;
ScriptGUIRenderTexture::ScriptGUIRenderTexture(GUIRenderTexture* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{
}

void ScriptGUIRenderTexture::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIRenderTexture::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTexture", (void*)&ScriptGUIRenderTexture::InternalSetTexture);
}

MonoObject* ScriptGUIRenderTexture::CreateScriptObject(bool construct)
{
	// TODO - Add a ctor in C# we can call if needed
	return nullptr;
}

void ScriptGUIRenderTexture::InternalCreateInstance(MonoObject* instance, ScriptRenderTexture* texture, bool transparent, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	SPtr<RenderTexture> renderTexture;
	if(texture != nullptr)
		renderTexture = texture->GetNativeObjectAsShared();

	GUIRenderTexture* guiTexture = GUIRenderTexture::Create(renderTexture, transparent, options, MonoUtil::MonoToString(style));

	ScriptObjectWrapper::Create<ScriptGUIRenderTexture>(guiTexture, instance);
}

void ScriptGUIRenderTexture::InternalSetTexture(ScriptGUIRenderTexture* self, ScriptRenderTexture* texture)
{
	if(!self->IsNativeObjectValid())
		return;

	SPtr<RenderTexture> renderTexture;
	if(texture != nullptr)
		renderTexture = texture->GetNativeObjectAsShared();

	self->GetNativeObject()->SetRenderTexture(renderTexture);
}
