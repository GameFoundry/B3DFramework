//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/B3DFolderMonitor.h"
#include "FileSystem/B3DFileSystem.h"
#include <sys/inotify.h>

using namespace b3d;

struct LinuxFolderMonitor
{
	LinuxFolderMonitor(const Path& folderToMonitor, int inHandle, bool monitorSubdirectories, FolderChangeFlags filter);
	~LinuxFolderMonitor();

	void StartMonitor();
	void StopMonitor();

	void AddPath(const Path& path);
	void RemovePath(const Path& path);
	Path GetPath(i32 handle);

	Path FolderToMonitor;
	int DirectoryHandle;
	bool MonitorSubdirectories;
	FolderChangeFlags Filter;

	UnorderedMap<Path, i32> PathToHandle;
	UnorderedMap<i32, Path> HandleToPath;
};

LinuxFolderMonitor::LinuxFolderMonitor(const Path& folderToMonitor, int inHandle, bool monitorSubdirectories, FolderChangeFlags filter)
	: FolderToMonitor(folderToMonitor), DirectoryHandle(inHandle), MonitorSubdirectories(monitorSubdirectories), Filter(filter)
{}

LinuxFolderMonitor::~LinuxFolderMonitor()
{
	StopMonitor();
}

void LinuxFolderMonitor::StartMonitor()
{
	AddPath(FolderToMonitor);

	if(MonitorSubdirectories)
		FileSystem::Iterate(FolderToMonitor, nullptr, [this](const Path& path) { AddPath(path); return true; });
}

void LinuxFolderMonitor::StopMonitor()
{
	for(auto& entry : PathToHandle)
		inotify_rm_watch(DirectoryHandle, entry.second);

	PathToHandle.clear();
}

void LinuxFolderMonitor::AddPath(const Path& path)
{
	String pathString = path.ToString();

	i32 watchHandle = inotify_add_watch(directoryHandle, pathString.c_str(), IN_ALL_EVENTS);
	if(watchHandle == -1)
	{
		String error = strerror(errno);
		B3D_LOG(Error, LogPlatform, "Unable to start folder monitor for path: \"{0}\". Error: {1}", pathString, error);
	}

	PathToHandle[path] = watchHandle;
	HandleToPath[watchHandle] = path;
}

void LinuxFolderMonitor::RemovePath(const Path& path)
{
	auto found = PathToHandle.find(path);
	if(found != PathToHandle.end())
	{
		i32 watchHandle = found->second;
		PathToHandle.erase(found);

		HandleToPath.erase(watchHandle);
	}
}

Path LinuxFolderMonitor::GetPath(i32 handle)
{
	auto found = HandleToPath.find(handle);
	if(found != HandleToPath.end())
		return found->second;

	return Path::kBlank;
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
	static FileAction* CreateAdded(const String& fileName)
	{
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->oldName = nullptr;
		action->newName = (String::value_type*)bytes;
		action->type = FileActionType::Added;

		memcpy(action->newName, fileName.data(), fileName.size() * sizeof(String::value_type));
		action->newName[fileName.size()] = L'\0';
		action->lastSize = 0;
		action->checkForWriteStarted = false;

		return action;
	}

	static FileAction* CreateRemoved(const String& fileName)
	{
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->oldName = nullptr;
		action->newName = (String::value_type*)bytes;
		action->type = FileActionType::Removed;

		memcpy(action->newName, fileName.data(), fileName.size() * sizeof(String::value_type));
		action->newName[fileName.size()] = L'\0';
		action->lastSize = 0;
		action->checkForWriteStarted = false;

		return action;
	}

	static FileAction* CreateModified(const String& fileName)
	{
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (fileName.size() + 1) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->oldName = nullptr;
		action->newName = (String::value_type*)bytes;
		action->type = FileActionType::Modified;

		memcpy(action->newName, fileName.data(), fileName.size() * sizeof(String::value_type));
		action->newName[fileName.size()] = L'\0';
		action->lastSize = 0;
		action->checkForWriteStarted = false;

		return action;
	}

	static FileAction* CreateRenamed(const String& oldFilename, const String& newfileName)
	{
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (oldFilename.size() + newfileName.size() + 2) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->oldName = (String::value_type*)bytes;
		bytes += (oldFilename.size() + 1) * sizeof(String::value_type);

		action->newName = (String::value_type*)bytes;
		action->type = FileActionType::Modified;

		memcpy(action->oldName, oldFilename.data(), oldFilename.size() * sizeof(String::value_type));
		action->oldName[oldFilename.size()] = L'\0';

		memcpy(action->newName, newfileName.data(), newfileName.size() * sizeof(String::value_type));
		action->newName[newfileName.size()] = L'\0';
		action->lastSize = 0;
		action->checkForWriteStarted = false;

		return action;
	}

	static void Destroy(FileAction* action)
	{
		B3DFree(action);
	}

	String::value_type* oldName;
	String::value_type* newName;
	FileActionType type;

	u64 lastSize;
	bool checkForWriteStarted;
};

