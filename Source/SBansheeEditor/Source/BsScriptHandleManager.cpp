//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHandleManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsSelection.h"
#include "BsSceneObject.h"
#include "BsManagedComponent.h"
#include "BsScriptObjectManager.h"

using namespace std::placeholders;

namespace bs
{
	ScriptHandleManager::ScriptHandleManager(ScriptAssemblyManager& scriptObjectManager)
		:mScriptObjectManager(scriptObjectManager)
	{
		mDomainLoadConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(std::bind(&ScriptHandleManager::loadAssemblyData, this));
		mDomainUnloadConn = MonoManager::instance().onDomainUnload.connect(std::bind(&ScriptHandleManager::clearAssemblyData, this));
		loadAssemblyData();
	}

	ScriptHandleManager::~ScriptHandleManager()
	{
		clearAssemblyData();

		mDomainUnloadConn.disconnect();
		mDomainLoadConn.disconnect();
	}

	void ScriptHandleManager::triggerPreInput()
	{
		// Activate global handles
		for(auto& handle : mGlobalHandlesToCreate)
		{
			MonoObject* newHandleInstance = handle->createInstance();

			mActiveGlobalHandles.push_back(ActiveCustomHandleData());
			ActiveCustomHandleData& newHandleData = mActiveGlobalHandles.back();

			newHandleData.object = newHandleInstance;
			newHandleData.gcHandle = MonoUtil::newGCHandle(newHandleInstance);
		}

		mGlobalHandlesToCreate.clear();

		// Activate object-specific handles
		const Vector<HSceneObject>& selectedSOs = Selection::instance().getSceneObjects();

		HSceneObject newSelectedObject;
		if (selectedSOs.size() > 0)
		{
			// We only consider the first selected object for custom handles, multiple selection is not supported
			newSelectedObject = selectedSOs[0]; 
		}

		if (newSelectedObject != mActiveHandleData.selectedObject)
		{
			for (auto& handle : mActiveHandleData.handles)
			{
				callDestroy(handle.object);
				MonoUtil::freeGCHandle(handle.gcHandle);
			}

			mActiveHandleData.selectedObject = newSelectedObject;
			mActiveHandleData.handles.clear();

			if (newSelectedObject)
			{
				const Vector<HComponent>& components = newSelectedObject->getComponents();

				for (auto& component : components)
				{
					if (!rtti_is_of_type<ManagedComponent>(component.getInternalPtr()))
						continue;

					HManagedComponent mc = static_object_cast<ManagedComponent>(component);
					const String& componentTypeName = mc->getManagedFullTypeName();

					auto iterFind = mHandles.find(componentTypeName);
					if (iterFind == mHandles.end())
						continue;

					CustomHandleData& handleData = iterFind->second;
					MonoObject* newHandleInstance = handleData.handleType->createInstance(false);

					void* params[1] = { mc->getManagedInstance() };
					handleData.ctor->invoke(newHandleInstance, params);

					mActiveHandleData.handles.push_back(ActiveCustomHandleData());
					ActiveCustomHandleData& newHandleData = mActiveHandleData.handles.back();

					newHandleData.object = newHandleInstance;
					newHandleData.gcHandle = MonoUtil::newGCHandle(newHandleInstance);
				}
			}
		}

		if (mDefaultHandleManager == nullptr)
		{
			mDefaultHandleManager = mDefaultHandleManagerClass->createInstance(true);
			mDefaultHandleManagerGCHandle = MonoUtil::newGCHandle(mDefaultHandleManager);
		}

		callPreInput(mDefaultHandleManager);

		for (auto& handle : mActiveGlobalHandles)
			callPreInput(handle.object);

		for (auto& handle : mActiveHandleData.handles)
			callPreInput(handle.object);
	}

	void ScriptHandleManager::triggerPostInput()
	{
		callPostInput(mDefaultHandleManager);

		for (auto& handle : mActiveGlobalHandles)
			callPostInput(handle.object);

		for (auto& handle : mActiveHandleData.handles)
			callPostInput(handle.object);
	}

	void ScriptHandleManager::queueDrawCommands()
	{
		if (mDefaultHandleManager != nullptr)
			callDraw(mDefaultHandleManager);

		for (auto& handle : mActiveGlobalHandles)
			callDraw(handle.object);

		for (auto& handle : mActiveHandleData.handles)
			callDraw(handle.object);
	}

