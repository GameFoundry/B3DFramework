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
		: TScriptGUIElementBase(instance, layout), mLayout(layout), mOwnsNative(ownsNative)
	{}

	void ScriptGUILayout::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstanceX", (void*)&ScriptGUILayout::InternalCreateInstanceX);
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstanceY", (void*)&ScriptGUILayout::InternalCreateInstanceY);
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstancePanel", (void*)&ScriptGUILayout::InternalCreateInstancePanel);
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstanceYFromScrollArea", (void*)&ScriptGUILayout::InternalCreateInstanceYFromScrollArea);
		metaData.ScriptClass->AddInternalCall("Internal_AddElement", (void*)&ScriptGUILayout::InternalAddElement);
		metaData.ScriptClass->AddInternalCall("Internal_InsertElement", (void*)&ScriptGUILayout::InternalInsertElement);
		metaData.ScriptClass->AddInternalCall("Internal_GetChildCount", (void*)&ScriptGUILayout::InternalGetChildCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptGUILayout::InternalGetChild);
		metaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUILayout::InternalClear);
	}

	void ScriptGUILayout::Destroy()
	{
		if(!mIsDestroyed)
		{
			if(mParent != nullptr)
				mParent->RemoveChild(this);

			while(mChildren.size() > 0)
			{
				ChildInfo childInfo = mChildren[0];
				childInfo.Element->Destroy();
			}

			if(mOwnsNative)
				GUILayout::Destroy(mLayout);

			mLayout = nullptr;
			mIsDestroyed = true;
		}
	}

	void ScriptGUILayout::AddChild(ScriptGUIElementBaseTBase* element)
	{
		ChildInfo childInfo;

		childInfo.Element = element;
		childInfo.GcHandle = MonoUtil::NewGcHandle(element->GetManagedInstance(), false);

		mChildren.push_back(childInfo);
	}

	void ScriptGUILayout::InsertChild(u32 idx, ScriptGUIElementBaseTBase* element)
	{
		ChildInfo childInfo;

		childInfo.Element = element;
		childInfo.GcHandle = MonoUtil::NewGcHandle(element->GetManagedInstance(), false);

		mChildren.insert(mChildren.begin() + idx, childInfo);
	}

	void ScriptGUILayout::RemoveChild(ScriptGUIElementBaseTBase* element)
	{
		auto iterFind = std::find_if(mChildren.begin(), mChildren.end(), [&](const ChildInfo& x)
									 { return x.Element == element; });

		if(iterFind != mChildren.end())
		{
			assert(iterFind->GcHandle != 0);

			MonoUtil::FreeGcHandle(iterFind->GcHandle);
			iterFind->GcHandle = 0;

			mChildren.erase(iterFind);
		}
	}

	void ScriptGUILayout::InternalCreateInstanceX(MonoObject* instance, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		GUILayout* layout = GUILayoutX::Create(options);

		new(bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::InternalCreateInstanceY(MonoObject* instance, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		GUILayout* layout = GUILayoutY::Create(options);

		new(bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::InternalCreateInstancePanel(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		GUILayout* layout = GUIPanel::Create(depth, depthRangeMin, depthRangeMax, options);

		new(bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::InternalCreateInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea)
	{
		ScriptGUIScrollArea* scriptScrollArea = ScriptGUIScrollArea::ToNative(parentScrollArea);
		GUIScrollArea* scrollArea = (GUIScrollArea*)scriptScrollArea->GetGuiElement();

		GUILayout* nativeLayout = &scrollArea->GetLayout();

		ScriptGUIScrollAreaLayout* nativeInstance = new(bs_alloc<ScriptGUIScrollAreaLayout>())
			ScriptGUIScrollAreaLayout(instance, nativeLayout);

		// This method is expected to be called during GUIScrollArea construction, so we finish its initialization
		scriptScrollArea->Initialize(nativeInstance);
	}

	void ScriptGUILayout::InternalAddElement(ScriptGUILayout* instance, ScriptGUIElementBaseTBase* element)
	{
		if(instance->IsDestroyed() || element->IsDestroyed())
			return;

		instance->GetInternalValue()->AddElement(element->GetGuiElement());

		if(element->GetParent() != nullptr)
			element->GetParent()->RemoveChild(element);

		element->SetParent(instance);
		instance->AddChild(element);
	}

	void ScriptGUILayout::InternalInsertElement(ScriptGUILayout* instance, u32 index, ScriptGUIElementBaseTBase* element)
	{
		if(instance->IsDestroyed() || element->IsDestroyed())
			return;

		instance->GetInternalValue()->InsertElement(index, element->GetGuiElement());

		if(element->GetParent() != nullptr)
			element->GetParent()->RemoveChild(element);

		element->SetParent(instance);
		instance->InsertChild(index, element);
	}

	u32 ScriptGUILayout::InternalGetChildCount(ScriptGUILayout* instance)
	{
		if(instance->IsDestroyed())
			return 0;

		return instance->mLayout->GetNumChildren();
	}

	MonoObject* ScriptGUILayout::InternalGetChild(ScriptGUILayout* instance, u32 index)
	{
		if(instance->IsDestroyed() || index >= instance->mChildren.size())
			return nullptr;

		return instance->mChildren[index].Element->GetManagedInstance();
	}

	void ScriptGUILayout::InternalClear(ScriptGUILayout* instance)
	{
		if(instance->IsDestroyed())
			return;

		for(auto& child : instance->mChildren)
		{
			instance->GetInternalValue()->RemoveElement(child.Element->GetGuiElement());

			assert(child.GcHandle != 0);

			MonoUtil::FreeGcHandle(child.GcHandle);
			child.GcHandle = 0;

			child.Element->SetParent(nullptr);
		}

		instance->mChildren.clear();
	}

	ScriptGUIPanel::ScriptGUIPanel(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptGUIPanel::InitRuntimeData()
	{}

	MonoObject* ScriptGUIPanel::CreateFromExisting(GUIPanel* panel)
	{
		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance();
		new(bs_alloc<ScriptGUILayout>()) ScriptGUILayout(managedInstance, panel, false);

		return managedInstance;
	}

	ScriptGUIScrollAreaLayout::ScriptGUIScrollAreaLayout(MonoObject* instance, GUILayout* layout)
		: ScriptGUILayout(instance, layout, false), mParentScrollArea(nullptr)
	{
	}

	void ScriptGUIScrollAreaLayout::Destroy()
	{
		if(!mIsDestroyed)
		{
			if(mParentScrollArea != nullptr)
				mParentScrollArea->NotifyLayoutDestroyed();

			while(mChildren.size() > 0)
			{
				ChildInfo childInfo = mChildren[0];
				childInfo.Element->Destroy();
			}

			mLayout = nullptr;
			mIsDestroyed = true;
		}
	}
} // namespace bs
