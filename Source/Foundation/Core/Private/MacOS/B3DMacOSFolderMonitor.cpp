//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/B3DFolderMonitor.h"
#include "FileSystem/B3DFileSystem.h"
#include "Error/B3DException.h"
#include "Utility/B3DTimer.h"

#include <CoreServices/CoreServices.h>

using namespace b3d;

static constexpr u32 WRITE_STEADY_WAIT = 2000;
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
	u64 lastSize;
	Timer timer;
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
		action->newName[fileName.size()] = '\0';

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
		action->newName[fileName.size()] = '\0';

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
		action->newName[fileName.size()] = '\0';

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
		action->oldName[oldFilename.size()] = '\0';

		memcpy(action->newName, newfileName.data(), newfileName.size() * sizeof(String::value_type));
		action->newName[newfileName.size()] = '\0';

		return action;
	}

	static void Destroy(FileAction* action)
	{
		B3DFree(action);
	}

	String::value_type* oldName;
	String::value_type* newName;
	FileActionType type;
};

struct MacOSFolderMonitor;

struct FolderMonitor::Pimpl
{
	MacOSFolderMonitor* LowLevelMonitor = nullptr;
	bool RequestLowLevelMonitorStart = false;
	bool RequestLowLevelMonitorStop = false;

	Vector<FileAction*> FileActions;
	Vector<FileAction*> ActiveFileActions;

	Mutex MainMutex;
	Thread* WorkerThread = nullptr;
};

static void WatcherCallback(ConstFSEventStreamRef streamRef, void* userInfo, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds);

struct MacOSFolderMonitor
{
	MacOSFolderMonitor(
		const Path& folderToMonitor,
		FolderMonitor* owner,
		bool monitorSubdirectories,
		FolderChangeFlags filter);
	~MacOSFolderMonitor();

	void StartMonitor();
	void StopMonitor();

	Path FolderToMonitor;
	FolderMonitor* Owner;
	bool MonitorSubdirectories;
	FolderChangeFlags Filter;
	FSEventStreamRef StreamRef;
	bool HasStarted;
	Vector<CreatedFileInfo> CreatedFiles;
};

MacOSFolderMonitor::MacOSFolderMonitor(
	const Path& folderToMonitor,
	FolderMonitor* owner,
	bool monitorSubdirectories,
	FolderChangeFlags filter)
	: FolderToMonitor(folderToMonitor)
	, Owner(owner)
	, MonitorSubdirectories(monitorSubdirectories)
	, Filter(filter)
	, StreamRef(nullptr)
	, HasStarted(false)
{}

MacOSFolderMonitor::~MacFolderMonitor()
{
	StopMonitor();
}

void MacOSFolderMonitor::StartMonitor()
{
	String pathString = FolderToMonitor.toString();
	CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, pathString.c_str(), kCFStringEncodingUTF8);

	CFArrayRef pathArray = CFArrayCreate(nullptr, (const void**)&path, 1, nullptr);
	FSEventStreamContext context = {};
	context.info = this;

	CFAbsoluteTime latency = 0.1f;
	StreamRef = FSEventStreamCreate(
		kCFAllocatorDefault,
		&WatcherCallback,
		&context,
		pathArray,
		kFSEventStreamEventIdSinceNow,
		latency,
		kFSEventStreamCreateFlagUseCFTypes | kFSEventStreamCreateFlagFileEvents);

	CFRelease(pathArray);

	if(StreamRef)
	{
		FSEventStreamScheduleWithRunLoop(StreamRef, CFRunLoopGetCurrent(), FolderMonitorMode);
		if(FSEventStreamStart(StreamRef))
			HasStarted = true;
	}

	CFRelease(path);
}

void MacOSFolderMonitor::StopMonitor()
{
	if(!StreamRef)
		return;

	if(HasStarted)
	{
		FSEventStreamStop(StreamRef);
		HasStarted = false;
	}

	FSEventStreamInvalidate(StreamRef);
	FSEventStreamRelease(StreamRef);
}

