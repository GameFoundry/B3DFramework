//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/B3DFolderMonitor.h"
#include "FileSystem/B3DFileSystem.h"

#include <windows.h>
#include "String/B3DUnicode.h"
#include "Threading/B3DThread.h"

namespace b3d
{
	class Thread;
}

using namespace b3d;

enum class MonitorState
{
	Inactive,
	Starting,
	Monitoring,
	Shutdown,
	Shutdown2
};

class WorkerFunc
{
public:
	WorkerFunc(FolderMonitor* owner);

	void operator()();

private:
	FolderMonitor* mOwner;
};

struct Win32FolderMonitor
{
	Win32FolderMonitor(const Path& folderToMonitor, HANDLE dirHandle, bool monitorSubdirectories, DWORD monitorFlags);
	~Win32FolderMonitor();

	void StartMonitor(HANDLE compPortHandle);
	void StopMonitor(HANDLE compPortHandle);

	static const u32 kReadBufferSize = 65536;

	Path FolderToMonitor;
	HANDLE DirectoryHandle;
	OVERLAPPED Overlapped;
	MonitorState State;
	u8 Buffer[kReadBufferSize];
	DWORD BufferSize;
	bool MonitorSubdirectories;
	DWORD MonitorFlags;
	DWORD ReadError;

	WString CachedOldFileName; // Used during rename notifications as they are handled in two steps

	Mutex StatusMutex;
	ConditionVariable StartStopEvent;
};

Win32FolderMonitor::Win32FolderMonitor(const Path& folderToMonitor, HANDLE dirHandle, bool monitorSubdirectories, DWORD monitorFlags)
	: FolderToMonitor(folderToMonitor), DirectoryHandle(dirHandle), State(MonitorState::Inactive), BufferSize(0), MonitorSubdirectories(monitorSubdirectories), MonitorFlags(monitorFlags), ReadError(0)
{
	memset(&Overlapped, 0, sizeof(Overlapped));
}

Win32FolderMonitor::~Win32FolderMonitor()
{
	B3D_ASSERT(State == MonitorState::Inactive);

	StopMonitor(0);
}

void Win32FolderMonitor::StartMonitor(HANDLE compPortHandle)
{
	if(State != MonitorState::Inactive)
		return; // Already monitoring

	{
		Lock lock(StatusMutex);

		State = MonitorState::Starting;
		PostQueuedCompletionStatus(compPortHandle, sizeof(this), (ULONG_PTR)this, &Overlapped);

		while(State != MonitorState::Monitoring)
			StartStopEvent.wait(lock);
	}

	if(ReadError != ERROR_SUCCESS)
	{
		{
			Lock lock(StatusMutex);
			State = MonitorState::Inactive;
		}

		B3D_LOG(Error, LogGeneric, "Failed to start folder monitor on folder \"{0}\" because ReadDirectoryChangesW failed.", FolderToMonitor);
		return;
	}
}

void Win32FolderMonitor::StopMonitor(HANDLE compPortHandle)
{
	if(State != MonitorState::Inactive)
	{
		Lock lock(StatusMutex);

		State = MonitorState::Shutdown;
		PostQueuedCompletionStatus(compPortHandle, sizeof(this), (ULONG_PTR)this, &Overlapped);

		while(State != MonitorState::Inactive)
			StartStopEvent.wait(lock);
	}

	if(DirectoryHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DirectoryHandle);
		DirectoryHandle = INVALID_HANDLE_VALUE;
	}
}

class FolderMonitor::FileNotifyInfo
{
public:
	FileNotifyInfo(u8* notifyBuffer, DWORD bufferSize)
		: mBuffer(notifyBuffer), mBufferSize(bufferSize)
	{
		mCurrentRecord = (PFILE_NOTIFY_INFORMATION)mBuffer;
	}

	bool GetNext();

	DWORD GetAction() const;
	WString GetFileName() const;
	WString GetFileNameWithPath(const Path& rootPath) const;

protected:
	u8* mBuffer;
	DWORD mBufferSize;
	PFILE_NOTIFY_INFORMATION mCurrentRecord;
};

