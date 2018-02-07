//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	void ScriptGUILayout::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstanceX", (void*)&ScriptGUILayout::internal_createInstanceX);
		metaData.scriptClass->addInternalCall("Internal_CreateInstanceY", (void*)&ScriptGUILayout::internal_createInstanceY);
		metaData.scriptClass->addInternalCall("Internal_CreateInstancePanel", (void*)&ScriptGUILayout::internal_createInstancePanel);
		metaData.scriptClass->addInternalCall("Internal_CreateInstanceYFromScrollArea", (void*)&ScriptGUILayout::internal_createInstanceYFromScrollArea);
		metaData.scriptClass->addInternalCall("Internal_AddElement", (void*)&ScriptGUILayout::internal_addElement);
		metaData.scriptClass->addInternalCall("Internal_InsertElement", (void*)&ScriptGUILayout::internal_insertElement);
		metaData.scriptClass->addInternalCall("Internal_GetChildCount", (void*)&ScriptGUILayout::internal_getChildCount);
		metaData.scriptClass->addInternalCall("Internal_GetChild", (void*)&ScriptGUILayout::internal_getChild);
		metaData.scriptClass->addInternalCall("Internal_Clear", (void*)&ScriptGUILayout::internal_clear);
	}

	void ScriptGUILayout::destroy()
	{
		if(!mIsDestroyed)
		{
			if (mParent != nullptr)
				mParent->removeChild(this);

			while (mChildren.size() > 0)
			{
				ChildInfo childInfo = mChildren[0];
				childInfo.element->destroy();
			}

			if (mOwnsNative)
				GUILayout::destroy(mLayout);

			mLayout = nullptr;
			mIsDestroyed = true;
		}
	}

	void ScriptGUILayout::addChild(ScriptGUIElementBaseTBase* element)
	{
		ChildInfo childInfo;

		childInfo.element = element;
		childInfo.gcHandle = MonoUtil::newGCHandle(element->getManagedInstance(), false);

		mChildren.push_back(childInfo);
	}

	void ScriptGUILayout::insertChild(UINT32 idx, ScriptGUIElementBaseTBase* element)
	{
		ChildInfo childInfo;

		childInfo.element = element;
		childInfo.gcHandle = MonoUtil::newGCHandle(element->getManagedInstance(), false);

		mChildren.insert(mChildren.begin() + idx, childInfo);
	}

	void ScriptGUILayout::removeChild(ScriptGUIElementBaseTBase* element)
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

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUILayout* layout = GUILayoutX::create(options);

		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::internal_createInstanceY(MonoObject* instance, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUILayout* layout = GUILayoutY::create(options);

		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::internal_createInstancePanel(MonoObject* instance, INT16 depth, UINT16 depthRangeMin, UINT32 depthRangeMax, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUILayout* layout = GUIPanel::create(depth, depthRangeMin, depthRangeMax, options);

		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(instance, layout);
	}

	void ScriptGUILayout::internal_createInstanceYFromScrollArea(MonoObject* instance, MonoObject* parentScrollArea)
	{
		ScriptGUIScrollArea* scriptScrollArea = ScriptGUIScrollArea::toNative(parentScrollArea);
		GUIScrollArea* scrollArea = (GUIScrollArea*)scriptScrollArea->getGUIElement();

		GUILayout* nativeLayout = &scrollArea->getLayout();

		ScriptGUIScrollAreaLayout* nativeInstance = new (bs_alloc<ScriptGUIScrollAreaLayout>())
			ScriptGUIScrollAreaLayout(instance, nativeLayout);

		// This method is expected to be called during GUIScrollArea construction, so we finish its initialization
		scriptScrollArea->initialize(nativeInstance);
	}

	void ScriptGUILayout::internal_addElement(ScriptGUILayout* instance, ScriptGUIElementBaseTBase* element)
	{
		if (instance->isDestroyed() || element->isDestroyed())
			return;

		instance->getInternalValue()->addElement(element->getGUIElement());

		if (element->getParent() != nullptr)
			element->getParent()->removeChild(element);

		element->setParent(instance);
		instance->addChild(element);
	}

	void ScriptGUILayout::internal_insertElement(ScriptGUILayout* instance, UINT32 index, ScriptGUIElementBaseTBase* element)
	{
		if (instance->isDestroyed() || element->isDestroyed())
			return;

		instance->getInternalValue()->insertElement(index, element->getGUIElement());

		if (element->getParent() != nullptr)
			element->getParent()->removeChild(element);

		element->setParent(instance);
		instance->insertChild(index, element);
	}

	UINT32 ScriptGUILayout::internal_getChildCount(ScriptGUILayout* instance)
	{
		if (instance->isDestroyed())
			return 0;

		return instance->mLayout->getNumChildren();
	}

	MonoObject* ScriptGUILayout::internal_getChild(ScriptGUILayout* instance, UINT32 index)
	{
		if (instance->isDestroyed() || index >= instance->mChildren.size())
			return nullptr;

		return instance->mChildren[index].element->getManagedInstance();
	}

	void ScriptGUILayout::internal_clear(ScriptGUILayout* instance)
	{
		if (instance->isDestroyed())
			return;

		for (auto& child : instance->mChildren)
		{
			instance->getInternalValue()->removeElement(child.element->getGUIElement());

			assert(child.gcHandle != 0);

			MonoUtil::freeGCHandle(child.gcHandle);
			child.gcHandle = 0;

			child.element->setParent(nullptr);
		}

		instance->mChildren.clear();
	}

	ScriptGUIPanel::ScriptGUIPanel(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptGUIPanel::initRuntimeData()
	{ }

	MonoObject* ScriptGUIPanel::createFromExisting(GUIPanel* panel)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptGUILayout>()) ScriptGUILayout(managedInstance, panel, false);

		return managedInstance;
	}

	ScriptGUIScrollAreaLayout::ScriptGUIScrollAreaLayout(MonoObject* instance, GUILayout* layout)
		:ScriptGUILayout(instance, layout, false), mParentScrollArea(nullptr)
	{
		
	}

	void ScriptGUIScrollAreaLayout::destroy()
	{
		if (!mIsDestroyed)
		{
			if (mParentScrollArea != nullptr)
				mParentScrollArea->notifyLayoutDestroyed();

			while (mChildren.size() > 0)
			{
				ChildInfo childInfo = mChildren[0];
				childInfo.element->destroy();
			}

			mLayout = nullptr;
			mIsDestroyed = true;
		}
	}
}