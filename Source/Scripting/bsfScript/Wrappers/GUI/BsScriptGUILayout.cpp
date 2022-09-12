//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Wrappers/GUI/BsScriptGUIScrollArea.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUIScrollArea.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptGUILayout::ScriptGUILayout(MonoObject* instance, GUILayout* layout, bool ownsNative)
		:TScriptGUIElementBase(instance, layout), mLayout(layout), mOwnsNative(ownsNative)
	{ }

	void ScriptGUILayout::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstanceX", (void*)&ScriptGUILayout::internal_createInstanceX);
		metaData.scriptClass->AddInternalCall("Internal_CreateInstanceY", (void*)&ScriptGUILayout::internal_createInstanceY);
		metaData.scriptClass->AddInternalCall("Internal_CreateInstancePanel", (void*)&ScriptGUILayout::internal_createInstancePanel);
		metaData.scriptClass->AddInternalCall("Internal_CreateInstanceYFromScrollArea", (void*)&ScriptGUILayout::internal_createInstanceYFromScrollArea);
		metaData.scriptClass->AddInternalCall("Internal_AddElement", (void*)&ScriptGUILayout::internal_addElement);
		metaData.scriptClass->AddInternalCall("Internal_InsertElement", (void*)&ScriptGUILayout::internal_insertElement);
		metaData.scriptClass->AddInternalCall("Internal_GetChildCount", (void*)&ScriptGUILayout::internal_getChildCount);
		metaData.scriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptGUILayout::internal_getChild);
		metaData.scriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUILayout::internal_clear);
	}

	void ScriptGUILayout::Destroy()
	{
		if(!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->RemoveChild(this);

			while (mChildren.size() > 0)
			{
				ChildInfo childInfo = mChildren[0];
				childInfo.element->Destroy();
			}

			if (mOwnsNative)
				GUILayout::destroy(mLayout);

			mLayout = nullptr;
			mIsDestroyed = true;
		}
	}

	void ScriptGUILayout::AddChild(ScriptGUIElementBaseTBase* element)
	{
		ChildInfo childInfo;

		childInfo.element = element;
		childInfo.gcHandle = MonoUtil::newGCHandle(element->GetManagedInstance(), false);

		mChildren.push_back(childInfo);
	}

	void ScriptGUILayout::InsertChild(UINT32 idx, ScriptGUIElementBaseTBase* element)
	{
		ChildInfo childInfo;

		childInfo.element = element;
		childInfo.gcHandle = MonoUtil::newGCHandle(element->GetManagedInstance(), false);

		mChildren.Insert(mChildren.begin() + idx, childInfo);
	}

	void ScriptGUILayout::RemoveChild(ScriptGUIElementBaseTBase* element)
	{
		auto iterFind = std::find_if(mChildren.begin(), mChildren.end(),
			[&](const ChildInfo& x)
		{
			return x.element == element;
		});

		if (iterFind != mChildren.end())
		{
			assert(iterFind->gcHandle != 0);

			MonoUtil::freeGCHandle(iterFind->gcHandle);
			iterFind->gcHandle = 0;

			mChildren.erase(iterFind);
		}
	}

	void ScriptGUILayout::internal_createInstanceX(MonoObject* instance, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUILayout* layout = GUILayoutX::create(options);

		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::internal_createInstanceY(MonoObject* instance, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUILayout* layout = GUILayoutY::create(options);

		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::internal_createInstancePanel(MonoObject* instance, INT16 depth, UINT16 depthRangeMin, UINT32 depthRangeMax, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUILayout* layout = GUIPanel::create(depth, depthRangeMin, depthRangeMax, options);

		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::internal_createInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea)
	{
		ScriptGUIScrollArea* scriptScrollArea = ScriptGUIScrollArea::toNative(parentScrollArea);
		GUIScrollArea* scrollArea = (GUIScrollArea*)scriptScrollArea->GetGUIElement();

		GUILayout* nativeLayout = &scrollArea->GetLayout();

		ScriptGUIScrollAreaLayout* nativeInstance = new (bs_alloc<ScriptGUIScrollAreaLayout>())
			ScriptGUIScrollAreaLayout(instance, nativeLayout);

		// This method is expected to be called during GUIScrollArea construction, so we finish its initialization
		scriptScrollArea->Initialize(nativeInstance);
	}

	void ScriptGUILayout::internal_addElement(ScriptGUILayout* instance, ScriptGUIElementBaseTBase* element)
	{
		if (instance->IsDestroyed() || element->isDestroyed())
			return;

		instance->GetInternalValue()->addElement(element->getGUIElement());

		if (element->GetParent() != nullptr)
			element->GetParent()->removeChild(element);

		element->SetParent(instance);
		instance->AddChild(element);
	}

	void ScriptGUILayout::internal_insertElement(ScriptGUILayout* instance, UINT32 index, ScriptGUIElementBaseTBase* element)
	{
		if (instance->IsDestroyed() || element->isDestroyed())
			return;

		instance->GetInternalValue()->insertElement(index, element->getGUIElement());

		if (element->GetParent() != nullptr)
			element->GetParent()->removeChild(element);

		element->SetParent(instance);
		instance->InsertChild(index, element);
	}

	UINT32 ScriptGUILayout::internal_getChildCount(ScriptGUILayout* instance)
	{
		if (instance->IsDestroyed())
			return 0;

		return instance->mLayout->GetNumChildren();
	}

	MonoObject* ScriptGUILayout::internal_getChild(ScriptGUILayout* instance, UINT32 index)
	{
		if (instance->IsDestroyed() || index >= instance->mChildren.size())
			return nullptr;

		return instance->mChildren[index].element->GetManagedInstance();
	}

	void ScriptGUILayout::internal_clear(ScriptGUILayout* instance)
	{
		if (instance->IsDestroyed())
			return;

		for (auto& child : instance->mChildren)
		{
			instance->GetInternalValue()->removeElement(child.element->getGUIElement());

			assert(child.gcHandle != 0);

			MonoUtil::freeGCHandle(child.gcHandle);
			child.gcHandle = 0;

			child.element->SetParent(nullptr);
		}

		instance->mChildren.Clear();
	}

	ScriptGUIPanel::ScriptGUIPanel(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptGUIPanel::InitRuntimeData()
	{ }

	MonoObject* ScriptGUIPanel::createFromExisting(GUIPanel* panel)
	{
		MonoObject* managedInstance = metaData.scriptClass->CreateInstance();
		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(managedInstance, panel, false);

		return managedInstance;
	}

	ScriptGUIScrollAreaLayout::ScriptGUIScrollAreaLayout(MonoObject* instance, GUILayout* layout)
		:ScriptGUILayout(instance, layout, false), mParentScrollArea(nullptr)
	{
		
	}

	void ScriptGUIScrollAreaLayout::Destroy()
	{
		if (!mIsDestroyed)
		{
			if (mParentScrollArea != nullptr)
				mParentScrollArea->NotifyLayoutDestroyed();

			while (mChildren.size() > 0)
			{
				ChildInfo childInfo = mChildren[0];
				childInfo.element->Destroy();
			}

			mLayout = nullptr;
			mIsDestroyed = true;
		}
	}
}
