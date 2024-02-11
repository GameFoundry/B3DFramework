//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUI.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "GUI/BsGUIWidget.h"
#include "Scene/BsSceneManager.h"
#include "BsScriptObjectManager.h"
#include "Resources/BsBuiltinResources.h"
#include "BsMonoMethod.h"

using namespace bs;
SPtr<GUIWidget> ScriptGUI::sGUIWidget;
ScriptGUILayout* ScriptGUI::sPanel = nullptr;
HEvent ScriptGUI::sDomainUnloadConn;
HEvent ScriptGUI::sDomainLoadConn;
bs::MonoMethod* ScriptGUI::sGUIPanelMethod = nullptr;

ScriptGUI::ScriptGUI(MonoObject* managedInstance)
	: ScriptObject(managedInstance)
{}

ScriptGUI::~ScriptGUI()
{}

void ScriptGUI::StartUp()
{
	SPtr<Camera> mainCamera = GetSceneManager().GetMainCamera();
	sGUIWidget = GUIWidget::Create(mainCamera);

	auto createPanel = []()
	{
		B3D_ASSERT(sPanel == nullptr);

		MonoObject* guiPanel = ScriptGUIPanel::CreateFromExisting(sGUIWidget->GetPanel());
		sPanel = ScriptGUILayout::ToNative(guiPanel);

		void* params[1];
		params[0] = guiPanel;

		sGUIPanelMethod->Invoke(nullptr, params);
	};

	auto clearPanel = []()
	{
		sPanel = nullptr;
	};

	createPanel();

	sDomainLoadConn = ScriptObjectManager::Instance().OnRefreshDomainLoaded.Connect(createPanel);
	sDomainUnloadConn = MonoManager::Instance().OnDomainUnload.Connect(clearPanel);
}

void ScriptGUI::Update()
{
	if(sGUIWidget == nullptr)
		return;

	SPtr<Camera> mainCamera = GetSceneManager().GetMainCamera();
	if(mainCamera != sGUIWidget->GetCamera())
		sGUIWidget->SetCamera(mainCamera);

	sGUIWidget->UpdateRTInternal();
}

void ScriptGUI::ShutDown()
{
	sDomainLoadConn.Disconnect();
	sDomainUnloadConn.Disconnect();

	if(sPanel != nullptr)
	{
		sPanel->Destroy();
		sPanel = nullptr;
	}

	if(sGUIWidget != nullptr)
	{
		sGUIWidget->DestroyInternal();
		sGUIWidget = nullptr;
	}
}

void ScriptGUI::InitRuntimeData()
{
	sGUIPanelMethod = metaData.ScriptClass->GetMethod("SetPanel", 1);
}
