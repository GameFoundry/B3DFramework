//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Platform/BsFolderMonitor.h"
#include "FileSystem/BsFileSystem.h"
#include "Error/BsException.h"
#include <sys/inotify.h>

namespace bs
{
	struct FolderMonitor::FolderWatchInfo
	{
		FolderWatchInfo(const Path& folderToMonitor, int inHandle, bool monitorSubdirectories, FolderChangeBits filter);
		~FolderWatchInfo();

		void startMonitor();
		void stopMonitor();

		void addPath(const Path& path);
		void removePath(const Path& path);
		Path getPath(INT32 handle);

		Path folderToMonitor;
		int dirHandle;
		bool monitorSubdirectories;
		FolderChangeBits filter;

		UnorderedMap<Path, INT32> pathToHandle;
		UnorderedMap<INT32, Path> handleToPath;
	};

	FolderMonitor::FolderWatchInfo::FolderWatchInfo(const Path& folderToMonitor, int inHandle, bool monitorSubdirectories,
													FolderChangeBits filter)
		: folderToMonitor(folderToMonitor), dirHandle(inHandle), monitorSubdirectories(monitorSubdirectories)
		, filter(filter)
	{ }

	FolderMonitor::FolderWatchInfo::~FolderWatchInfo()
	{
		stopMonitor();
	}

	void FolderMonitor::FolderWatchInfo::startMonitor()
	{
		addPath(folderToMonitor);

		if(monitorSubdirectories)
		{
			FileSystem::iterate(folderToMonitor, nullptr, [this](const Path& path)
			{
				addPath(path);
				return true;
			});
		}
	}

	void FolderMonitor::FolderWatchInfo::stopMonitor()
	{
		for(auto& entry : pathToHandle)
			inotify_rm_watch(dirHandle, entry.second);

		pathToHandle.clear();
	}

	void FolderMonitor::FolderWatchInfo::addPath(const Path& path)
	{
		String pathString = path.toString();

		INT32 watchHandle = inotify_add_watch(dirHandle, pathString.c_str(), IN_ALL_EVENTS);
		if(watchHandle == -1)
		{
			String error = strerror(errno);
			LOGERR("Unable to start folder monitor for path: \"" + pathString +"\". Error: " + error);
		}

		pathToHandle[path] = watchHandle;
		handleToPath[watchHandle] = path;
	}

	void FolderMonitor::FolderWatchInfo::removePath(const Path& path)
	{
		auto iterFind = pathToHandle.find(path);
		if(iterFind != pathToHandle.end())
		{
			INT32 watchHandle = iterFind->second;
			pathToHandle.erase(iterFind);

			handleToPath.erase(watchHandle);
		}
	}

	Path FolderMonitor::FolderWatchInfo::getPath(INT32 handle)
	{
		auto iterFind = handleToPath.find(handle);
		if(iterFind != handleToPath.end())
			return iterFind->second;

		return Path::BLANK;
	}

	class FolderMonitor::FileNotifyInfo
	{
	};

	enum class FileActionType
	{
		Added,
		Removed,
		Modified,
		Renamed
	};

