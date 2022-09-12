//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIWidget.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "GUI/BsGUIWidget.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"
#include "Resources/BsBuiltinResources.h"
#include "Components/BsCCamera.h"

#include "Generated/BsScriptCCamera.generated.h"
#include "Generated/BsScriptGUISkin.generated.h"

namespace bs
{
	MonoField* ScriptGUIWidget::sGUIPanelField = nullptr;

	ScriptGUIWidget::ScriptGUIWidget(MonoObject* managedInstance)
		:ScriptObject(managedInstance), mGUIWidget(nullptr)
	{
		SPtr<Camera> mainCamera = gSceneManager().GetMainCamera();

		mGUIWidget = GUIWidget::create(mainCamera);
		mGUIWidget->SetSkin(BuiltinResources::instance().GetGUISkin());

		MonoObject* guiPanel = ScriptGUIPanel::createFromExisting(mGUIWidget->GetPanel());
		mPanel = ScriptGUILayout::toNative(guiPanel);

		sGUIPanelField->Set(managedInstance, guiPanel);
	}

	ScriptGUIWidget::~ScriptGUIWidget()
	{ }

	void ScriptGUIWidget::InitRuntimeData()
	{
		sGUIPanelField = metaData.scriptClass->GetField("panel");

		metaData.scriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIWidget::internal_Create);
		metaData.scriptClass->AddInternalCall("Internal_UpdateTransform", (void*)&ScriptGUIWidget::internal_UpdateTransform);
		metaData.scriptClass->AddInternalCall("Internal_UpdateMainCamera", (void*)&ScriptGUIWidget::internal_UpdateMainCamera);
		metaData.scriptClass->AddInternalCall("Internal_SetSkin", (void*)&ScriptGUIWidget::internal_SetSkin);
		metaData.scriptClass->AddInternalCall("Internal_SetCamera", (void*)&ScriptGUIWidget::internal_SetCamera);
		metaData.scriptClass->AddInternalCall("Internal_SetDepth", (void*)&ScriptGUIWidget::internal_SetDepth);
		metaData.scriptClass->AddInternalCall("Internal_GetDepth", (void*)&ScriptGUIWidget::internal_GetDepth);
		metaData.scriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptGUIWidget::internal_Destroy);
	}

	void ScriptGUIWidget::internal_Create(MonoObject* managedInstance)
	{
		new (bs_alloc<ScriptGUIWidget>()) ScriptGUIWidget(managedInstance);
	}

	void ScriptGUIWidget::internal_UpdateTransform(ScriptGUIWidget* thisPtr, ScriptSceneObject* parent)
	{
		HSceneObject parentSO = parent->GetHandle();

		SPtr<GUIWidget> widget = thisPtr->GetInternal();
		if (!parentSO.IsDestroyed() && widget != nullptr)
		{
			widget->_updateTransform(parentSO);
			widget->_updateRT();

			if (parentSO->GetActive() != widget->getIsActive())
				widget->SetIsActive(parentSO->getActive());
		}
	}

	void ScriptGUIWidget::internal_UpdateMainCamera(ScriptGUIWidget* instance, ScriptCCamera* camera)
	{
		SPtr<GUIWidget> widget = instance->GetInternal();

		if (widget != nullptr)
		{
			SPtr<Camera> nativeCamera;
			if (camera != nullptr)
				nativeCamera = camera->GetHandle()->_getCamera();

			widget->SetCamera(nativeCamera);
		}
	}

	void ScriptGUIWidget::internal_SetSkin(ScriptGUIWidget* instance, ScriptGUISkin* skin)
	{
		HGUISkin guiSkin;
		if (skin != nullptr)
			guiSkin = skin->GetHandle();

		if (!guiSkin.IsLoaded())
			guiSkin = BuiltinResources::instance().GetGUISkin();

		SPtr<GUIWidget> widget = instance->GetInternal();
		if (widget != nullptr)
			widget->SetSkin(guiSkin);
	}

	void ScriptGUIWidget::internal_SetCamera(ScriptGUIWidget* instance, ScriptCCamera* camera)
	{
		SPtr<Camera> nativeCamera;
		if (camera != nullptr)
			nativeCamera = camera->GetHandle()->_getCamera();

		if(nativeCamera == nullptr)
			nativeCamera = gSceneManager().GetMainCamera();

		SPtr<GUIWidget> widget = instance->GetInternal();
		if(widget != nullptr)
			widget->SetCamera(nativeCamera);
	}

	void ScriptGUIWidget::internal_SetDepth(ScriptGUIWidget* instance, INT8 value)
	{
		instance->GetInternal()->setDepth(value);
	}

	INT8 ScriptGUIWidget::internal_GetDepth(ScriptGUIWidget* instance)
	{
		return instance->GetInternal()->getDepth();
	}

	void ScriptGUIWidget::internal_Destroy(ScriptGUIWidget* instance)
	{
		instance->Destroy();
	}

	void ScriptGUIWidget::Destroy(bool destroyPanel)
	{
		if(mPanel != nullptr && destroyPanel)
		{
			mPanel->Destroy();
			mPanel = nullptr;
		}

		if (mGUIWidget != nullptr)
		{
			mGUIWidget->_destroy();
			mGUIWidget = nullptr;
		}
	}

	void ScriptGUIWidget::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		destroy(false);

		ScriptObject::_onManagedInstanceDeleted(assemblyRefresh);
	}
}