	void ScriptHandleManager::clearAssemblyData()
	{
		mGlobalHandlesToCreate.clear();

		for (auto& handle : mActiveGlobalHandles)
		{
			callDestroy(handle.object);
			MonoUtil::freeGCHandle(handle.gcHandle);
		}

		mActiveGlobalHandles.clear();

		for (auto& handle : mActiveHandleData.handles)
		{
			callDestroy(handle.object);
			MonoUtil::freeGCHandle(handle.gcHandle);
		}

		mActiveHandleData.selectedObject = HSceneObject();
		mActiveHandleData.handles.clear();

		if (mDefaultHandleManager != nullptr)
		{
			callDestroy(mDefaultHandleManager);
			MonoUtil::freeGCHandle(mDefaultHandleManagerGCHandle);
		}

		mDefaultHandleManager = nullptr;
		mDefaultHandleManagerGCHandle = 0;
	}

	void ScriptHandleManager::loadAssemblyData()
	{
		MonoAssembly* editorAssembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);
		mCustomHandleAttribute = editorAssembly->getClass("BansheeEditor", "CustomHandle");
		if (mCustomHandleAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find CustomHandle managed class.");

		mHandleBaseClass = editorAssembly->getClass("BansheeEditor", "Handle");
		if (mHandleBaseClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Handle managed class.");

		mDefaultHandleManagerClass = editorAssembly->getClass("BansheeEditor", "DefaultHandleManager");
		if (mDefaultHandleManagerClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find DefaultHandleManager managed class.");

		mTypeField = mCustomHandleAttribute->getField("type");

		mPreInputMethod = mHandleBaseClass->getMethod("PreInput", 0);
		mPostInputMethod = mHandleBaseClass->getMethod("PostInput", 0);
		mDrawMethod = mHandleBaseClass->getMethod("Draw", 0);
		mDestroyThunk = (DestroyThunkDef)mHandleBaseClass->getMethod("Destroy", 0)->getThunk();

		Vector<String> scriptAssemblyNames = mScriptObjectManager.getScriptAssemblies();
		for (auto& assemblyName : scriptAssemblyNames)
		{
			MonoAssembly* assembly = MonoManager::instance().getAssembly(assemblyName);

			// Find new custom handle types
			const Vector<MonoClass*>& allClasses = assembly->getAllClasses();
			for (auto curClass : allClasses)
			{
				MonoClass* componentType = nullptr;
				MonoMethod* ctor = nullptr;

				if (isValidHandleType(curClass, componentType, ctor))
				{
					if (componentType != nullptr)
					{
						String fullComponentName = componentType->getFullName();
						CustomHandleData& newHandleData = mHandles[fullComponentName];

						newHandleData.componentType = componentType;
						newHandleData.handleType = curClass;
						newHandleData.ctor = ctor;
					}
					else // Global handle
					{
						mGlobalHandlesToCreate.push_back(curClass);
					}
				}
			}
		}
	}

	bool ScriptHandleManager::isValidHandleType(MonoClass* type, MonoClass*& componentType, MonoMethod*& ctor) const
	{
		componentType = nullptr;
		ctor = nullptr;

		if (!type->hasAttribute(mCustomHandleAttribute))
			return false;

		if (!type->isSubClassOf(mHandleBaseClass))
			return false;

		MonoObject* customHandleAttrib = type->getAttribute(mCustomHandleAttribute);
		MonoReflectionType* attribReflType = nullptr;

		mTypeField->getValue(customHandleAttrib, &attribReflType);

		// Handle shown only when specific component type is selected
		if (attribReflType != nullptr)
		{
			::MonoClass* attribMonoClass = MonoUtil::getClass(attribReflType);

			MonoClass* attribClass = MonoManager::instance().findClass(attribMonoClass);
			if (attribClass != nullptr)
				return false;

			MonoClass* componentClass = mScriptObjectManager.getComponentClass();
			if (!attribClass->isSubClassOf(componentClass))
				return false;

			MonoMethod* constructor = type->getMethod(".ctor", 1);
			if (constructor == nullptr)
				return false;

			MonoClass* paramType = constructor->getParameterType(0);
			if (paramType != attribClass)
				return false;

			componentType = paramType;
			ctor = constructor;
		}
		else // Handle shown always
		{
			// Do nothing
		}

		return true;
	}

	void ScriptHandleManager::callPreInput(MonoObject* instance)
	{
		mPreInputMethod->invokeVirtual(instance, nullptr);
	}

	void ScriptHandleManager::callPostInput(MonoObject* instance)
	{
		mPostInputMethod->invokeVirtual(instance, nullptr);
	}

	void ScriptHandleManager::callDraw(MonoObject* instance)
	{
		mDrawMethod->invokeVirtual(instance, nullptr);
	}

	void ScriptHandleManager::callDestroy(MonoObject* instance)
	{
		MonoUtil::invokeThunk(mDestroyThunk, instance);
	}
}