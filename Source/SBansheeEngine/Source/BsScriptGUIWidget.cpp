//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIWidget.h"
#include "BsScriptSceneObject.h"
#include "BsScriptGUISkin.h"
#include "BsScriptCamera.h"
#include "BsScriptGUILayout.h"
#include "BsGUIWidget.h"
#include "BsSceneObject.h"
#include "BsSceneManager.h"
#include "BsBuiltinResources.h"

namespace bs
{
	MonoField* ScriptGUIWidget::sGUIPanelField = nullptr;

	ScriptGUIWidget::ScriptGUIWidget(MonoObject* managedInstance)
		:ScriptObject(managedInstance), mGUIWidget(nullptr), mLastUpdateHash((UINT32)-1)
	{
		SPtr<Camera> mainCamera = gSceneManager().getMainCamera().camera;

		mGUIWidget = GUIWidget::create(mainCamera);
		mGUIWidget->setSkin(BuiltinResources::instance().getGUISkin());

		MonoObject* guiPanel = ScriptGUIPanel::createFromExisting(mGUIWidget->getPanel());
		mPanel = ScriptGUILayout::toNative(guiPanel);

		sGUIPanelField->setValue(managedInstance, guiPanel);
	}

	ScriptGUIWidget::~ScriptGUIWidget()
	{ }

	void ScriptGUIWidget::initRuntimeData()
	{
		sGUIPanelField = metaData.scriptClass->getField("panel");

		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptGUIWidget::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_UpdateTransform", &ScriptGUIWidget::internal_UpdateTransform);
		metaData.scriptClass->addInternalCall("Internal_UpdateMainCamera", &ScriptGUIWidget::internal_UpdateMainCamera);
		metaData.scriptClass->addInternalCall("Internal_SetSkin", &ScriptGUIWidget::internal_SetSkin);
		metaData.scriptClass->addInternalCall("Internal_SetCamera", &ScriptGUIWidget::internal_SetCamera);
		metaData.scriptClass->addInternalCall("Internal_SetDepth", &ScriptGUIWidget::internal_SetDepth);
		metaData.scriptClass->addInternalCall("Internal_GetDepth", &ScriptGUIWidget::internal_GetDepth);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptGUIWidget::internal_Destroy);
	}

	void ScriptGUIWidget::internal_Create(MonoObject* managedInstance)
	{
		new (bs_alloc<ScriptGUIWidget>()) ScriptGUIWidget(managedInstance);
	}

	void ScriptGUIWidget::internal_UpdateTransform(ScriptGUIWidget* thisPtr, ScriptSceneObject* parent)
	{
		HSceneObject parentSO = parent->getNativeSceneObject();

		SPtr<GUIWidget> widget = thisPtr->getInternal();
		if (!parentSO.isDestroyed() && widget != nullptr)
		{
			widget->_updateTransform(parentSO);
			widget->_updateRT();

			if (parentSO->getActive() != widget->getIsActive())
				widget->setIsActive(parentSO->getActive());
		}
	}

	void ScriptGUIWidget::internal_UpdateMainCamera(ScriptGUIWidget* instance, ScriptCamera* camera)
	{
		SPtr<GUIWidget> widget = instance->getInternal();

		if (widget != nullptr)
		{
			SPtr<Camera> nativeCamera;
			if (camera != nullptr)
				nativeCamera = camera->getInternal();

			widget->setCamera(nativeCamera);
		}
	}

	void ScriptGUIWidget::internal_SetSkin(ScriptGUIWidget* instance, ScriptGUISkin* skin)
	{
		HGUISkin guiSkin;
		if (skin != nullptr)
			guiSkin = skin->getHandle();

		if (!guiSkin.isLoaded())
			guiSkin = BuiltinResources::instance().getGUISkin();

		SPtr<GUIWidget> widget = instance->getInternal();
		if (widget != nullptr)
			widget->setSkin(guiSkin);
	}

	void ScriptGUIWidget::internal_SetCamera(ScriptGUIWidget* instance, ScriptCamera* camera)
	{
		SPtr<Camera> nativeCamera;
		if (camera != nullptr)
			nativeCamera = camera->getInternal();

		if(nativeCamera == nullptr)
			nativeCamera = gSceneManager().getMainCamera().camera;

		SPtr<GUIWidget> widget = instance->getInternal();
		if(widget != nullptr)
			widget->setCamera(nativeCamera);
	}

	void ScriptGUIWidget::internal_SetDepth(ScriptGUIWidget* instance, INT8 value)
	{
		instance->getInternal()->setDepth(value);
	}

	INT8 ScriptGUIWidget::internal_GetDepth(ScriptGUIWidget* instance)
	{
		return instance->getInternal()->getDepth();
	}

	void ScriptGUIWidget::internal_Destroy(ScriptGUIWidget* instance)
	{
		instance->destroy();
	}

	void ScriptGUIWidget::destroy()
	{
		if(mPanel != nullptr)
		{
			mPanel->destroy();
			mPanel = nullptr;
		}

		if (mGUIWidget != nullptr)
		{
			mGUIWidget->_destroy();
			mGUIWidget = nullptr;
		}
	}

	void ScriptGUIWidget::_onManagedInstanceDeleted()
	{
		destroy();

		ScriptObject::_onManagedInstanceDeleted();
	}
}