bool FolderMonitor::FileNotifyInfo::GetNext()
{
	if(mCurrentRecord && mCurrentRecord->NextEntryOffset != 0)
	{
		PFILE_NOTIFY_INFORMATION oldRecord = mCurrentRecord;
		mCurrentRecord = (PFILE_NOTIFY_INFORMATION)((u8*)mCurrentRecord + mCurrentRecord->NextEntryOffset);

		if((DWORD)((u8*)mCurrentRecord - mBuffer) > mBufferSize)
		{
			// Gone out of range, something bad happened
			B3D_ASSERT(false);

			mCurrentRecord = oldRecord;
		}

		return (mCurrentRecord != oldRecord);
	}

	return false;
}

DWORD FolderMonitor::FileNotifyInfo::GetAction() const
{
	B3D_ASSERT(mCurrentRecord != nullptr);

	if(mCurrentRecord)
		return mCurrentRecord->Action;

	return 0;
}

WString FolderMonitor::FileNotifyInfo::GetFileName() const
{
	if(mCurrentRecord)
	{
		wchar_t fileNameBuffer[32768 + 1] = { 0 };

		memcpy(fileNameBuffer, mCurrentRecord->FileName, std::min(DWORD(32768 * sizeof(wchar_t)), mCurrentRecord->FileNameLength));

		return WString(fileNameBuffer);
	}

	return WString();
}

WString FolderMonitor::FileNotifyInfo::GetFileNameWithPath(const Path& rootPath) const
{
	Path fullPath = rootPath;
	fullPath.Append(UTF8::FromWide(GetFileName()));

	return UTF8::ToWide(fullPath.ToString());
}

enum class FileActionType
{
	Added,
	Removed,
	Modified,
	Renamed
};

struct FileAction
{
	static FileAction* CreateAdded(const WString& fileName)
	{
		String utf8filename = UTF8::FromWide(fileName);
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (utf8filename.size() + 1) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->OldName = nullptr;
		action->NewName = (String::value_type*)bytes;
		action->Type = FileActionType::Added;

		memcpy(action->NewName, utf8filename.data(), utf8filename.size() * sizeof(String::value_type));
		action->NewName[utf8filename.size()] = L'\0';
		action->LastSize = 0;
		action->CheckForWriteStarted = false;

		return action;
	}

	static FileAction* CreateRemoved(const WString& fileName)
	{
		String utf8filename = UTF8::FromWide(fileName);
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (utf8filename.size() + 1) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->OldName = nullptr;
		action->NewName = (String::value_type*)bytes;
		action->Type = FileActionType::Removed;

		memcpy(action->NewName, utf8filename.data(), utf8filename.size() * sizeof(String::value_type));
		action->NewName[utf8filename.size()] = L'\0';
		action->LastSize = 0;
		action->CheckForWriteStarted = false;

		return action;
	}

	static FileAction* CreateModified(const WString& fileName)
	{
		String utf8filename = UTF8::FromWide(fileName);
		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (utf8filename.size() + 1) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->OldName = nullptr;
		action->NewName = (String::value_type*)bytes;
		action->Type = FileActionType::Modified;

		memcpy(action->NewName, utf8filename.data(), utf8filename.size() * sizeof(String::value_type));
		action->NewName[utf8filename.size()] = L'\0';
		action->LastSize = 0;
		action->CheckForWriteStarted = false;

		return action;
	}

	static FileAction* CreateRenamed(const WString& oldFilename, const WString& newFileName)
	{
		String utf8Oldfilename = UTF8::FromWide(oldFilename);
		String utf8Newfilename = UTF8::FromWide(newFileName);

		u8* bytes = (u8*)B3DAllocate((u32)(sizeof(FileAction) + (utf8Oldfilename.size() + utf8Newfilename.size() + 2) * sizeof(String::value_type)));

		FileAction* action = (FileAction*)bytes;
		bytes += sizeof(FileAction);

		action->OldName = (String::value_type*)bytes;
		bytes += (utf8Oldfilename.size() + 1) * sizeof(String::value_type);

		action->NewName = (String::value_type*)bytes;
		action->Type = FileActionType::Modified;

		memcpy(action->OldName, utf8Oldfilename.data(), utf8Oldfilename.size() * sizeof(String::value_type));
		action->OldName[utf8Oldfilename.size()] = L'\0';

		memcpy(action->NewName, utf8Newfilename.data(), utf8Newfilename.size() * sizeof(String::value_type));
		action->NewName[utf8Newfilename.size()] = L'\0';
		action->LastSize = 0;
		action->CheckForWriteStarted = false;

		return action;
	}

