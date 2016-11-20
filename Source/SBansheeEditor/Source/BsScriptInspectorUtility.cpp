//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptInspectorUtility.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoAssembly.h"
#include "BsMonoUtil.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptObjectManager.h"
#include "BsScriptResource.h"
#include "BsScriptComponent.h"

namespace bs
{
	MonoClass* ScriptInspectorUtility::mCustomInspectorAtribute = nullptr;
	MonoField* ScriptInspectorUtility::mTypeField = nullptr;

	UnorderedMap<MonoClass*, MonoClass*> ScriptInspectorUtility::mInspectorTypes;
	UnorderedMap<MonoClass*, MonoClass*> ScriptInspectorUtility::mInspectableFieldTypes;

	HEvent ScriptInspectorUtility::mDomainLoadedConn;

	ScriptInspectorUtility::ScriptInspectorUtility(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptInspectorUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetCustomInspector", &ScriptInspectorUtility::internal_GetCustomInspector);
		metaData.scriptClass->addInternalCall("Internal_GetCustomInspectable", &ScriptInspectorUtility::internal_GetCustomInspectable);
	}

	void ScriptInspectorUtility::startUp()
	{
		mDomainLoadedConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(&ScriptInspectorUtility::reloadAssemblyData);
		reloadAssemblyData();
	}

	void ScriptInspectorUtility::shutDown()
	{
		mDomainLoadedConn.disconnect();
	}

	void ScriptInspectorUtility::reloadAssemblyData()
	{
		mInspectorTypes.clear();
		mInspectableFieldTypes.clear();

		// Reload MenuItem attribute from editor assembly
		MonoAssembly* editorAssembly = MonoManager::instance().getAssembly(EDITOR_ASSEMBLY);
		mCustomInspectorAtribute = editorAssembly->getClass("BansheeEditor", "CustomInspector");
		if (mCustomInspectorAtribute == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find CustomInspector managed class.");

		MonoClass* inspectorClass = editorAssembly->getClass("BansheeEditor", "Inspector");
		if (inspectorClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find Inspector managed class.");

		MonoClass* inspectableFieldClass = editorAssembly->getClass("BansheeEditor", "InspectableField");
		if (inspectableFieldClass == nullptr)
			BS_EXCEPT(InvalidStateException, "Cannot find InspectableField managed class.");

		mTypeField = mCustomInspectorAtribute->getField("type");

		ScriptAssemblyManager& sam = ScriptAssemblyManager::instance();

		Vector<String> scriptAssemblyNames = sam.getScriptAssemblies();
		for (auto& assemblyName : scriptAssemblyNames)
		{
			MonoAssembly* assembly = MonoManager::instance().getAssembly(assemblyName);

			// Find new classes/structs with the custom inspector attribute
			const Vector<MonoClass*>& allClasses = assembly->getAllClasses();
			for (auto curClass : allClasses)
			{
				MonoObject* attrib = curClass->getAttribute(mCustomInspectorAtribute);
				if (attrib == nullptr)
					continue;

				// Check if the attribute references a valid class
				MonoReflectionType* referencedReflType = nullptr;
				mTypeField->getValue(attrib, &referencedReflType);

				::MonoClass* referencedMonoClass = MonoUtil::getClass(referencedReflType);

				MonoClass* referencedClass = MonoManager::instance().findClass(referencedMonoClass);
				if (referencedClass == nullptr)
					continue;

				if (curClass->isSubClassOf(inspectorClass))
				{
					bool isValidInspectorType = referencedClass->isSubClassOf(ScriptResource::getMetaData()->scriptClass) ||
						referencedClass->isSubClassOf(ScriptComponent::getMetaData()->scriptClass);

					if (!isValidInspectorType)
						continue;

					mInspectorTypes[referencedClass] = curClass;
				}
				else if (curClass->isSubClassOf(inspectableFieldClass))
				{
					mInspectorTypes[referencedClass] = curClass;
				}
			}
		}
	}

	MonoObject* ScriptInspectorUtility::internal_GetCustomInspector(MonoReflectionType* reflType)
	{
		if (reflType == nullptr)
			return nullptr;

		::MonoClass* monoClass = MonoUtil::getClass(reflType);

		MonoClass* klass = MonoManager::instance().findClass(monoClass);
		if (klass == nullptr)
			return nullptr;

		auto iterFind = mInspectorTypes.find(klass);
		if (iterFind != mInspectorTypes.end())
			return iterFind->second->createInstance();

		return nullptr;
	}

	MonoReflectionType* ScriptInspectorUtility::internal_GetCustomInspectable(MonoReflectionType* reflType)
	{
		if (reflType == nullptr)
			return nullptr;

		::MonoClass* monoClass = MonoUtil::getClass(reflType);

		MonoClass* klass = MonoManager::instance().findClass(monoClass);
		if (klass == nullptr)
			return nullptr;

		// Try to find an inspectable field implementation directly referencing the class
		auto iterFind = mInspectableFieldTypes.find(klass);
		if (iterFind != mInspectableFieldTypes.end())
			return MonoUtil::getType(iterFind->second->_getInternalClass());

		// Try to find an inspectable field implementation referencing any of the class' attributes
		Vector<MonoClass*> attribs = klass->getAllAttributes();
		for (auto attrib : attribs)
		{
			auto iterFind = mInspectableFieldTypes.find(attrib);
			if (iterFind != mInspectableFieldTypes.end())
				return MonoUtil::getType(iterFind->second->_getInternalClass());
		}

		return nullptr;
	}
}