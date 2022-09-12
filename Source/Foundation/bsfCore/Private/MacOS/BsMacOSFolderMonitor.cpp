//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsFolderMonitor.h"
#include "FileSystem/BsFileSystem.h"
#include "Error/BsException.h"
#include "Utility/BsTimer.h"

#include <CoreServices/CoreServices.h>

namespace bs
{
	static constexpr UINT32 WRITE_STEADY_WAIT = 2000;
	CFStringRef FolderMonitorMode = CFSTR("BSFolderMonitor");

	enum class FileActionType
	{
		Added,
		Removed,
		Modified,
		Renamed
	};

	struct CreatedFileInfo
	{
		Path path;
		UINT64 lastSize;
		Timer timer;
	};

	struct FileAction
	{
		static FileAction* createAdded(const String& fileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Added;

			memcpy(action->newName, fileName.data(), fileName.size() * sizeof(String::value_type));
			action->newName[fileName.size()] = '\0';

			return action;
		}

		static FileAction* createRemoved(const String& fileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Removed;

			memcpy(action->newName, fileName.data(), fileName.size() * sizeof(String::value_type));
			action->newName[fileName.size()] = '\0';

			return action;
		}

		static FileAction* createModified(const String& fileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Modified;

			memcpy(action->newName, fileName.data(), fileName.size() * sizeof(String::value_type));
			action->newName[fileName.size()] = '\0';

			return action;
		}

		static FileAction* createRenamed(const String& oldFilename, const String& newfileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) +
					(oldFilename.size() + newfileName.size() + 2) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = (String::value_type*)bytes;
			bytes += (oldFilename.size() + 1) * sizeof(String::value_type);

			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Modified;

			memcpy(action->oldName, oldFilename.data(), oldFilename.size() * sizeof(String::value_type));
			action->oldName[oldFilename.size()] = '\0';

			memcpy(action->newName, newfileName.data(), newfileName.size() * sizeof(String::value_type));
			action->newName[newfileName.size()] = '\0';

			return action;
		}

		static void Destroy(FileAction* action)
		{
			bs_free(action);
		}

