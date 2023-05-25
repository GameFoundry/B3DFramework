//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsFolderMonitor.h"
#include "FileSystem/BsFileSystem.h"
#include "Error/BsException.h"

#include <windows.h>
#include "String/BsUnicode.h"
#include "Threading/BsThread.h"

namespace bs
{
	class Thread;
}

using namespace bs;

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

struct FolderMonitor::FolderWatchInfo
{
	FolderWatchInfo(const Path& folderToMonitor, HANDLE dirHandle, bool monitorSubdirectories, DWORD monitorFlags);
	~FolderWatchInfo();

	void StartMonitor(HANDLE compPortHandle);
	void StopMonitor(HANDLE compPortHandle);

	static const u32 kReadBufferSize = 65536;

	Path MFolderToMonitor;
	HANDLE MDirHandle;
	OVERLAPPED MOverlapped;
	MonitorState MState;
	u8 MBuffer[kReadBufferSize];
	DWORD MBufferSize;
	bool MMonitorSubdirectories;
	DWORD MMonitorFlags;
	DWORD MReadError;

	WString MCachedOldFileName; // Used during rename notifications as they are handled in two steps

	Mutex MStatusMutex;
	ConditionVariable MStartStopEvent;
};

FolderMonitor::FolderWatchInfo::FolderWatchInfo(const Path& folderToMonitor, HANDLE dirHandle, bool monitorSubdirectories, DWORD monitorFlags)
	: MFolderToMonitor(folderToMonitor), MDirHandle(dirHandle), MState(MonitorState::Inactive), MBufferSize(0), MMonitorSubdirectories(monitorSubdirectories), MMonitorFlags(monitorFlags), MReadError(0)
{
	memset(&MOverlapped, 0, sizeof(MOverlapped));
}

FolderMonitor::FolderWatchInfo::~FolderWatchInfo()
{
	B3D_ASSERT(MState == MonitorState::Inactive);

	StopMonitor(0);
}

void FolderMonitor::FolderWatchInfo::StartMonitor(HANDLE compPortHandle)
{
	if(MState != MonitorState::Inactive)
		return; // Already monitoring

	{
		Lock lock(MStatusMutex);

		MState = MonitorState::Starting;
		PostQueuedCompletionStatus(compPortHandle, sizeof(this), (ULONG_PTR)this, &MOverlapped);

		while(MState != MonitorState::Monitoring)
			MStartStopEvent.wait(lock);
	}

	if(MReadError != ERROR_SUCCESS)
	{
		{
			Lock lock(MStatusMutex);
			MState = MonitorState::Inactive;
		}

		B3D_EXCEPT(InternalErrorException, "Failed to start folder monitor on folder \"" + MFolderToMonitor.ToString() + "\" because ReadDirectoryChangesW failed.");
	}
}

