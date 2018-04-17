//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsFolderMonitor.h"
#include "FileSystem/BsFileSystem.h"
#include "Error/BsException.h"

#include <windows.h>
#include "String/BsUnicode.h"

namespace bs
{
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

		void startMonitor(HANDLE compPortHandle);
		void stopMonitor(HANDLE compPortHandle);

		static const UINT32 READ_BUFFER_SIZE = 65536;

		Path mFolderToMonitor;
		HANDLE mDirHandle;
		OVERLAPPED mOverlapped;
		MonitorState mState;
		UINT8 mBuffer[READ_BUFFER_SIZE];
		DWORD mBufferSize;
		bool mMonitorSubdirectories;
		DWORD mMonitorFlags;
		DWORD mReadError;

		WString mCachedOldFileName; // Used during rename notifications as they are handled in two steps

		Mutex mStatusMutex;
		Signal mStartStopEvent;
	};

	FolderMonitor::FolderWatchInfo::FolderWatchInfo(const Path& folderToMonitor, HANDLE dirHandle, bool monitorSubdirectories, DWORD monitorFlags)
		:mFolderToMonitor(folderToMonitor), mDirHandle(dirHandle), mState(MonitorState::Inactive), mBufferSize(0),
		mMonitorSubdirectories(monitorSubdirectories), mMonitorFlags(monitorFlags), mReadError(0)
	{
		memset(&mOverlapped, 0, sizeof(mOverlapped));
	}

	FolderMonitor::FolderWatchInfo::~FolderWatchInfo()
	{
		assert(mState == MonitorState::Inactive);

		stopMonitor(0);
	}

	void FolderMonitor::FolderWatchInfo::startMonitor(HANDLE compPortHandle)
	{
		if(mState != MonitorState::Inactive)
			return; // Already monitoring

		{
			Lock lock(mStatusMutex);

			mState = MonitorState::Starting;
			PostQueuedCompletionStatus(compPortHandle, sizeof(this), (ULONG_PTR)this, &mOverlapped);

			while(mState != MonitorState::Monitoring)
				mStartStopEvent.wait(lock);
		}

		if(mReadError != ERROR_SUCCESS)
		{
			{
				Lock lock(mStatusMutex);
				mState = MonitorState::Inactive;
			}

			BS_EXCEPT(InternalErrorException, "Failed to start folder monitor on folder \"" + 
				mFolderToMonitor.toString() + "\" because ReadDirectoryChangesW failed.");
		}
	}

	void FolderMonitor::FolderWatchInfo::stopMonitor(HANDLE compPortHandle)
	{
		if(mState != MonitorState::Inactive)
		{
			Lock lock(mStatusMutex);

			mState = MonitorState::Shutdown;
			PostQueuedCompletionStatus(compPortHandle, sizeof(this), (ULONG_PTR)this, &mOverlapped);

			while(mState != MonitorState::Inactive)
				mStartStopEvent.wait(lock);
		}

		if(mDirHandle != INVALID_HANDLE_VALUE)
		{			
			CloseHandle(mDirHandle);
			mDirHandle = INVALID_HANDLE_VALUE;
		}
	}

	class FolderMonitor::FileNotifyInfo
	{
	public:
		FileNotifyInfo(UINT8* notifyBuffer, DWORD bufferSize)
		:mBuffer(notifyBuffer), mBufferSize(bufferSize)
		{
			mCurrentRecord = (PFILE_NOTIFY_INFORMATION)mBuffer;
		}

		bool getNext();
	
		DWORD	getAction() const;
		WString getFileName() const;
		WString getFileNameWithPath(const Path& rootPath) const;

	protected:
		UINT8* mBuffer;
		DWORD mBufferSize;
		PFILE_NOTIFY_INFORMATION mCurrentRecord;
	};

	bool FolderMonitor::FileNotifyInfo::getNext()
	{
		if(mCurrentRecord && mCurrentRecord->NextEntryOffset != 0)
		{
			PFILE_NOTIFY_INFORMATION oldRecord = mCurrentRecord;
			mCurrentRecord = (PFILE_NOTIFY_INFORMATION) ((UINT8*)mCurrentRecord + mCurrentRecord->NextEntryOffset);

			if((DWORD)((UINT8*)mCurrentRecord - mBuffer) > mBufferSize)
			{
				// Gone out of range, something bad happened
				assert(false);

				mCurrentRecord = oldRecord;
			}
					
			return (mCurrentRecord != oldRecord);
		}

		return false;
	}

	DWORD FolderMonitor::FileNotifyInfo::getAction() const
	{ 
		assert(mCurrentRecord != nullptr);

		if(mCurrentRecord)
			return mCurrentRecord->Action;

		return 0;
	}

	WString FolderMonitor::FileNotifyInfo::getFileName() const
	{
		if(mCurrentRecord)
		{
			wchar_t fileNameBuffer[32768 + 1] = {0};

			memcpy(fileNameBuffer, mCurrentRecord->FileName, 
					std::min(DWORD(32768 * sizeof(wchar_t)), mCurrentRecord->FileNameLength));
		
			return WString(fileNameBuffer);
		}

		return WString();
	}		

	WString FolderMonitor::FileNotifyInfo::getFileNameWithPath(const Path& rootPath) const
	{
		Path fullPath = rootPath;
		fullPath.append(UTF8::fromWide(getFileName()));
				
		return UTF8::toWide(fullPath.toString());
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
		static FileAction* createAdded(const WString& fileName)
		{
			String utf8filename = UTF8::fromWide(fileName);
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (utf8filename.size() + 1) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Added;

			memcpy(action->newName, utf8filename.data(), utf8filename.size() * sizeof(String::value_type));
			action->newName[utf8filename.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static FileAction* createRemoved(const WString& fileName)
		{
			String utf8filename = UTF8::fromWide(fileName);
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (utf8filename.size() + 1) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Removed;

			memcpy(action->newName, utf8filename.data(), utf8filename.size() * sizeof(String::value_type));
			action->newName[utf8filename.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static FileAction* createModified(const WString& fileName)
		{
			String utf8filename = UTF8::fromWide(fileName);
			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + (utf8filename.size() + 1) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = nullptr;
			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Modified;

			memcpy(action->newName, utf8filename.data(), utf8filename.size() * sizeof(String::value_type));
			action->newName[utf8filename.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static FileAction* createRenamed(const WString& oldFilename, const WString& newFileName)
		{
			String utf8Oldfilename = UTF8::fromWide(oldFilename);
			String utf8Newfilename = UTF8::fromWide(newFileName);

			UINT8* bytes = (UINT8*)bs_alloc((UINT32)(sizeof(FileAction) + 
				(utf8Oldfilename.size() + utf8Newfilename.size() + 2) * sizeof(String::value_type)));

			FileAction* action = (FileAction*)bytes;
			bytes += sizeof(FileAction);

			action->oldName = (String::value_type*)bytes;
			bytes += (utf8Oldfilename.size() + 1) * sizeof(String::value_type);

			action->newName = (String::value_type*)bytes;
			action->type = FileActionType::Modified;

			memcpy(action->oldName, utf8Oldfilename.data(), utf8Oldfilename.size() * sizeof(String::value_type));
			action->oldName[utf8Oldfilename.size()] = L'\0';

			memcpy(action->newName, utf8Newfilename.data(), utf8Newfilename.size() * sizeof(String::value_type));
			action->newName[utf8Newfilename.size()] = L'\0';
			action->lastSize = 0;
			action->checkForWriteStarted = false;

			return action;
		}

		static void destroy(FileAction* action)
		{
			bs_free(action);
		}

		String::value_type* oldName;
		String::value_type* newName;
		FileActionType type;

		UINT64 lastSize;
		bool checkForWriteStarted;
	};

	struct FolderMonitor::Pimpl
	{
		Vector<FolderWatchInfo*> mFoldersToWatch;
		HANDLE mCompPortHandle;

		Queue<FileAction*> mFileActions;
		List<FileAction*> mActiveFileActions;

		Mutex mMainMutex;
		Thread* mWorkerThread;
	};

	FolderMonitor::FolderMonitor()
	{
		m = bs_new<Pimpl>();
		m->mWorkerThread = nullptr;
		m->mCompPortHandle = nullptr;
	}

	FolderMonitor::~FolderMonitor()
	{
		stopMonitorAll();

		// No need for mutex since we know worker thread is shut down by now
		while(!m->mFileActions.empty())
		{
			FileAction* action = m->mFileActions.front();
			m->mFileActions.pop();

			FileAction::destroy(action);
		}

		bs_delete(m);
	}

	void FolderMonitor::startMonitor(const Path& folderPath, bool subdirectories, FolderChangeBits changeFilter)
	{
		if(!FileSystem::isDirectory(folderPath))
		{
			LOGERR("Provided path \"" + folderPath.toString() + "\" is not a directory");
			return;
		}

		WString extendedFolderPath = L"\\\\?\\" + UTF8::toWide(folderPath.toString(Path::PathType::Windows));
		HANDLE dirHandle = CreateFileW(extendedFolderPath.c_str(), FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

		if(dirHandle == INVALID_HANDLE_VALUE)
		{
			BS_EXCEPT(InternalErrorException, "Failed to open folder \"" + folderPath.toString() + "\" for monitoring. Error code: " + toString((UINT64)GetLastError()));
		}

		DWORD filterFlags = 0;

		if(changeFilter.isSet(FolderChangeBit::FileName))
			filterFlags |= FILE_NOTIFY_CHANGE_FILE_NAME;

		if(changeFilter.isSet(FolderChangeBit::DirName))
			filterFlags |= FILE_NOTIFY_CHANGE_DIR_NAME;

		if(changeFilter.isSet(FolderChangeBit::FileWrite))
			filterFlags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

		m->mFoldersToWatch.push_back(bs_new<FolderWatchInfo>(folderPath, dirHandle, subdirectories, filterFlags));
		FolderWatchInfo* watchInfo = m->mFoldersToWatch.back();

		m->mCompPortHandle = CreateIoCompletionPort(dirHandle, m->mCompPortHandle, (ULONG_PTR)watchInfo, 0);

		if(m->mCompPortHandle == nullptr)
		{
			m->mFoldersToWatch.erase(m->mFoldersToWatch.end() - 1);
			bs_delete(watchInfo);
			BS_EXCEPT(InternalErrorException, "Failed to open completion port for folder monitoring. Error code: " + toString((UINT64)GetLastError()));
		}

		if(m->mWorkerThread == nullptr)
		{
			m->mWorkerThread = bs_new<Thread>(std::bind(&FolderMonitor::workerThreadMain, this));

			if(m->mWorkerThread == nullptr)
			{
				m->mFoldersToWatch.erase(m->mFoldersToWatch.end() - 1);
				bs_delete(watchInfo);
				BS_EXCEPT(InternalErrorException, "Failed to create a new worker thread for folder monitoring");
			}
		}

		if(m->mWorkerThread != nullptr)
		{
			watchInfo->startMonitor(m->mCompPortHandle);
		}
		else
		{
			m->mFoldersToWatch.erase(m->mFoldersToWatch.end() - 1);
			bs_delete(watchInfo);
			BS_EXCEPT(InternalErrorException, "Failed to create a new worker thread for folder monitoring");
		}
	}

	void FolderMonitor::stopMonitor(const Path& folderPath)
	{
		auto findIter = std::find_if(m->mFoldersToWatch.begin(), m->mFoldersToWatch.end(), 
			[&](const FolderWatchInfo* x) { return x->mFolderToMonitor == folderPath; });

		if(findIter != m->mFoldersToWatch.end())
		{
			FolderWatchInfo* watchInfo = *findIter;

			watchInfo->stopMonitor(m->mCompPortHandle);
			bs_delete(watchInfo);

			m->mFoldersToWatch.erase(findIter);
		}

		if(m->mFoldersToWatch.size() == 0)
			stopMonitorAll();
	}

	void FolderMonitor::stopMonitorAll()
	{
		for(auto& watchInfo : m->mFoldersToWatch)
		{
			watchInfo->stopMonitor(m->mCompPortHandle);

			{
				// Note: Need this mutex to ensure worker thread is done with watchInfo.
				// Even though we wait for a condition variable from the worker thread in stopMonitor,
				// that doesn't mean the worker thread is done with the condition variable
				// (which is stored inside watchInfo)
				Lock lock(m->mMainMutex);
				bs_delete(watchInfo);
			}
		}

		m->mFoldersToWatch.clear();

		if(m->mWorkerThread != nullptr)
		{
			PostQueuedCompletionStatus(m->mCompPortHandle, 0, 0, nullptr);

			m->mWorkerThread->join();
			bs_delete(m->mWorkerThread);
			m->mWorkerThread = nullptr;
		}

		if(m->mCompPortHandle != nullptr)
		{
			CloseHandle(m->mCompPortHandle);
			m->mCompPortHandle = nullptr;
		}
	}

	void FolderMonitor::workerThreadMain()
	{
		FolderWatchInfo* watchInfo = nullptr;

		do 
		{
			DWORD numBytes;
			LPOVERLAPPED overlapped;

			if(!GetQueuedCompletionStatus(m->mCompPortHandle, &numBytes, (PULONG_PTR) &watchInfo, &overlapped, INFINITE))
			{
				assert(false);
				// TODO: Folder handle was lost most likely. Not sure how to deal with that. Shutdown watch on this folder and cleanup?
			}

			if(watchInfo != nullptr)
			{
				MonitorState state;

				{
					Lock lock(watchInfo->mStatusMutex);
					state = watchInfo->mState;
				}

				switch(state)
				{
				case MonitorState::Starting:
					if(!ReadDirectoryChangesW(watchInfo->mDirHandle, watchInfo->mBuffer, FolderWatchInfo::READ_BUFFER_SIZE,
						watchInfo->mMonitorSubdirectories, watchInfo->mMonitorFlags, &watchInfo->mBufferSize, &watchInfo->mOverlapped, nullptr))
					{
						assert(false); // TODO - Possibly the buffer was too small?
						watchInfo->mReadError = GetLastError();
					}
					else
					{
						watchInfo->mReadError = ERROR_SUCCESS;

						{
							Lock lock(watchInfo->mStatusMutex);
							watchInfo->mState = MonitorState::Monitoring;
						}
					}

					watchInfo->mStartStopEvent.notify_one();

					break;
				case MonitorState::Monitoring:
					{
						FileNotifyInfo info(watchInfo->mBuffer, FolderWatchInfo::READ_BUFFER_SIZE);
						handleNotifications(info, *watchInfo);

						if(!ReadDirectoryChangesW(watchInfo->mDirHandle, watchInfo->mBuffer, FolderWatchInfo::READ_BUFFER_SIZE,
							watchInfo->mMonitorSubdirectories, watchInfo->mMonitorFlags, &watchInfo->mBufferSize, &watchInfo->mOverlapped, nullptr))
						{
							assert(false); // TODO: Failed during normal operation, possibly the buffer was too small. Shutdown watch on this folder and cleanup?
							watchInfo->mReadError = GetLastError();
						}
						else
						{
							watchInfo->mReadError = ERROR_SUCCESS;
						}
					}
					break;
				case MonitorState::Shutdown:
					if(watchInfo->mDirHandle != INVALID_HANDLE_VALUE)
					{
						CloseHandle(watchInfo->mDirHandle);
						watchInfo->mDirHandle = INVALID_HANDLE_VALUE;

						{
							Lock lock(watchInfo->mStatusMutex);
							watchInfo->mState = MonitorState::Shutdown2;
						}
					}
					else
					{
						{
							Lock lock(watchInfo->mStatusMutex);
							watchInfo->mState = MonitorState::Inactive;
						}

						{
							Lock lock(m->mMainMutex); // Ensures that we don't delete "watchInfo" before this thread is done with mStartStopEvent
							watchInfo->mStartStopEvent.notify_one();
						}
					}

					break;
				case MonitorState::Shutdown2:
					if(watchInfo->mDirHandle != INVALID_HANDLE_VALUE)
					{
						// Handle is still open? Try again.
						CloseHandle(watchInfo->mDirHandle);
						watchInfo->mDirHandle = INVALID_HANDLE_VALUE;
					}
					else
					{
						{
							Lock lock(watchInfo->mStatusMutex);
							watchInfo->mState = MonitorState::Inactive;
						}

						{
							Lock lock(m->mMainMutex); // Ensures that we don't delete "watchInfo" before this thread is done with mStartStopEvent
							watchInfo->mStartStopEvent.notify_one();
						}
					}

					break;
				default:
					break;
				}
			}

		} while (watchInfo != nullptr);
	}

	void FolderMonitor::handleNotifications(FileNotifyInfo& notifyInfo, FolderWatchInfo& watchInfo)
	{
		Vector<FileAction*> mActions;

		do
		{
			WString fullPath = notifyInfo.getFileNameWithPath(watchInfo.mFolderToMonitor);

			// Ignore notifications about hidden files
			if ((GetFileAttributesW(fullPath.c_str()) & FILE_ATTRIBUTE_HIDDEN) != 0)
				continue;

			switch(notifyInfo.getAction())
			{
			case FILE_ACTION_ADDED:
					mActions.push_back(FileAction::createAdded(fullPath));
				break;
			case FILE_ACTION_REMOVED:
					mActions.push_back(FileAction::createRemoved(fullPath));
				break;
			case FILE_ACTION_MODIFIED:
					mActions.push_back(FileAction::createModified(fullPath));
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
					watchInfo.mCachedOldFileName = fullPath;
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
					mActions.push_back(FileAction::createRenamed(watchInfo.mCachedOldFileName, fullPath));
				break;
			}
	
		} while(notifyInfo.getNext());

		{
			Lock lock(m->mMainMutex);

			for(auto& action : mActions)
				m->mFileActions.push(action);
		}
	}

	void FolderMonitor::_update()
	{
		{
			Lock lock(m->mMainMutex);

			while (!m->mFileActions.empty())
			{
				FileAction* action = m->mFileActions.front();
				m->mFileActions.pop();

				m->mActiveFileActions.push_back(action);
			}
		}

		for (auto iter = m->mActiveFileActions.begin(); iter != m->mActiveFileActions.end();)
		{
			FileAction* action = *iter;
			
			// Reported file actions might still be in progress (i.e. something might still be writing to those files).
			// Sadly there doesn't seem to be a way to properly determine when those files are done being written, so instead
			// we check for at least a couple of frames if the file's size hasn't changed before reporting a file action.
			// This takes care of most of the issues and avoids reporting partially written files in almost all cases.
			if (FileSystem::exists(action->newName))
			{
				UINT64 size = FileSystem::getFileSize(action->newName);
				if (!action->checkForWriteStarted)
				{
					action->checkForWriteStarted = true;
					action->lastSize = size;

					++iter;
					continue;
				}
				else
				{
					if (action->lastSize != size)
					{
						action->lastSize = size;
						++iter;
						continue;
					}
				}
			}

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

			m->mActiveFileActions.erase(iter++);
			FileAction::destroy(action);
		}
	}
}