		String::value_type* oldName;
		String::value_type* newName;
		FileActionType type;
	};

	struct FolderMonitor::Pimpl
	{
		Vector<FolderWatchInfo*> monitorsToStart;
		Vector<FolderWatchInfo*> monitorsToStop;
		Vector<FolderWatchInfo*> monitors;

		Vector<FileAction*> fileActions;
		Vector<FileAction*> activeFileActions;

		Mutex mainMutex;
		Thread* workerThread;
	};

	static void watcherCallback(ConstFSEventStreamRef streamRef, void* userInfo, size_t numEvents, void* eventPaths,
								const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds);

	struct FolderMonitor::FolderWatchInfo
	{
		FolderWatchInfo(
			const Path& folderToMonitor,
			FolderMonitor* owner,
			bool monitorSubdirectories,
			FolderChangeBits filter);
		~FolderWatchInfo();

		void StartMonitor();
		void StopMonitor();

		Path folderToMonitor;
		FolderMonitor* owner;
		bool monitorSubdirectories;
		FolderChangeBits filter;
		FSEventStreamRef streamRef;
		bool hasStarted;
		Vector<CreatedFileInfo> createdFiles;
	};

	FolderMonitor::FolderWatchInfo::FolderWatchInfo(
		const Path& folderToMonitor,
		FolderMonitor* owner,
		bool monitorSubdirectories,
		FolderChangeBits filter)
			: FolderToMonitor(folderToMonitor)
			, Owner(owner)
			, MonitorSubdirectories(monitorSubdirectories)
			, Filter(filter)
			, StreamRef(nullptr)
			, HasStarted(false)
	{ }

	FolderMonitor::FolderWatchInfo::~FolderWatchInfo()
	{
		stopMonitor();
	}

	void FolderMonitor::FolderWatchInfo::startMonitor()
	{
		String pathString = folderToMonitor.toString();
		CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, pathString.c_str(), kCFStringEncodingUTF8);

		CFArrayRef pathArray = CFArrayCreate(nullptr, (const void **)&path, 1, nullptr);
		FSEventStreamContext context = {};
		context.info = this;

		CFAbsoluteTime latency = 0.1f;
		streamRef = FSEventStreamCreate(
				kCFAllocatorDefault,
				&watcherCallback,
				&context,
				pathArray,
				kFSEventStreamEventIdSinceNow,
				latency,
				kFSEventStreamCreateFlagUseCFTypes | kFSEventStreamCreateFlagFileEvents);

		CFRelease(pathArray);

		if (streamRef)
		{
			FSEventStreamScheduleWithRunLoop(streamRef, CFRunLoopGetCurrent(), FolderMonitorMode);
			if(FSEventStreamStart(streamRef))
				hasStarted = true;
		}

		CFRelease(path);
 	}

	void FolderMonitor::FolderWatchInfo::stopMonitor()
	{
		if(!streamRef)
			return;

		if(hasStarted)
		{
			FSEventStreamStop(streamRef);
			hasStarted = false;
		}

		FSEventStreamInvalidate(streamRef);
		FSEventStreamRelease(streamRef);
	}

	static void watcherCallback(ConstFSEventStreamRef streamRef, void* userInfo, size_t numEvents, void* eventPaths,
								const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds)
	{
		auto* watcher = (FolderMonitor::FolderWatchInfo*)userInfo;
		FolderMonitor::Pimpl* folderData = watcher->owner->_getPrivateData();

		auto paths = (CFArrayRef)eventPaths;
		CFIndex length = CFArrayGetCount(paths);

		for(CFIndex i = 0; i < length; i++)
		{
			auto pathEntry = (CFStringRef)CFArrayGetValueAtIndex(paths, i);
			Path path = CFStringGetCStringPtr(pathEntry, kCFStringEncodingUTF8);

			// Ignore folder meta-data (.DS_Store)
			String filename = path.getFilename(false);
			if(filename == ".DS_Store")
				continue;

			CFIndex pathLength = CFStringGetLength(pathEntry);
			if(pathLength == 0)
				continue;

			Lock Lock(folderData->mainMutex);

			// If not monitoring subdirectories, ignore paths that aren't direct descendants of the root path
			if(!watcher->monitorSubdirectories)
			{
				if(path.getParent() != watcher->folderToMonitor)
					continue;
			}

			FSEventStreamEventFlags flags = eventFlags[i];

			bool isFile = (flags & kFSEventStreamEventFlagItemIsFile) != 0;
			bool wasCreated = (flags & kFSEventStreamEventFlagItemCreated) != 0;
			bool wasModified = (flags & kFSEventStreamEventFlagItemModified) != 0;
			bool wasRemoved = (flags & kFSEventStreamEventFlagItemRemoved) != 0;
			bool ownerChange = (flags & kFSEventStreamEventFlagItemChangeOwner) != 0;

			// Ignore owner change as they just result in duplicate events
			if(ownerChange)
				continue;

			// Rename events get translated to create/remove events
			bool wasRenamed = (flags & kFSEventStreamEventFlagItemRenamed) != 0;

			if(wasRenamed)
			{
				if(FileSystem::exists(path))
					folderData->fileActions.push_back(FileAction::createAdded(path.toString()));
				else
					folderData->fileActions.push_back(FileAction::createRemoved(path.toString()));
			}

			// File/folder was added
			if(wasCreated)
			{
				if (!isFile)
				{
					if (watcher->filter.isSet(FolderChangeBit::DirName))
						folderData->fileActions.push_back(FileAction::createAdded(path.toString()));
				}
				else
				{
					if (watcher->filter.isSet(FolderChangeBit::FileName))
					{
						// We delay all file creation events until the file is done writing
						watcher->createdFiles.push_back(CreatedFileInfo());
						CreatedFileInfo& createdFileInfo = watcher->createdFiles.back();
						createdFileInfo.path = path;
						createdFileInfo.lastSize = FileSystem::getFileSize(path);
						createdFileInfo.timer.reset();
					}
				}
			}

			// File/folder was removed
			if(wasRemoved)
			{
				if(!isFile)
				{
					if(watcher->filter.isSet(FolderChangeBit::DirName))
						folderData->fileActions.push_back(FileAction::createRemoved(path.toString()));
				}
				else
				{
					if(watcher->filter.isSet(FolderChangeBit::FileName))
						folderData->fileActions.push_back(FileAction::createRemoved(path.toString()));
				}
			}

			// File was modified
			if(wasModified && watcher->filter.isSet(FolderChangeBit::FileWrite))
			{
				// Don't send out modified event if file was created
				auto iterFind = std::find_if(watcher->createdFiles.begin(), watcher->createdFiles.end(),
					 [&path](const CreatedFileInfo& info)
					 {
						return info.path == path;
					 }
				);

				if(iterFind == watcher->createdFiles.end())
					folderData->fileActions.push_back(FileAction::createModified(path.toString()));
			}
		}
	}

	class FolderMonitor::FileNotifyInfo
	{
	};

	FolderMonitor::FolderMonitor()
	{
		m = bs_new<Pimpl>();
		m->workerThread = nullptr;
	}

	FolderMonitor::~FolderMonitor()
	{
		stopMonitorAll();

		// No need for mutex since we know worker thread is shut down by now
		for(auto& action : m->fileActions)
			FileAction::destroy(action);

		bs_delete(m);
	}

	void FolderMonitor::StartMonitor(const Path& folderPath, bool subdirectories, FolderChangeBits changeFilter)
	{
		if(!FileSystem::isDirectory(folderPath))
		{
			BS_LOG(Error, Platform, "Provided path \"{0}\" is not a directory.", folderPath);
			return;
		}

		// Check if there is overlap with existing monitors
		for(auto& monitor : m->monitors)
		{
			// Identical monitor exists
			if(monitor->folderToMonitor.equals(folderPath))
			{
				BS_LOG(Warning, Platform, "Folder is already monitored, cannot monitor it again.");
				return;
			}

			// This directory is part of a directory that's being monitored
			if(monitor->monitorSubdirectories && folderPath.includes(monitor->folderToMonitor))
			{
				BS_LOG(Warning, Platform, "Folder is already monitored, cannot monitor it again.");
				return;
			}

			// This directory would include a directory of another monitor
			if(subdirectories && monitor->folderToMonitor.includes(folderPath))
			{
				BS_LOG(Warning, Platform, "Cannot add a recursive monitor as it conflicts with a previously monitored path.");
				return;
			}
		}

		auto watchInfo = bs_new<FolderWatchInfo>(folderPath, this, subdirectories, changeFilter);

		// Register and start the monitor
		{
			Lock Lock(m->mainMutex);

			m->monitorsToStart.push_back(watchInfo);
			m->monitors.push_back(watchInfo);
		}

		// Start the worker thread if it isn't already
		if(m->workerThread == nullptr)
		{
			m->workerThread = bs_new<Thread>(std::bind(&FolderMonitor::workerThreadMain, this));

			if(m->workerThread == nullptr)
				BS_LOG(Error, Platform, "Failed to create a new worker thread for folder monitoring");
		}
	}

	void FolderMonitor::StopMonitor(const Path& folderPath)
	{
		auto findIter = std::find_if(m->monitors.begin(), m->monitors.end(),
				[&](const FolderWatchInfo* x) { return x->folderToMonitor == folderPath; });

		if(findIter != m->monitors.end())
		{
			// Special case if this is the last monitor
			if(m->monitors.size() == 1)
				stopMonitorAll();
			else
			{
				Lock Lock(m->mainMutex);
				FolderWatchInfo* watchInfo = *findIter;

				m->monitorsToStop.push_back(watchInfo);
				m->monitors.erase(findIter);
			}
		}
	}

	void FolderMonitor::StopMonitorAll()
	{
		{
			Lock Lock(m->mainMutex);

			// Remove all watches (this will also wake up the thread)
			for (auto& watchInfo : m->monitors)
				m->monitorsToStop.push_back(watchInfo);

			m->monitors.clear();
		}

		// Wait for the thread to shutdown
		if(m->workerThread != nullptr)
		{
			m->workerThread->join();
			bs_delete(m->workerThread);
			m->workerThread = nullptr;
		}
	}

	void FolderMonitor::WorkerThreadMain()
	{
		MemStack::beginThread();

		while(true)
		{
			// Start up any newly added monitors
			{
				Lock Lock(m->mainMutex);

				for(auto& entry : m->monitorsToStart)
					entry->startMonitor();

				m->monitorsToStart.clear();
			}

			// Run the loop in order to receive events
			INT32 result = CFRunLoopRunInMode(FolderMonitorMode, 0.1f, false);

			// Delete any stopped monitors
			{
				Lock Lock(m->mainMutex);

				for (auto& entry : m->monitorsToStop)
					bs_delete(entry);

				m->monitorsToStop.clear();
			}

			// All input sources removed, or explicitly stopped, bail
			if((result == kCFRunLoopRunStopped) || (result == kCFRunLoopRunFinished))
				break;

			// Check if any created files have completed writing, and handle rename events
			{
				Lock Lock(m->mainMutex);

				for(auto& monitor : m->monitors)
				{
					FolderMonitor::Pimpl* folderData = monitor->owner->_getPrivateData();

					for(auto iter = monitor->createdFiles.begin(); iter != monitor->createdFiles.end();)
					{
						CreatedFileInfo& entry = *iter;

						UINT64 fileSize = FileSystem::getFileSize(entry.path);
						if(fileSize != entry.lastSize)
						{
							entry.lastSize = fileSize;
							entry.timer.reset();
						}

						if(entry.timer.getMilliseconds() > WRITE_STEADY_WAIT)
						{
							folderData->fileActions.push_back(FileAction::createAdded(entry.path.toString()));
							iter = monitor->createdFiles.erase(iter);
						}
						else
							++iter;
					}
				}
			}

			// It's possible some system registered an input source with our loop, in which case the above check will not
			// work. Instead check if there are any monitors left.
			// Note: In this case we may also pay a 0.1 second timeout cost, since we don't explicitly wake the run loop.
			//       Ideally we would also wake the run loop from the main thread so it is able to exit immediately.
			{
				Lock Lock(m->mainMutex);

				if(m->monitors.empty())
					break;
			}
		}

		MemStack::endThread();
	}

	void FolderMonitor::HandleNotifications(FileNotifyInfo& notifyInfo, FolderWatchInfo& watchInfo)
	{
		// Do nothing
	}

	void FolderMonitor::_update()
	{
		{
			Lock Lock(m->mainMutex);

			std::swap(m->fileActions, m->activeFileActions);
		}

		for(auto& action : m->activeFileActions)
		{
			switch (action->type)
			{
			case FileActionType::Added:
				if (!onAdded.empty())
					onAdded(Path(action->newName));
				break;
			case FileActionType::Removed:
				if (!onRemoved.empty())
					onRemoved(Path(action->newName));
				break;
			case FileActionType::Modified:
				if (!onModified.empty())
					onModified(Path(action->newName));
				break;
			case FileActionType::Renamed:
				if (!onRenamed.empty())
					onRenamed(Path(action->oldName), Path(action->newName));
				break;
			}

			FileAction::destroy(action);
		}

		m->activeFileActions.clear();
	}
}

