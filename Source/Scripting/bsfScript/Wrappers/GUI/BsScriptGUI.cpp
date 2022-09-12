//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUI.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "GUI/BsGUIWidget.h"
#include "Scene/BsSceneManager.h"
#include "BsScriptObjectManager.h"
#include "Resources/BsBuiltinResources.h"
#include "BsMonoMethod.h"

#include "Generated/BsScriptGUISkin.generated.h"

namespace bs
{
	SPtr<GUIWidget> ScriptGUI::sGUIWidget;
	ScriptGUILayout* ScriptGUI::sPanel = nullptr;
	HEvent ScriptGUI::sDomainUnloadConn;
	HEvent ScriptGUI::sDomainLoadConn;
	MonoMethod* ScriptGUI::sGUIPanelMethod = nullptr;

	ScriptGUI::ScriptGUI(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	ScriptGUI::~ScriptGUI()
	{ }

	void ScriptGUI::StartUp()
	{
		SPtr<Camera> mainCamera = gSceneManager().GetMainCamera();
		sGUIWidget = GUIWidget::create(mainCamera);
		sGUIWidget->SetSkin(BuiltinResources::instance().GetGUISkin());

		auto createPanel = [] ()
		{
			assert(sPanel == nullptr);

			MonoObject* guiPanel = ScriptGUIPanel::createFromExisting(sGUIWidget->GetPanel());
			sPanel = ScriptGUILayout::toNative(guiPanel);

			void* params[1];
			params[0] = guiPanel;

			sGUIPanelMethod->Invoke(nullptr, params);
		};

		auto clearPanel = [] ()
		{
			sPanel = nullptr;
		};

		createPanel();

		sDomainLoadConn = ScriptObjectManager::instance().onRefreshDomainLoaded.Connect(createPanel);
		sDomainUnloadConn = MonoManager::instance().onDomainUnload.Connect(clearPanel);
	}

	void ScriptGUI::Update()
	{
		if (sGUIWidget == nullptr)
			return;

		SPtr<Camera> mainCamera = gSceneManager().GetMainCamera();
		if (mainCamera != sGUIWidget->GetCamera())
			sGUIWidget->SetCamera(mainCamera);

		sGUIWidget->_updateRT();
	}

	void ScriptGUI::ShutDown()
	{
		sDomainLoadConn.Disconnect();
		sDomainUnloadConn.Disconnect();

		if (sPanel != nullptr)
		{
			sPanel->Destroy();
			sPanel = nullptr;
		}

		if (sGUIWidget != nullptr)
		{
			sGUIWidget->_destroy();
			sGUIWidget = nullptr;
		}
	}

	void ScriptGUI::InitRuntimeData()
	{
		sGUIPanelMethod = metaData.scriptClass->GetMethod("SetPanel", 1);

		metaData.scriptClass->AddInternalCall("Internal_SetSkin", (void*)&ScriptGUI::internal_SetSkin);
	}

	void ScriptGUI::internal_SetSkin(ScriptGUISkin* skin)
	{
		HGUISkin guiSkin;
		if (skin != nullptr)
			guiSkin = skin->GetHandle();

		if (!guiSkin.IsLoaded())
			guiSkin = BuiltinResources::instance().GetGUISkin();

		if(sGUIWidget != nullptr)
			sGUIWidget->SetSkin(guiSkin);
	}
}
