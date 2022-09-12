//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/BsLog.h"
#include "Error/BsException.h"

namespace bs
{
	UnorderedMap<UINT32, String> Log::sCategories;

	Log::~Log()
	{
		clear();
	}

	void Log::LogMsg(const String& message, LogVerbosity verbosity, UINT32 category)
	{
		RecursiveLock Lock(mMutex);

		mUnreadEntries.Push(LogEntry(message, verbosity, category));
	}

	void Log::Clear()
	{
		RecursiveLock Lock(mMutex);

		mEntries.Clear();

		while (!mUnreadEntries.Empty())
			mUnreadEntries.Pop();

		mHash++;
	}

	void Log::Clear(LogVerbosity verbosity, UINT32 category)
	{
		RecursiveLock Lock(mMutex);

		Vector<LogEntry> newEntries;
		for(auto& entry : mEntries)
		{
			if (((verbosity == LogVerbosity::Any) || entry.GetVerbosity() == verbosity) &&
				(category == (UINT32)-1 || entry.GetCategory() == category))
				continue;

			newEntries.push_back(entry);
		}

		mEntries = newEntries;

		Queue<LogEntry> newUnreadEntries;
		while (!mUnreadEntries.Empty())
		{
			LogEntry entry = mUnreadEntries.Front();
			mUnreadEntries.Pop();

			if (((verbosity == LogVerbosity::Any) || entry.GetVerbosity() == verbosity) &&
				(category == (UINT32)-1 || entry.GetCategory() == category))
				continue;

			newUnreadEntries.Push(entry);
		}

		mUnreadEntries = newUnreadEntries;
		mHash++;
	}

	bool Log::GetUnreadEntry(LogEntry& entry)
	{
		RecursiveLock Lock(mMutex);

		if (mUnreadEntries.Empty())
			return false;

		entry = mUnreadEntries.Front();
		mUnreadEntries.Pop();
		mEntries.push_back(entry);
		mHash++;

		return true;
	}

	bool Log::GetLastEntry(LogEntry& entry)
	{
		if (mEntries.Size() == 0)
			return false;

		entry = mEntries.Back();
		return true;
	}

	Vector<LogEntry> Log::GetEntries() const
	{
		RecursiveLock Lock(mMutex);

		return mEntries;
	}
	
	bool Log::_registerCategory(UINT32 id, const char* name)
	{
		if (!categoryExists(id))
		{
			sCategories.Emplace(id, name);
			return true;
		}

		return false;
	}
	
	bool Log::CategoryExists(UINT32 id)
	{
		return sCategories.Find(id) != sCategories.end();
	}
	
	bool Log::GetCategoryName(UINT32 id, String& name)
	{
		auto search = sCategories.Find(id);
		if (search != sCategories.End())
		{
			name = search->second;
			return true;
		}

		name = "Unknown";
		return false;
	}
	
	Vector<LogEntry> Log::GetAllEntries() const
	{
		Vector<LogEntry> entries;
		{
			RecursiveLock Lock(mMutex);

			for (auto& entry : mEntries)
				entries.push_back(entry);

			Queue<LogEntry> unreadEntries = mUnreadEntries;
			while (!unreadEntries.Empty())
			{
				entries.push_back(unreadEntries.Front());
				unreadEntries.Pop();
			}
		}

		return entries;
	}
}