struct FolderMonitor::Pimpl
{
	LinuxFolderMonitor* LowLevelMonitor = nullptr;

	Vector<FileAction*> FileActions;
	Vector<FileAction*> ActiveFileActions;

	int INotifyHandle = 0;
	bool Started = false;
	Mutex MainMutex;
	Thread* WorkerThread = nullptr;
};

FolderMonitor::FolderMonitor(const Path& folderPath, bool monitorSubdirectories, FolderChangeFlags changeFilter)
{
	m = B3DNew<Pimpl>();

	if(!FileSystem::IsDirectory(folderPath))
	{
		B3D_LOG(Error, LogPlatform, "Provided path \"{0}\" is not a directory", folderPath);
		return;
	}

	// Initialize inotify
	{
		Lock lock(m->MainMutex);

		m->INotifyHandle = inotify_init();
		m->Started = true;
	}

	LinuxFolderMonitor* lowLeveMonitor = B3DNew<LinuxFolderMonitor>(folderPath, m->INotifyHandle, monitorSubdirectories, changeFilter);

	// Register and start the monitor
	{
		Lock lock(m->mainMutex);

		m->LowLevelMonitor = lowLevelMonitor;
		LowLevelMonitor->StartMonitor();
	}

	// Start the worker thread
	m->WorkerThread = B3DNew<Thread>(std::bind(&FolderMonitor::WorkerThreadMain, this));

	if(m->WorkerThread == nullptr)
	{
		B3D_LOG(Error, LogPlatform, "Failed to create a new worker thread for folder monitoring");
		return;
	}

	FolderMonitorManager::Instance().RegisterMonitor(this);
}

FolderMonitor::~FolderMonitor()
{
	FolderMonitorManager::Instance().UnregisterMonitor(this);

	{
		Lock lock(m->MainMutex);

		// First tell the thread it's ready to be shutdown
		m->Started = false;

		// Remove the low level monitor (this will also wake up the thread).
		if(m->LowLevelMonitor != nullptr)
		{
			m->LowLevelMonitor->StopMonitor();

			B3DDelete(m->LowLevelMonitor);
			m->LowLevelMonitor = nullptr;
		}
	}

	// Wait for the thread to shutdown
	if(m->WorkerThread != nullptr)
	{
		m->WorkerThread->join();

		B3DDelete(m->WorkerThread);
		m->WorkerThread = nullptr;
	}

	// Close the inotify handle
	{
		Lock lock(m->MainMutex);
		if(m->INotifyHandle != 0)
		{
			close(m->INotifyHandle);
			m->INotifyHandle = 0;
		}
	}

	// No need for mutex since we know worker thread is shut down by now
	for(auto& action : m->FileActions)
		FileAction::Destroy(action);

	B3DDelete(m);
}