	struct FileAction
	{
		static FileAction* createAdded(const WString& fileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(WString::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (WString::value_type*)bytes;
			action->type = FileActionType::Added;

			memcpy(action->newName, fileName.data(), fileName.size() * sizeof(WString::value_type));
			action->newName[fileName.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static FileAction* createRemoved(const WString& fileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(WString::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (WString::value_type*)bytes;
			action->type = FileActionType::Removed;

			memcpy(action->newName, fileName.data(), fileName.size() * sizeof(WString::value_type));
			action->newName[fileName.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static FileAction* createModified(const WString& fileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(WString::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (WString::value_type*)bytes;
			action->type = FileActionType::Modified;

			memcpy(action->newName, fileName.data(), fileName.size() * sizeof(WString::value_type));
			action->newName[fileName.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static FileAction* createRenamed(const WString& oldFilename, const WString& newfileName)
		{
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + 
				(oldFilename.size() + newfileName.size() + 2) * sizeof(WString::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = (WString::value_type*)bytes;
			bytes += (oldFilename.size() + 1) * sizeof(WString::value_type);

			action->newName = (WString::value_type*)bytes;
			action->type = FileActionType::Modified;

			memcpy(action->oldName, oldFilename.data(), oldFilename.size() * sizeof(WString::value_type));
			action->oldName[oldFilename.size()] = L'\0';

			memcpy(action->newName, newfileName.data(), newfileName.size() * sizeof(WString::value_type));
			action->newName[newfileName.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static void destroy(FileAction* action)
		{
			bs_free(action);
		}

		WString::value_type* oldName;
		WString::value_type* newName;
		FileActionType type;

		UINT64 lastSize;
		bool checkForWriteStarted;
	};

	struct FolderMonitor::Pimpl
	{
		Vector<FolderWatchInfo*> monitors;

		Vector<FileAction*> fileActions;
		Vector<FileAction*> activeFileActions;

		int inHandle;
		bool started;
		Mutex mainMutex;
		Thread* workerThread;
	};

	FolderMonitor::FolderMonitor()
	{
		m = bs_new<Pimpl>();
		m->workerThread = nullptr;
		m->inHandle = 0;
		m->started = false;
	}

	FolderMonitor::~FolderMonitor()
	{
		stopMonitorAll();

		// No need for mutex since we know worker thread is shut down by now
		for(auto& action : m->fileActions)
			FileAction::destroy(action);

		bs_delete(m);
	}

	void FolderMonitor::startMonitor(const Path& folderPath, bool subdirectories, FolderChangeBits changeFilter)
	{
		if(!FileSystem::isDirectory(folderPath))
		{
			LOGERR("Provided path \"" + folderPath.toString() + "\" is not a directory");
			return;
		}

		// Check if there is overlap with existing monitors
		for(auto& monitor : m->monitors)
		{
			// Identical monitor exists
			if(monitor->folderToMonitor.equals(folderPath))
			{
				LOGWRN("Folder is already monitored, cannot monitor it again.");
				return;
			}

			// This directory is part of a directory that's being monitored
			if(monitor->monitorSubdirectories && folderPath.includes(monitor->folderToMonitor))
			{
				LOGWRN("Folder is already monitored, cannot monitor it again.");
				return;
			}

			// This directory would include a directory of another monitor
			if(subdirectories && monitor->folderToMonitor.includes(folderPath))
			{
				LOGWRN("Cannot add a recursive monitor as it conflicts with a previously monitored path");
				return;
			}
		}

		// Initialize inotify if required
		if(!m->started)
		{
			Lock lock(m->mainMutex);

			m->inHandle = inotify_init();
			m->started = true;
		}

		FolderWatchInfo* watchInfo = bs_new<FolderWatchInfo>(folderPath, m->inHandle, subdirectories, changeFilter);

		// Register and start the monitor
		{
			Lock lock(m->mainMutex);

			m->monitors.push_back(watchInfo);
			watchInfo->startMonitor();
		}

		// Start the worker thread if it isn't already
		if(m->workerThread == nullptr)
		{
			m->workerThread = bs_new<Thread>(std::bind(&FolderMonitor::workerThreadMain, this));

			if(m->workerThread == nullptr)
				LOGERR("Failed to create a new worker thread for folder monitoring");
		}
	}

	void FolderMonitor::stopMonitor(const Path& folderPath)
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
				Lock lock(m->mainMutex);
				FolderWatchInfo* watchInfo = *findIter;

				watchInfo->stopMonitor();
				bs_delete(watchInfo);

				m->monitors.erase(findIter);
			}
		}
	}

	void FolderMonitor::stopMonitorAll()
	{
		if(m->started)
		{
			Lock lock(m->mainMutex);

			// First tell the thread it's ready to be shutdown
			m->started = false;

			// Remove all watches (this will also wake up the thread). Note that at least one watch must be present otherwise
			// the thread won't wake up (we ensure that elsewhere).
			for (auto& watchInfo : m->monitors)
			{
				watchInfo->stopMonitor();
				bs_delete(watchInfo);
			}

			m->monitors.clear();
		}

		// Wait for the thread to shutdown
		if(m->workerThread != nullptr)
		{
			m->workerThread->join();
			bs_delete(m->workerThread);
			m->workerThread = nullptr;
		}

		// Close the inotify handle
		{
			Lock lock(m->mainMutex);
			if (m->inHandle != 0)
			{
				close(m->inHandle);
				m->inHandle = 0;
			}
		}
	}

	void FolderMonitor::workerThreadMain()
	{
		static const UINT32 BUFFER_SIZE = 16384;

		bool shouldRun;
		INT32 watchHandle;
		{
			Lock(m->mainMutex);
			watchHandle = m->inHandle;
			shouldRun = m->started;
		}

		UINT8 buffer[BUFFER_SIZE];

		while(shouldRun)
		{
			INT32 length = (INT32)read(watchHandle, buffer, sizeof(buffer));

			// Handle was closed, shutdown thread
			if (length < 0)
				return;

			// Note: Must be after read, so shutdown can be started when we remove the watches (as then read() will return)
			{
				Lock(m->mainMutex);
				shouldRun = m->started;
			}

			INT32 readPos = 0;
			while(readPos < length)
			{
				inotify_event* event = (inotify_event*)&buffer[readPos];
				if(event->len > 0)
				{
					{
						Lock lock(m->mainMutex);

						Path path;
						FolderWatchInfo* monitor = nullptr;
						for (auto& entry : m->monitors)
						{
							path = entry->getPath(event->wd);
							if (!path.isEmpty())
							{
								path.append(event->name);
								monitor = entry;
								break;
							}
						}

						// This can happen if the path got removed during some recent previous event
						if(monitor == nullptr)
							goto next;

						// Need to add/remove sub-directories to/from watch list
						bool isDirectory = (event->mask & IN_ISDIR) != 0;
						if(isDirectory && monitor->monitorSubdirectories)
						{
							bool added = (event->mask & (IN_CREATE | IN_MOVED_TO)) != 0;
							bool removed = (event->mask & (IN_DELETE | IN_MOVED_FROM)) != 0;

							if(added)
								monitor->addPath(path);
							else if(removed)
								monitor->removePath(path);
						}

						// Actually trigger the events

						// File/folder was added
						if(((event->mask & (IN_CREATE | IN_MOVED_TO)) != 0))
						{
							if (isDirectory)
							{
								if (monitor->filter.isSet(FolderChangeBit::DirName))
									m->fileActions.push_back(FileAction::createAdded(path.toWString()));
							}
							else
							{
								if (monitor->filter.isSet(FolderChangeBit::FileName))
									m->fileActions.push_back(FileAction::createAdded(path.toWString()));
							}
						}

						// File/folder was removed
						if(((event->mask & (IN_DELETE | IN_MOVED_FROM)) != 0))
						{
							if(isDirectory)
							{
								if(monitor->filter.isSet(FolderChangeBit::DirName))
									m->fileActions.push_back(FileAction::createRemoved(path.toWString()));
							}
							else
							{
								if(monitor->filter.isSet(FolderChangeBit::FileName))
									m->fileActions.push_back(FileAction::createRemoved(path.toWString()));
							}
						}

						// File was modified
						if(((event->mask & IN_CLOSE_WRITE) != 0) && monitor->filter.isSet(FolderChangeBit::FileWrite))
						{
							m->fileActions.push_back(FileAction::createModified(path.toWString()));
						}

						// Note: Not reporting renames, instead a remove + add event is created. To support renames I'd need
						// to defer all event triggering until I have processed move event pairs and determined if the
						// move is a rename (i.e. parent folder didn't change). All events need to be deferred (not just
						// move events) in order to preserve the event ordering. For now this is too much hassle considering
						// no external code relies on the rename functionality.
					}
				}

				next:
				readPos += sizeof(inotify_event) + event->len;
			}
		}
	}

	void FolderMonitor::handleNotifications(FileNotifyInfo& notifyInfo, FolderWatchInfo& watchInfo)
	{
		// Do nothing
	}

	void FolderMonitor::_update()
	{
		{
			Lock lock(m->mainMutex);

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
