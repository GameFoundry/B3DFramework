//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptCodeEditor.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "CodeEditor/BsCodeEditor.h"

using namespace std::placeholders;

namespace bs
{
	ScriptCodeEditor::ScriptCodeEditor(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptCodeEditor::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_SetActiveEditor", (void*)&ScriptCodeEditor::internal_SetActiveEditor);
		metaData.scriptClass->addInternalCall("Internal_GetActiveEditor", (void*)&ScriptCodeEditor::internal_GetActiveEditor);
		metaData.scriptClass->addInternalCall("Internal_GetAvailableEditors", (void*)&ScriptCodeEditor::internal_GetAvailableEditors);
		metaData.scriptClass->addInternalCall("Internal_OpenFile", (void*)&ScriptCodeEditor::internal_OpenFile);
		metaData.scriptClass->addInternalCall("Internal_SyncSolution", (void*)&ScriptCodeEditor::internal_SyncSolution);
	}

	CodeEditorType ScriptCodeEditor::internal_GetActiveEditor()
	{
		return CodeEditorManager::instance().getActive();
	}

	void ScriptCodeEditor::internal_SetActiveEditor(CodeEditorType type)
	{
		CodeEditorManager::instance().setActive(type);
	}

	MonoArray* ScriptCodeEditor::internal_GetAvailableEditors()
	{
		Vector<CodeEditorType> availableEditors = CodeEditorManager::instance().getAvailableEditors();

		ScriptArray outArray = ScriptArray::create<INT32>((UINT32)availableEditors.size());
		UINT32 idx = 0;
		for (auto& entry : availableEditors)
		{
			outArray.set(idx, (INT32)entry);
			idx++;
		}

		return outArray.getInternal();
	}

	void ScriptCodeEditor::internal_OpenFile(MonoString* path, UINT32 line)
	{
		Path filePath = MonoUtil::monoToWString(path);

		CodeEditorManager::instance().openFile(filePath, line);
	}

	void ScriptCodeEditor::internal_SyncSolution()
	{
		CodeEditorManager::instance().syncSolution();
	}
}