void FolderMonitor::WorkerThreadMain()
{
	static const u32 kBufferSize = 16384;

	bool shouldRun;
	i32 watchHandle;
	{
		Lock(m->MainMutex);
		watchHandle = m->INotifyHandle;
		shouldRun = m->Started;
	}

	u8 buffer[kBufferSize];

	while(shouldRun)
	{
		i32 length = (i32)read(watchHandle, buffer, sizeof(buffer));

		// Handle was closed, shutdown thread
		if(length < 0)
			return;

		// Note: Must be after read, so shutdown can be started when we remove the watches (as then read() will return)
		{
			Lock(m->MainMutex);
			shouldRun = m->Started;
		}

		i32 readPos = 0;
		while(readPos < length)
		{
			inotify_event* event = (inotify_event*)&buffer[readPos];
			if(event->len > 0)
			{
				{
					Lock lock(m->MainMutex);

					Path path;
					LinuxFolderMonitor* lowLevelMonitor = nullptr;
					if(m->LowLevelMonitor != nullptr)
					{
						path = m->LowLevelMonitor->GetPath(event->wd);
						if(!path.isEmpty())
						{
							path.append(event->name);
							lowLevelMonitor = m->LowLevelMonitor;
							break;
						}
					}

					// This can happen if the path got removed during some recent previous event
					if(lowLevelMonitor == nullptr)
						goto next;

					// Need to add/remove sub-directories to/from watch list
					bool isDirectory = (event->mask & IN_ISDIR) != 0;
					if(isDirectory && monitor->monitorSubdirectories)
					{
						bool added = (event->mask & (IN_CREATE | IN_MOVED_TO)) != 0;
						bool removed = (event->mask & (IN_DELETE | IN_MOVED_FROM)) != 0;

						if(added)
							lowLevelMonitor->AddPath(path);
						else if(removed)
							lowLevelMonitor->RemovePath(path);
					}

					// Actually trigger the events

					// File/folder was added
					if(((event->mask & (IN_CREATE | IN_MOVED_TO)) != 0))
					{
						if(isDirectory)
						{
							if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::DirectoryAddedOrRemoved))
								m->FileActions.push_back(FileAction::CreateAdded(path.ToString()));
						}
						else
						{
							if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::FileAddedOrRemoved))
								m->FileActions.push_back(FileAction::CreateAdded(path.ToString()));
						}
					}

					// File/folder was removed
					if(((event->mask & (IN_DELETE | IN_MOVED_FROM)) != 0))
					{
						if(isDirectory)
						{
							if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::DirectoryAddedOrRemoved))
								m->FileActions.push_back(FileAction::CreateRemoved(path.ToString()));
						}
						else
						{
							if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::FileAddedOrRemoved))
								m->FileActions.push_back(FileAction::CreateRemoved(path.ToString()));
						}
					}

					// File was modified
					if(((event->mask & IN_CLOSE_WRITE) != 0) && lowLevelMonitor->Filter.IsSet(FolderChangeFlag::FileWritten))
					{
						m->FileActions.push_back(FileAction::CreateModified(path.ToString()));
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

void FolderMonitor::HandleNotifications(FileNotifyInfo& notifyInfo)
{
	// Do nothing
}

void FolderMonitor::Update()
{
	{
		Lock lock(m->MainMutex);

		std::swap(m->FileActions, m->ActiveFileActions);
	}

	for(auto& action : m->ActiveFileActions)
	{
		switch(action->type)
		{
		case FileActionType::Added:
			if(!OnAdded.empty())
				OnAdded(Path(action->newName));
			break;
		case FileActionType::Removed:
			if(!OnRemoved.empty())
				OnRemoved(Path(action->newName));
			break;
		case FileActionType::Modified:
			if(!OnModified.empty())
				OnModified(Path(action->newName));
			break;
		case FileActionType::Renamed:
			if(!OnRenamed.empty())
				OnRenamed(Path(action->oldName), Path(action->newName));
			break;
		}

		FileAction::Destroy(action);
	}

	m->ActiveFileActions.clear();
}
