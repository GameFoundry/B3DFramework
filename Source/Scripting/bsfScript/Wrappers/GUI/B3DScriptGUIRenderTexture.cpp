//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/B3DScriptGUIRenderTexture.h"
#include "B3DMonoField.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "Image/B3DSpriteTexture.h"
#include "B3DMonoUtil.h"
#include "GUI/B3DGUILayout.h"
#include "GUI/B3DGUIRenderTexture.h"
#include "GUI/B3DGUIOptions.h"

#include "Generated/B3DScriptHString.generated.h"
#include "Generated/B3DScriptGUIContent.generated.h"
#include "Generated/B3DScriptRenderTexture.generated.h"

using namespace b3d;
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

	TShared<RenderTexture> renderTexture;
	if(texture != nullptr)
		renderTexture = texture->GetNativeObjectAsShared();

	GUIRenderTexture* guiTexture = GUIRenderTexture::Create(renderTexture, transparent, options, MonoUtil::MonoToString(style));

	ScriptObjectWrapper::Create<ScriptGUIRenderTexture>(guiTexture, instance);
}

void ScriptGUIRenderTexture::InternalSetTexture(ScriptGUIRenderTexture* self, ScriptRenderTexture* texture)
{
	if(!self->IsNativeObjectValid())
		return;

	TShared<RenderTexture> renderTexture;
	if(texture != nullptr)
		renderTexture = texture->GetNativeObjectAsShared();

	self->GetNativeObject()->SetRenderTexture(renderTexture);
}