static void WatcherCallback(ConstFSEventStreamRef streamRef, void* userInfo, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags* eventFlags, const FSEventStreamEventId* eventIds)
{
	auto* lowLevelMonitor = (MacOSFolderMonitor*)userInfo;
	FolderMonitor::Pimpl* folderData = lowLevelMonitor->Owner->GetPrivateDataInternal();

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

		Lock lock(folderData->MainMutex);

		// If not monitoring subdirectories, ignore paths that aren't direct descendants of the root path
		if(!lowLevelMonitor->MonitorSubdirectories)
		{
			if(path.getParent() != lowLevelMonitor->FolderToMonitor)
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
			if(FileSystem::Exists(path))
				FolderData->FileActions.push_back(FileAction::CreateAdded(path.ToString()));
			else
				FolderData->FileActions.push_back(FileAction::CreateRemoved(path.ToString()));
		}

		// File/folder was added
		if(wasCreated)
		{
			if(!isFile)
			{
				if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::DirectoryAddedOrRemoved))
					FolderData->FileActions.push_back(FileAction::CreateAdded(path.ToString()));
			}
			else
			{
				if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::FileAddedOrRemoved))
				{
					// We delay all file creation events until the file is done writing
					lowLevelMonitor->CreatedFiles.push_back(CreatedFileInfo());
					CreatedFileInfo& createdFileInfo = lowLevelMonitor->createdFiles.back();
					createdFileInfo.path = path;
					createdFileInfo.lastSize = FileSystem::GetFileSize(path);
					createdFileInfo.timer.reset();
				}
			}
		}

		// File/folder was removed
		if(wasRemoved)
		{
			if(!isFile)
			{
				if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::DirectoryAddedOrRemoved))
					FolderData->FileActions.push_back(FileAction::CreateRemoved(path.ToString()));
			}
			else
			{
				if(lowLevelMonitor->Filter.IsSet(FolderChangeFlag::FileAddedOrRemoved))
					FolderData->FileActions.push_back(FileAction::CreateRemoved(path.ToString()));
			}
		}

		// File was modified
		if(wasModified && lowLevelMonitor->Filter.IsSet(FolderChangeFlag::FileWritten))
		{
			// Don't send out modified event if file was created
			auto found = std::find_if(lowLevelMonitor->CreatedFiles.begin(), lowLevelMonitor->CreatedFiles.end(), [&path](const CreatedFileInfo& info) { return info.path == path; });

			if(found == lowLevelMonitor->CreatedFiles.end())
				folderData->FileActions.push_back(FileAction::CreateModified(path.ToString()));
		}
	}
}

class FolderMonitor::FileNotifyInfo
{
};

FolderMonitor::FolderMonitor(const Path& folderPath, bool monitorSubdirectories, FolderChangeFlags changeFilter)
{
	m = B3DNew<Pimpl>();

	if(!FileSystem::IsDirectory(folderPath))
	{
		B3D_LOG(Error, Platform, "Provided path \"{0}\" is not a directory.", folderPath);
		return;
	}

	auto lowLevelMonitor = B3DNew<MacOSFolderMonitor>(folderPath, this, monitorSubdirectories, changeFilter);

	// Register and start the monitor
	{
		Lock lock(m->MainMutex);

		m->LowLevelMonitor = lowLevelMonitor;
		m->RequestLowLevelMonitorStart = true;
	}

	// Start the worker thread
	m->WorkerThread = B3DNew<Thread>(std::bind(&FolderMonitor::WorkerThreadMain, this));

	if(m->WorkerThread == nullptr)
	{
		B3D_LOG(Error, Platform, "Failed to create a new worker thread for folder monitoring");
		return;
	}

	FolderMonitorManager::Instance().RegisterMonitor(this);
}

FolderMonitor::~FolderMonitor()
{
	FolderMonitorManager::Instance().UnregisterMonitor(this);

	{
		Lock lock(m->MainMutex);

		m->RequestLowLevelMonitorStop = true;
	}

	// Wait for the thread to shutdown
	if(m->WorkerThread != nullptr)
	{
		m->WorkerThread->join();

		B3DDelete(m->WorkerThread);
		m->WorkerThread = nullptr;
	}

	// No need for mutex since we know worker thread is shut down by now
	for(auto& action : m->FileActions)
		FileAction::Destroy(action);

	B3DDelete(m);
}

void FolderMonitor::WorkerThreadMain()
{
	MemStack::BeginThread();

	while(true)
	{
		// Start up low level monitor if needed
		{
			Lock lock(m->MainMutex);

			if(m->RequestLowLevelMonitorStart)
			{
				m->LowLevelMonitor->StartMonitor();
				m->RequestLowLevelMonitorStart = false;
			}
		}

		// Run the loop in order to receive events
		i32 result = CFRunLoopRunInMode(FolderMonitorMode, 0.1f, false);

		// Delete low level monitor if needed
		{
			Lock lock(m->MainMutex);

			if(m->RequestLowLevelMonitorStop)
			{
				B3DDelete(m->LowLevelMonitor);
				m->LowLevelMonitor = nullptr;

				m->RequestLowLevelMonitorStop = false
			}
		}

		// All input sources removed, or explicitly stopped, bail
		if((result == kCFRunLoopRunStopped) || (result == kCFRunLoopRunFinished))
			break;

		// Check if any created files have completed writing, and handle rename events
		{
			Lock lock(m->MainMutex);

			if(m->LowLevelMonitor != nullptr)
			{
				FolderMonitor::Pimpl* folderData = m->LowLevelMonitor->Owner->GetPrivateDataInternal();

				for(auto iter = m->LowLevelMonitor->CreatedFiles.begin(); iter != m->LowLevelMonitor->CreatedFiles.end();)
				{
					CreatedFileInfo& entry = *iter;

					u64 fileSize = FileSystem::GetFileSize(entry.path);
					if(fileSize != entry.lastSize)
					{
						entry.lastSize = fileSize;
						entry.timer.reset();
					}

					if(entry.timer.getMilliseconds() > WRITE_STEADY_WAIT)
					{
						folderData->FileActions.push_back(FileAction::CreateAdded(entry.path.ToString()));
						iter = m->lowLevelMonitor->CreatedFiles.erase(iter);
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
			Lock lock(m->MainMutex);

			if(m->LowLevelMonitor == nullptr)
				break;
		}
	}

	MemStack::EndThread();
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
