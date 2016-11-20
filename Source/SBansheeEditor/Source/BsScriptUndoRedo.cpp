//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptUndoRedo.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsScriptSceneObject.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "BsUndoRedo.h"
#include "BsCmdRecordSO.h"
#include "BsCmdCloneSO.h"
#include "BsCmdCreateSO.h"
#include "BsCmdDeleteSO.h"
#include "BsCmdInstantiateSO.h"
#include "BsCmdReparentSO.h"
#include "BsCmdBreakPrefab.h"
#include "BsScriptPrefab.h"
#include "BsManagedEditorCommand.h"
#include "BsPrefab.h"
#include "BsScriptObjectManager.h"

namespace bs
{
	ScriptUndoRedo* ScriptUndoRedo::sGlobalUndoRedo = nullptr;
	HEvent ScriptUndoRedo::sDomainLoadConn;

	ScriptUndoRedo::ScriptUndoRedo(MonoObject* instance, const SPtr<UndoRedo>& undoRedo)
		:ScriptObject(instance), mUndoRedo(undoRedo)
	{ }

	void ScriptUndoRedo::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptUndoRedo::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Undo", &ScriptUndoRedo::internal_Undo);
		metaData.scriptClass->addInternalCall("Internal_Redo", &ScriptUndoRedo::internal_Redo);
		metaData.scriptClass->addInternalCall("Internal_RegisterCommand", &ScriptUndoRedo::internal_RegisterCommand);
		metaData.scriptClass->addInternalCall("Internal_PushGroup", &ScriptUndoRedo::internal_PushGroup);
		metaData.scriptClass->addInternalCall("Internal_PopGroup", &ScriptUndoRedo::internal_PopGroup);
		metaData.scriptClass->addInternalCall("Internal_Clear", &ScriptUndoRedo::internal_Clear);
		metaData.scriptClass->addInternalCall("Internal_GetTopCommandId", &ScriptUndoRedo::internal_GetTopCommandId);
		metaData.scriptClass->addInternalCall("Internal_PopCommand", &ScriptUndoRedo::internal_PopCommand);
		metaData.scriptClass->addInternalCall("Internal_RecordSO", &ScriptUndoRedo::internal_RecordSO);
		metaData.scriptClass->addInternalCall("Internal_CloneSO", &ScriptUndoRedo::internal_CloneSO);
		metaData.scriptClass->addInternalCall("Internal_CloneSOMulti", &ScriptUndoRedo::internal_CloneSOMulti);
		metaData.scriptClass->addInternalCall("Internal_Instantiate", &ScriptUndoRedo::internal_Instantiate);
		metaData.scriptClass->addInternalCall("Internal_CreateSO", &ScriptUndoRedo::internal_CreateSO);
		metaData.scriptClass->addInternalCall("Internal_DeleteSO", &ScriptUndoRedo::internal_DeleteSO);
		metaData.scriptClass->addInternalCall("Internal_ReparentSO", &ScriptUndoRedo::internal_ReparentSO);
		metaData.scriptClass->addInternalCall("Internal_ReparentSOMulti", &ScriptUndoRedo::internal_ReparentSOMulti);
		metaData.scriptClass->addInternalCall("Internal_BreakPrefab", &ScriptUndoRedo::internal_BreakPrefab);
	}

	void ScriptUndoRedo::startUp()
	{
		auto createPanel = []()
		{
			assert(sGlobalUndoRedo == nullptr);

			bool dummy = false;
			void* ctorParams[1] = { &dummy };

			MonoObject* instance = metaData.scriptClass->createInstance("bool", ctorParams);
			new (bs_alloc<ScriptUndoRedo>()) ScriptUndoRedo(instance, nullptr);

			MonoMethod* setGlobal = metaData.scriptClass->getMethod("Internal_SetGlobal", 1);
			void* setGlobalParams[1] = { instance };
			setGlobal->invoke(nullptr, setGlobalParams);
		};

		sDomainLoadConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(createPanel);

		createPanel();
	}

	void ScriptUndoRedo::shutDown()
	{
		sDomainLoadConn.disconnect();
	}

	MonoObject* ScriptUndoRedo::create()
	{
		bool dummy = false;
		void* params[1] = { &dummy };

		MonoObject* instance = metaData.scriptClass->createInstance("bool", params);

		SPtr<UndoRedo> undoRedo = bs_shared_ptr_new<UndoRedo>();
		new (bs_alloc<ScriptUndoRedo>()) ScriptUndoRedo(instance, undoRedo);

		return instance;
	}

	void ScriptUndoRedo::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<UndoRedo> undoRedo = bs_shared_ptr_new<UndoRedo>();
		new (bs_alloc<ScriptUndoRedo>()) ScriptUndoRedo(instance, undoRedo);
	}

	void ScriptUndoRedo::internal_Undo(ScriptUndoRedo* thisPtr)
	{
		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		undoRedo->undo();
	}

	void ScriptUndoRedo::internal_Redo(ScriptUndoRedo* thisPtr)
	{
		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		undoRedo->redo();
	}

	void ScriptUndoRedo::internal_RegisterCommand(ScriptUndoRedo* thisPtr, ScriptCmdManaged* command)
	{
		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		undoRedo->registerCommand(command->getInternal());
	}

	void ScriptUndoRedo::internal_PushGroup(ScriptUndoRedo* thisPtr, MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		undoRedo->pushGroup(nativeName);
	}

	void ScriptUndoRedo::internal_PopGroup(ScriptUndoRedo* thisPtr, MonoString* name)
	{
		String nativeName = MonoUtil::monoToString(name);

		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		undoRedo->popGroup(nativeName);
	}

	void ScriptUndoRedo::internal_Clear(ScriptUndoRedo* thisPtr)
	{
		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		return undoRedo->clear();
	}

	UINT32 ScriptUndoRedo::internal_GetTopCommandId(ScriptUndoRedo* thisPtr)
	{
		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		return undoRedo->getTopCommandId();
	}

	void ScriptUndoRedo::internal_PopCommand(ScriptUndoRedo* thisPtr, UINT32 id)
	{
		UndoRedo* undoRedo = thisPtr->mUndoRedo != nullptr ? thisPtr->mUndoRedo.get() : UndoRedo::instancePtr();
		undoRedo->popCommand(id);
	}

	void ScriptUndoRedo::internal_RecordSO(ScriptSceneObject* soPtr, bool recordHierarchy, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);
		CmdRecordSO::execute(soPtr->getNativeSceneObject(), recordHierarchy, nativeDescription);
	}

	MonoObject* ScriptUndoRedo::internal_CloneSO(ScriptSceneObject* soPtr, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);
		HSceneObject clone = CmdCloneSO::execute(soPtr->getNativeSceneObject(), nativeDescription);

		ScriptSceneObject* cloneSoPtr = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(clone);
		return cloneSoPtr->getManagedInstance();
	}

	MonoArray* ScriptUndoRedo::internal_CloneSOMulti(MonoArray* soPtrs, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);

		ScriptArray input(soPtrs);
		ScriptArray output = ScriptArray::create<ScriptSceneObject>(input.size());

		for (UINT32 i = 0; i < input.size(); i++)
		{
			ScriptSceneObject* soPtr = input.get<ScriptSceneObject*>(i);
			HSceneObject clone = CmdCloneSO::execute(soPtr->getNativeSceneObject(), nativeDescription);

			ScriptSceneObject* cloneSoPtr = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(clone);
			output.set(i, cloneSoPtr->getManagedInstance());
		}

		return output.getInternal();
	}

	MonoObject* ScriptUndoRedo::internal_Instantiate(ScriptPrefab* prefabPtr, MonoString* description)
	{
		HPrefab prefab = prefabPtr->getHandle();
		if (!prefab.isLoaded())
			return nullptr;

		WString nativeDescription = MonoUtil::monoToWString(description);
		HSceneObject clone = CmdInstantiateSO::execute(prefab, nativeDescription);

		ScriptSceneObject* cloneSoPtr = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(clone);
		return cloneSoPtr->getManagedInstance();
	}

	MonoObject* ScriptUndoRedo::internal_CreateSO(MonoString* name, MonoString* description)
	{
		String nativeName = MonoUtil::monoToString(name);
		WString nativeDescription = MonoUtil::monoToWString(description);
		HSceneObject newObj = CmdCreateSO::execute(nativeName, 0, nativeDescription);

		return ScriptGameObjectManager::instance().createScriptSceneObject(newObj)->getManagedInstance();
	}

	void ScriptUndoRedo::internal_DeleteSO(ScriptSceneObject* soPtr, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);
		CmdDeleteSO::execute(soPtr->getNativeSceneObject(), nativeDescription);
	}

	void ScriptUndoRedo::internal_ReparentSO(ScriptSceneObject* soPtr, ScriptSceneObject* parentSOPtr, MonoString* description)
	{
		HSceneObject parent;
		if (parentSOPtr != nullptr)
			parent = parentSOPtr->getNativeSceneObject();

		WString nativeDescription = MonoUtil::monoToWString(description);

		HSceneObject so = soPtr->getNativeSceneObject();
		CmdReparentSO::execute(so, parent, nativeDescription);
	}

	void ScriptUndoRedo::internal_ReparentSOMulti(MonoArray* soPtrs, ScriptSceneObject* parentSOPtr, MonoString* description)
	{
		HSceneObject parent;
		if (parentSOPtr != nullptr)
			parent = parentSOPtr->getNativeSceneObject();

		WString nativeDescription = MonoUtil::monoToWString(description);

		ScriptArray input(soPtrs);
		for (UINT32 i = 0; i < input.size(); i++)
		{
			ScriptSceneObject* soPtr = input.get<ScriptSceneObject*>(i);
			HSceneObject so = soPtr->getNativeSceneObject();
			CmdReparentSO::execute(so, parent, nativeDescription);
		}
	}

	void ScriptUndoRedo::internal_BreakPrefab(ScriptSceneObject* soPtr, MonoString* description)
	{
		WString nativeDescription = MonoUtil::monoToWString(description);

		HSceneObject so = soPtr->getNativeSceneObject();
		CmdBreakPrefab::execute(so, nativeDescription);
	}
}