	static void Destroy(FileAction* action)
	{
		B3DFree(action);
	}

	String::value_type* OldName;
	String::value_type* NewName;
	FileActionType Type;

	u64 LastSize;
	bool CheckForWriteStarted;
};

struct FolderMonitor::Pimpl
{
	Win32FolderMonitor* LowLevelMonitor = nullptr;
	HANDLE CompPortHandle;

	Queue<FileAction*> FileActions;
	List<FileAction*> ActiveFileActions;

	Mutex MainMutex;
	Thread* WorkerThread;
};

FolderMonitor::FolderMonitor(const Path& folderPath, bool monitorSubdirectories, FolderChangeFlags changeFilter)
{
	m = B3DNew<Pimpl>();
	m->WorkerThread = nullptr;
	m->CompPortHandle = nullptr;

	if(!FileSystem::IsFolder(folderPath))
	{
		B3D_LOG(Error, LogGeneric, "Provided path \"{0}\" is not a directory", folderPath);
		return;
	}

	WString extendedFolderPath = L"\\\\?\\" + UTF8::ToWide(folderPath.ToString(Path::PathType::Windows));
	HANDLE dirHandle = CreateFileW(extendedFolderPath.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

	if(dirHandle == INVALID_HANDLE_VALUE)
	{
		B3D_LOG(Error, LogGeneric, "Failed to open folder \"{0}\" for monitoring. Error code: {1}", folderPath, (u64)GetLastError());
		return;
	}

	DWORD filterFlags = 0;

	if(changeFilter.IsSet(FolderChangeFlag::FileAddedOrRemoved))
		filterFlags |= FILE_NOTIFY_CHANGE_FILE_NAME;

	if(changeFilter.IsSet(FolderChangeFlag::DirectoryAddedOrRemoved))
		filterFlags |= FILE_NOTIFY_CHANGE_DIR_NAME;

	if(changeFilter.IsSet(FolderChangeFlag::FileWritten))
		filterFlags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

	m->LowLevelMonitor = B3DNew<Win32FolderMonitor>(folderPath, dirHandle, monitorSubdirectories, filterFlags);

	m->CompPortHandle = CreateIoCompletionPort(dirHandle, m->CompPortHandle, (ULONG_PTR)m->LowLevelMonitor, 0);

	if(m->CompPortHandle == nullptr)
	{
		B3DDelete(m->LowLevelMonitor);
		m->LowLevelMonitor = nullptr;

		B3D_LOG(Error, LogGeneric, "Failed to open completion port for folder monitoring. Error code: {0}", (u64)GetLastError());
		return;
	}

	if(m->WorkerThread == nullptr)
	{
		m->WorkerThread = B3DNew<Thread>([this]() { WorkerThreadMain(); });

		if(m->WorkerThread == nullptr)
		{
			B3DDelete(m->LowLevelMonitor);
			m->LowLevelMonitor = nullptr;

			B3D_LOG(Error, LogGeneric, "Failed to create a new worker thread for folder monitoring");
			return;
		}
	}

	if(m->WorkerThread != nullptr)
	{
		m->LowLevelMonitor->StartMonitor(m->CompPortHandle);
	}
	else
	{
		B3DDelete(m->LowLevelMonitor);
		m->LowLevelMonitor = nullptr;

		B3D_LOG(Error, LogGeneric, "Failed to create a new worker thread for folder monitoring");
		return;
	}

	FolderMonitorManager::Instance().RegisterMonitor(this);
}

FolderMonitor::~FolderMonitor()
{
	FolderMonitorManager::Instance().UnregisterMonitor(this);

	if(m->LowLevelMonitor != nullptr)
	{
		m->LowLevelMonitor->StopMonitor(m->CompPortHandle);

		{
			// Note: Need this mutex to ensure worker thread is done with watchInfo.
			// Even though we wait for a condition variable from the worker thread in stopMonitor,
			// that doesn't mean the worker thread is done with the condition variable
			// (which is stored inside watchInfo)
			Lock lock(m->MainMutex);
			B3DDelete(m->LowLevelMonitor);
			m->LowLevelMonitor = nullptr;
		}
	}

	if(m->WorkerThread != nullptr)
	{
		PostQueuedCompletionStatus(m->CompPortHandle, 0, 0, nullptr);

		m->WorkerThread->WaitUntilComplete();
		B3DDelete(m->WorkerThread);
		m->WorkerThread = nullptr;
	}

	if(m->CompPortHandle != nullptr)
	{
		CloseHandle(m->CompPortHandle);
		m->CompPortHandle = nullptr;
	}

	// No need for mutex since we know worker thread is shut down by now
	while(!m->FileActions.empty())
	{
		FileAction* action = m->FileActions.front();
		m->FileActions.pop();

		FileAction::Destroy(action);
	}

	B3DDelete(m);
}

void FolderMonitor::WorkerThreadMain()
{
	Win32FolderMonitor* lowLevelMonitor = nullptr;

	do
	{
		DWORD numBytes;
		LPOVERLAPPED overlapped;

		if(!GetQueuedCompletionStatus(m->CompPortHandle, &numBytes, (PULONG_PTR)&lowLevelMonitor, &overlapped, INFINITE))
		{
			B3D_ASSERT(false);
			// TODO: Folder handle was lost most likely. Not sure how to deal with that. Shutdown watch on this folder and cleanup?
		}

		if(lowLevelMonitor != nullptr)
		{
			MonitorState state;

			{
				Lock lock(lowLevelMonitor->StatusMutex);
				state = lowLevelMonitor->State;
			}

			switch(state)
			{
			case MonitorState::Starting:
				if(!ReadDirectoryChangesW(lowLevelMonitor->DirectoryHandle, lowLevelMonitor->Buffer, Win32FolderMonitor::kReadBufferSize, lowLevelMonitor->MonitorSubdirectories, lowLevelMonitor->MonitorFlags, &lowLevelMonitor->BufferSize, &lowLevelMonitor->Overlapped, nullptr))
				{
					B3D_ASSERT(false); // TODO - Possibly the buffer was too small?
					lowLevelMonitor->ReadError = GetLastError();
				}
				else
				{
					lowLevelMonitor->ReadError = ERROR_SUCCESS;

					{
						Lock lock(lowLevelMonitor->StatusMutex);
						lowLevelMonitor->State = MonitorState::Monitoring;
					}
				}

				lowLevelMonitor->StartStopEvent.notify_one();

				break;
			case MonitorState::Monitoring:
				{
					FileNotifyInfo info(lowLevelMonitor->Buffer, Win32FolderMonitor::kReadBufferSize);
					HandleNotifications(info);

					if(!ReadDirectoryChangesW(lowLevelMonitor->DirectoryHandle, lowLevelMonitor->Buffer, Win32FolderMonitor::kReadBufferSize, lowLevelMonitor->MonitorSubdirectories, lowLevelMonitor->MonitorFlags, &lowLevelMonitor->BufferSize, &lowLevelMonitor->Overlapped, nullptr))
					{
						B3D_ASSERT(false); // TODO: Failed during normal operation, possibly the buffer was too small. Shutdown watch on this folder and cleanup?
						lowLevelMonitor->ReadError = GetLastError();
					}
					else
					{
						lowLevelMonitor->ReadError = ERROR_SUCCESS;
					}
				}
				break;
			case MonitorState::Shutdown:
				if(lowLevelMonitor->DirectoryHandle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(lowLevelMonitor->DirectoryHandle);
					lowLevelMonitor->DirectoryHandle = INVALID_HANDLE_VALUE;

					{
						Lock lock(lowLevelMonitor->StatusMutex);
						lowLevelMonitor->State = MonitorState::Shutdown2;
					}
				}
				else
				{
					{
						Lock lock(lowLevelMonitor->StatusMutex);
						lowLevelMonitor->State = MonitorState::Inactive;
					}

					{
						Lock lock(m->MainMutex); // Ensures that we don't delete "watchInfo" before this thread is done with mStartStopEvent
						lowLevelMonitor->StartStopEvent.notify_one();
					}
				}

				break;
			case MonitorState::Shutdown2:
				if(lowLevelMonitor->DirectoryHandle != INVALID_HANDLE_VALUE)
				{
					// Handle is still open? Try again.
					CloseHandle(lowLevelMonitor->DirectoryHandle);
					lowLevelMonitor->DirectoryHandle = INVALID_HANDLE_VALUE;
				}
				else
				{
					{
						Lock lock(lowLevelMonitor->StatusMutex);
						lowLevelMonitor->State = MonitorState::Inactive;
					}

					{
						Lock lock(m->MainMutex); // Ensures that we don't delete "watchInfo" before this thread is done with mStartStopEvent
						lowLevelMonitor->StartStopEvent.notify_one();
					}
				}

				break;
			default:
				break;
			}
		}
	}
	while(lowLevelMonitor != nullptr);
}

void FolderMonitor::HandleNotifications(FileNotifyInfo& notifyInfo)
{
	Vector<FileAction*> mActions;

	do
	{
		WString fullPath = notifyInfo.GetFileNameWithPath(m->LowLevelMonitor->FolderToMonitor);

		// Ignore notifications about hidden files
		if(notifyInfo.GetAction() != FILE_ACTION_REMOVED && (GetFileAttributesW(fullPath.c_str()) & FILE_ATTRIBUTE_HIDDEN) != 0)
			continue;

		switch(notifyInfo.GetAction())
		{
		case FILE_ACTION_ADDED:
			mActions.push_back(FileAction::CreateAdded(fullPath));
			break;
		case FILE_ACTION_REMOVED:
			mActions.push_back(FileAction::CreateRemoved(fullPath));
			break;
		case FILE_ACTION_MODIFIED:
			mActions.push_back(FileAction::CreateModified(fullPath));
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			m->LowLevelMonitor->CachedOldFileName = fullPath;
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			mActions.push_back(FileAction::CreateRenamed(m->LowLevelMonitor->CachedOldFileName, fullPath));
			break;
		}
	}
	while(notifyInfo.GetNext());

	{
		Lock lock(m->MainMutex);

		for(auto& action : mActions)
			m->FileActions.push(action);
	}
}

void FolderMonitor::Update()
{
	{
		Lock lock(m->MainMutex);

		while(!m->FileActions.empty())
		{
			FileAction* action = m->FileActions.front();
			m->FileActions.pop();

			m->ActiveFileActions.push_back(action);
		}
	}

	for(auto iter = m->ActiveFileActions.begin(); iter != m->ActiveFileActions.end();)
	{
		FileAction* action = *iter;

		// Reported file actions might still be in progress (i.e. something might still be writing to those files).
		// Sadly there doesn't seem to be a way to properly determine when those files are done being written, so instead
		// we check for at least a couple of frames if the file's size hasn't changed before reporting a file action.
		// This takes care of most of the issues and avoids reporting partially written files in almost all cases.
		if(FileSystem::Exists(action->NewName))
		{
			u64 size = FileSystem::GetFileSize(action->NewName);
			if(!action->CheckForWriteStarted)
			{
				action->CheckForWriteStarted = true;
				action->LastSize = size;

				++iter;
				continue;
			}
			else
			{
				if(action->LastSize != size)
				{
					action->LastSize = size;
					++iter;
					continue;
				}
			}
		}

		switch(action->Type)
		{
		case FileActionType::Added:
			if(!OnAdded.Empty())
				OnAdded(Path(action->NewName));
			break;
		case FileActionType::Removed:
			if(!OnRemoved.Empty())
				OnRemoved(Path(action->NewName));
			break;
		case FileActionType::Modified:
			if(!OnModified.Empty())
				OnModified(Path(action->NewName));
			break;
		case FileActionType::Renamed:
			if(!OnRenamed.Empty())
				OnRenamed(Path(action->OldName), Path(action->NewName));
			break;
		}

		m->ActiveFileActions.erase(iter++);
		FileAction::Destroy(action);
	}
}
