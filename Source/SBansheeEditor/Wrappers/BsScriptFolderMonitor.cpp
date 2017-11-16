//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptFolderMonitor.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"

using namespace std::placeholders;

namespace bs
{
	ScriptFolderMonitor::OnModifiedThunkDef ScriptFolderMonitor::OnModifiedThunk;
	ScriptFolderMonitor::OnModifiedThunkDef ScriptFolderMonitor::OnAddedThunk;
	ScriptFolderMonitor::OnModifiedThunkDef ScriptFolderMonitor::OnRemovedThunk;
	ScriptFolderMonitor::OnRenamedThunkDef ScriptFolderMonitor::OnRenamedThunk;

	ScriptFolderMonitor::ScriptFolderMonitor(MonoObject* instance, FolderMonitor* monitor)
		:ScriptObject(instance), mMonitor(monitor)
	{
		mGCHandle = MonoUtil::newWeakGCHandle(instance);

		if (mMonitor != nullptr)
		{
			ScriptFolderMonitorManager::instance()._registerMonitor(this);

			mMonitor->onAdded.connect(std::bind(&ScriptFolderMonitor::onMonitorFileAdded, this, _1));
			mMonitor->onRemoved.connect(std::bind(&ScriptFolderMonitor::onMonitorFileRemoved, this, _1));
			mMonitor->onModified.connect(std::bind(&ScriptFolderMonitor::onMonitorFileModified, this, _1));
			mMonitor->onRenamed.connect(std::bind(&ScriptFolderMonitor::onMonitorFileRenamed, this, _1, _2));
		}
	}

	ScriptFolderMonitor::~ScriptFolderMonitor()
	{
		destroy();
	}

	void ScriptFolderMonitor::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptFolderMonitor::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Destroy", (void*)&ScriptFolderMonitor::internal_Destroy);

		OnModifiedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnModified", 1)->getThunk();
		OnAddedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnAdded", 1)->getThunk();
		OnRemovedThunk = (OnModifiedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnRemoved", 1)->getThunk();
		OnRenamedThunk = (OnRenamedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnRenamed", 2)->getThunk();
	}

	void ScriptFolderMonitor::internal_CreateInstance(MonoObject* instance, MonoString* folder)
	{
		FolderMonitor* monitor = nullptr;
		if (folder != nullptr)
		{
			Path folderPath = MonoUtil::monoToWString(folder);

			FolderChangeBits folderChanges;
			folderChanges |= FolderChangeBit::FileName;
			folderChanges |= FolderChangeBit::DirName;
			folderChanges |= FolderChangeBit::FileWrite;

			monitor = bs_new<FolderMonitor>();
			monitor->startMonitor(folderPath, true, folderChanges);
		}

		new (bs_alloc<ScriptFolderMonitor>()) ScriptFolderMonitor(instance, monitor);
	}

	void ScriptFolderMonitor::internal_Destroy(ScriptFolderMonitor* thisPtr)
	{
		thisPtr->destroy();
	}

	void ScriptFolderMonitor::onMonitorFileModified(const Path& path)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		MonoString* monoPath = MonoUtil::wstringToMono(path.toWString());
		MonoUtil::invokeThunk(OnModifiedThunk, instance, monoPath);
	}

	void ScriptFolderMonitor::onMonitorFileAdded(const Path& path)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		MonoString* monoPath = MonoUtil::wstringToMono(path.toWString());
		MonoUtil::invokeThunk(OnAddedThunk, instance, monoPath);
	}

	void ScriptFolderMonitor::onMonitorFileRemoved(const Path& path)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		MonoString* monoPath = MonoUtil::wstringToMono(path.toWString());
		MonoUtil::invokeThunk(OnRemovedThunk, instance, monoPath);
	}

	void ScriptFolderMonitor::onMonitorFileRenamed(const Path& from, const Path& to)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		MonoString* monoPathFrom = MonoUtil::wstringToMono(from.toWString());
		MonoString* monoPathTo = MonoUtil::wstringToMono(to.toWString());

		MonoUtil::invokeThunk(OnRenamedThunk, instance, monoPathFrom, monoPathTo);
	}

	void ScriptFolderMonitor::update()
	{
		mMonitor->_update();
	}

	void ScriptFolderMonitor::destroy()
	{
		if (mMonitor != nullptr)
		{
			mMonitor->stopMonitorAll();
			bs_delete(mMonitor);
			mMonitor = nullptr;

			ScriptFolderMonitorManager::instance()._unregisterMonitor(this);
		}

		mGCHandle = 0;
	}

	void ScriptFolderMonitorManager::update()
	{
		for (auto& monitor : mMonitors)
			monitor->update();
	}

	void ScriptFolderMonitorManager::_registerMonitor(ScriptFolderMonitor* monitor)
	{
		mMonitors.insert(monitor);
	}

	void ScriptFolderMonitorManager::_unregisterMonitor(ScriptFolderMonitor* monitor)
	{
		mMonitors.erase(monitor);
	}
}