void FolderMonitor::FolderWatchInfo::StopMonitor(HANDLE compPortHandle)
{
	if(MState != MonitorState::Inactive)
	{
		Lock lock(MStatusMutex);

		MState = MonitorState::Shutdown;
		PostQueuedCompletionStatus(compPortHandle, sizeof(this), (ULONG_PTR)this, &MOverlapped);

		while(MState != MonitorState::Inactive)
			MStartStopEvent.wait(lock);
	}

	if(MDirHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(MDirHandle);
		MDirHandle = INVALID_HANDLE_VALUE;
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
	Vector<FolderWatchInfo*> MFoldersToWatch;
	HANDLE MCompPortHandle;

	Queue<FileAction*> MFileActions;
	List<FileAction*> MActiveFileActions;

	Mutex MMainMutex;
	Thread* MWorkerThread;
};

FolderMonitor::FolderMonitor()
{
	m = B3DNew<Pimpl>();
	m->MWorkerThread = nullptr;
	m->MCompPortHandle = nullptr;
}

FolderMonitor::~FolderMonitor()
{
	StopMonitorAll();

	// No need for mutex since we know worker thread is shut down by now
	while(!m->MFileActions.empty())
	{
		FileAction* action = m->MFileActions.front();
		m->MFileActions.pop();

		FileAction::Destroy(action);
	}

	B3DDelete(m);
}

void FolderMonitor::StartMonitor(const Path& folderPath, bool subdirectories, FolderChangeBits changeFilter)
{
	if(!FileSystem::IsDirectory(folderPath))
	{
		B3D_LOG(Error, Generic, "Provided path \"{0}\" is not a directory", folderPath);
		return;
	}

	WString extendedFolderPath = L"\\\\?\\" + UTF8::ToWide(folderPath.ToString(Path::PathType::Windows));
	HANDLE dirHandle = CreateFileW(extendedFolderPath.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

	if(dirHandle == INVALID_HANDLE_VALUE)
	{
		B3D_EXCEPT(InternalErrorException, "Failed to open folder \"" + folderPath.ToString() + "\" for monitoring. Error code: " + ToString((u64)GetLastError()));
	}

	DWORD filterFlags = 0;

	if(changeFilter.IsSet(FolderChangeBit::FileName))
		filterFlags |= FILE_NOTIFY_CHANGE_FILE_NAME;

	if(changeFilter.IsSet(FolderChangeBit::DirName))
		filterFlags |= FILE_NOTIFY_CHANGE_DIR_NAME;

	if(changeFilter.IsSet(FolderChangeBit::FileWrite))
		filterFlags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

	m->MFoldersToWatch.push_back(B3DNew<FolderWatchInfo>(folderPath, dirHandle, subdirectories, filterFlags));
	FolderWatchInfo* watchInfo = m->MFoldersToWatch.back();

	m->MCompPortHandle = CreateIoCompletionPort(dirHandle, m->MCompPortHandle, (ULONG_PTR)watchInfo, 0);

	if(m->MCompPortHandle == nullptr)
	{
		m->MFoldersToWatch.erase(m->MFoldersToWatch.end() - 1);
		B3DDelete(watchInfo);
		B3D_EXCEPT(InternalErrorException, "Failed to open completion port for folder monitoring. Error code: " + ToString((u64)GetLastError()));
	}

	if(m->MWorkerThread == nullptr)
	{
		m->MWorkerThread = B3DNew<Thread>([this]() { WorkerThreadMain(); });

		if(m->MWorkerThread == nullptr)
		{
			m->MFoldersToWatch.erase(m->MFoldersToWatch.end() - 1);
			B3DDelete(watchInfo);
			B3D_EXCEPT(InternalErrorException, "Failed to create a new worker thread for folder monitoring");
		}
	}

	if(m->MWorkerThread != nullptr)
	{
		watchInfo->StartMonitor(m->MCompPortHandle);
	}
	else
	{
		m->MFoldersToWatch.erase(m->MFoldersToWatch.end() - 1);
		B3DDelete(watchInfo);
		B3D_EXCEPT(InternalErrorException, "Failed to create a new worker thread for folder monitoring");
	}
}

void FolderMonitor::StopMonitor(const Path& folderPath)
{
	auto findIter = std::find_if(m->MFoldersToWatch.begin(), m->MFoldersToWatch.end(), [&](const FolderWatchInfo* x)
								 { return x->MFolderToMonitor == folderPath; });

	if(findIter != m->MFoldersToWatch.end())
	{
		FolderWatchInfo* watchInfo = *findIter;

		watchInfo->StopMonitor(m->MCompPortHandle);
		B3DDelete(watchInfo);

		m->MFoldersToWatch.erase(findIter);
	}

	if(m->MFoldersToWatch.size() == 0)
		StopMonitorAll();
}

void FolderMonitor::StopMonitorAll()
{
	for(auto& watchInfo : m->MFoldersToWatch)
	{
		watchInfo->StopMonitor(m->MCompPortHandle);

		{
			// Note: Need this mutex to ensure worker thread is done with watchInfo.
			// Even though we wait for a condition variable from the worker thread in stopMonitor,
			// that doesn't mean the worker thread is done with the condition variable
			// (which is stored inside watchInfo)
			Lock lock(m->MMainMutex);
			B3DDelete(watchInfo);
		}
	}

	m->MFoldersToWatch.clear();

	if(m->MWorkerThread != nullptr)
	{
		PostQueuedCompletionStatus(m->MCompPortHandle, 0, 0, nullptr);

		m->MWorkerThread->WaitUntilComplete();
		B3DDelete(m->MWorkerThread);
		m->MWorkerThread = nullptr;
	}

	if(m->MCompPortHandle != nullptr)
	{
		CloseHandle(m->MCompPortHandle);
		m->MCompPortHandle = nullptr;
	}
}

void FolderMonitor::WorkerThreadMain()
{
	FolderWatchInfo* watchInfo = nullptr;

	do
	{
		DWORD numBytes;
		LPOVERLAPPED overlapped;

		if(!GetQueuedCompletionStatus(m->MCompPortHandle, &numBytes, (PULONG_PTR)&watchInfo, &overlapped, INFINITE))
		{
			B3D_ASSERT(false);
			// TODO: Folder handle was lost most likely. Not sure how to deal with that. Shutdown watch on this folder and cleanup?
		}

		if(watchInfo != nullptr)
		{
			MonitorState state;

			{
				Lock lock(watchInfo->MStatusMutex);
				state = watchInfo->MState;
			}

			switch(state)
			{
			case MonitorState::Starting:
				if(!ReadDirectoryChangesW(watchInfo->MDirHandle, watchInfo->MBuffer, FolderWatchInfo::kReadBufferSize, watchInfo->MMonitorSubdirectories, watchInfo->MMonitorFlags, &watchInfo->MBufferSize, &watchInfo->MOverlapped, nullptr))
				{
					B3D_ASSERT(false); // TODO - Possibly the buffer was too small?
					watchInfo->MReadError = GetLastError();
				}
				else
				{
					watchInfo->MReadError = ERROR_SUCCESS;

					{
						Lock lock(watchInfo->MStatusMutex);
						watchInfo->MState = MonitorState::Monitoring;
					}
				}

				watchInfo->MStartStopEvent.notify_one();

				break;
			case MonitorState::Monitoring:
				{
					FileNotifyInfo info(watchInfo->MBuffer, FolderWatchInfo::kReadBufferSize);
					HandleNotifications(info, *watchInfo);

					if(!ReadDirectoryChangesW(watchInfo->MDirHandle, watchInfo->MBuffer, FolderWatchInfo::kReadBufferSize, watchInfo->MMonitorSubdirectories, watchInfo->MMonitorFlags, &watchInfo->MBufferSize, &watchInfo->MOverlapped, nullptr))
					{
						B3D_ASSERT(false); // TODO: Failed during normal operation, possibly the buffer was too small. Shutdown watch on this folder and cleanup?
						watchInfo->MReadError = GetLastError();
					}
					else
					{
						watchInfo->MReadError = ERROR_SUCCESS;
					}
				}
				break;
			case MonitorState::Shutdown:
				if(watchInfo->MDirHandle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(watchInfo->MDirHandle);
					watchInfo->MDirHandle = INVALID_HANDLE_VALUE;

					{
						Lock lock(watchInfo->MStatusMutex);
						watchInfo->MState = MonitorState::Shutdown2;
					}
				}
				else
				{
					{
						Lock lock(watchInfo->MStatusMutex);
						watchInfo->MState = MonitorState::Inactive;
					}

					{
						Lock lock(m->MMainMutex); // Ensures that we don't delete "watchInfo" before this thread is done with mStartStopEvent
						watchInfo->MStartStopEvent.notify_one();
					}
				}

				break;
			case MonitorState::Shutdown2:
				if(watchInfo->MDirHandle != INVALID_HANDLE_VALUE)
				{
					// Handle is still open? Try again.
					CloseHandle(watchInfo->MDirHandle);
					watchInfo->MDirHandle = INVALID_HANDLE_VALUE;
				}
				else
				{
					{
						Lock lock(watchInfo->MStatusMutex);
						watchInfo->MState = MonitorState::Inactive;
					}

					{
						Lock lock(m->MMainMutex); // Ensures that we don't delete "watchInfo" before this thread is done with mStartStopEvent
						watchInfo->MStartStopEvent.notify_one();
					}
				}

				break;
			default:
				break;
			}
		}
	}
	while(watchInfo != nullptr);
}

void FolderMonitor::HandleNotifications(FileNotifyInfo& notifyInfo, FolderWatchInfo& watchInfo)
{
	Vector<FileAction*> mActions;

	do
	{
		WString fullPath = notifyInfo.GetFileNameWithPath(watchInfo.MFolderToMonitor);

		// Ignore notifications about hidden files
		if((GetFileAttributesW(fullPath.c_str()) & FILE_ATTRIBUTE_HIDDEN) != 0)
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
			watchInfo.MCachedOldFileName = fullPath;
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			mActions.push_back(FileAction::CreateRenamed(watchInfo.MCachedOldFileName, fullPath));
			break;
		}
	}
	while(notifyInfo.GetNext());

	{
		Lock lock(m->MMainMutex);

		for(auto& action : mActions)
			m->MFileActions.push(action);
	}
}

void FolderMonitor::UpdateInternal()
{
	{
		Lock lock(m->MMainMutex);

		while(!m->MFileActions.empty())
		{
			FileAction* action = m->MFileActions.front();
			m->MFileActions.pop();

			m->MActiveFileActions.push_back(action);
		}
	}

	for(auto iter = m->MActiveFileActions.begin(); iter != m->MActiveFileActions.end();)
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

		m->MActiveFileActions.erase(iter++);
		FileAction::Destroy(action);
	}
}
