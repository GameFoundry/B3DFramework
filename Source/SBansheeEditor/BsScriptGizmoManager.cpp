//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGizmoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoField.h"
#include "BsMonoManager.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsComponent.h"
#include "BsManagedComponent.h"
#include "Scene/BsGizmoManager.h"
#include "Scene/BsSelection.h"
#include "BsScriptObjectManager.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptComponent.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGizmoManager::ScriptGizmoManager(ScriptAssemblyManager& scriptObjectManager)
		:mScriptObjectManager(scriptObjectManager), mDrawGizmoAttribute(nullptr), mFlagsField(nullptr)
	{
		mDomainLoadedConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(std::bind(&ScriptGizmoManager::reloadAssemblyData, this));
		reloadAssemblyData();
	}

	ScriptGizmoManager::~ScriptGizmoManager()
	{
		mDomainLoadedConn.disconnect();
	}

	void ScriptGizmoManager::update()
	{
		GizmoManager::instance().clearGizmos();

		HSceneObject rootSO = SceneManager::instance().getRootNode();

		Stack<HSceneObject> todo;
		todo.push(rootSO);

		bool isParentSelected = false;
		UINT32 parentSelectedPopIdx = 0;
		
		Vector<HSceneObject> selectedObjects = Selection::instance().getSceneObjects();

		while (!todo.empty())
		{
			if (isParentSelected && parentSelectedPopIdx == (UINT32)todo.size())
			{
				isParentSelected = false;
			}

			HSceneObject curSO = todo.top();
			todo.pop();

			bool isSelected = std::count(selectedObjects.begin(), selectedObjects.end(), curSO) > 0;
			if (isSelected && !isParentSelected)
			{
				isParentSelected = true;
				parentSelectedPopIdx = (UINT32)todo.size();
			}

			const Vector<HComponent>& components = curSO->getComponents();
			for (auto& component : components)
			{
				String componentName;
				MonoObject* managedInstance = nullptr;
				if (rtti_is_of_type<ManagedComponent>(component.get()))
				{
					ManagedComponent* managedComponent = static_cast<ManagedComponent*>(component.get());
					componentName = managedComponent->getManagedFullTypeName();
					managedInstance = managedComponent->getManagedInstance();
				}
				else
				{
					ScriptGameObjectManager& sgoManager = ScriptGameObjectManager::instance();
					ScriptComponentBase* scriptComponent = sgoManager.getBuiltinScriptComponent(component, false);

					if (scriptComponent)
					{
						managedInstance = scriptComponent->getManagedInstance();

						String ns, typeName;
						MonoUtil::getClassName(managedInstance, ns, typeName);

						componentName = ns + "." + typeName;
					}
				}

				if (componentName.empty())
					continue;

				auto iterFind = mGizmoDrawers.find(componentName);
				if (iterFind != mGizmoDrawers.end())
				{
					UINT32 flags = iterFind->second.flags;

					bool drawGizmo = false;
					if (((flags & (UINT32)DrawGizmoFlags::Selected) != 0) && isSelected)
						drawGizmo = true;

					if (((flags & (UINT32)DrawGizmoFlags::ParentSelected) != 0) && isParentSelected)
						drawGizmo = true;

					if (((flags & (UINT32)DrawGizmoFlags::NotSelected) != 0) && !isSelected && !isParentSelected)
						drawGizmo = true;

					if (drawGizmo)
					{
						bool pickable = (flags & (UINT32)DrawGizmoFlags::Pickable) != 0;
						GizmoManager::instance().startGizmo(curSO);
						GizmoManager::instance().setPickable(pickable);

						void* params[1] = { managedInstance };
						iterFind->second.drawGizmosMethod->invoke(nullptr, params);

						GizmoManager::instance().endGizmo();
					}
				}
			}

			for (UINT32 i = 0; i < curSO->getNumChildren(); i++)
				todo.push(curSO->getChild(i));
		}
	}

	void ScriptGizmoManager::reloadAssemblyData()
	{
		// Reload DrawGizmo attribute from editor assembly
		MonoAssembly* editorAssembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);
		mDrawGizmoAttribute = editorAssembly->getClass("BansheeEditor", "DrawGizmo");
		if (mDrawGizmoAttribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find DrawGizmo managed class.");

		mFlagsField = mDrawGizmoAttribute->getField("flags");

		Vector<String> scriptAssemblyNames = mScriptObjectManager.getScriptAssemblies();
		for (auto& assemblyName : scriptAssemblyNames)
		{
			MonoAssembly* assembly = MonoManager::instance().getAssembly(assemblyName);

			// Find new gizmo drawer methods
			const Vector<MonoClass*>& allClasses = assembly->getAllClasses();
			for (auto curClass : allClasses)
			{
				const Vector<MonoMethod*>& methods = curClass->getAllMethods();
				for (auto& curMethod : methods)
				{
					UINT32 drawGizmoFlags = 0;
					MonoClass* componentType = nullptr;
					if (isValidDrawGizmoMethod(curMethod, componentType, drawGizmoFlags))
					{
						String fullComponentName = componentType->getFullName();
						GizmoData& newGizmoData = mGizmoDrawers[fullComponentName];

						newGizmoData.componentType = componentType;
						newGizmoData.drawGizmosMethod = curMethod;
						newGizmoData.flags = drawGizmoFlags;
					}
				}
			}
		}
	}

	bool ScriptGizmoManager::isValidDrawGizmoMethod(MonoMethod* method, MonoClass*& componentType, UINT32& drawGizmoFlags)
	{
		componentType = nullptr;
		drawGizmoFlags = 0;

		if (!method->hasAttribute(mDrawGizmoAttribute))
			return false;

		if (method->getNumParameters() != 1)
			return false;

		if (!method->isStatic())
			return false;

		MonoClass* paramType = method->getParameterType(0);
		MonoClass* componentClass = mScriptObjectManager.getComponentClass();

		if (!paramType->isSubClassOf(componentClass))
			return false;

		componentType = paramType;

		MonoObject* drawGizmoAttrib = method->getAttribute(mDrawGizmoAttribute);
		mFlagsField->get(drawGizmoAttrib, &drawGizmoFlags);

		return true;
	}
}