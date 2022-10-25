//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/BsLog.h"
#include "Error/BsException.h"

using namespace bs;

UnorderedMap<u32, String> Log::sCategories;

Log::~Log()
{
	Clear();
}

void Log::LogMsg(const String& message, LogVerbosity verbosity, u32 category)
{
	RecursiveLock lock(mMutex);

	mUnreadEntries.push(LogEntry(message, verbosity, category));
}

void Log::Clear()
{
	RecursiveLock lock(mMutex);

	mEntries.clear();

	while(!mUnreadEntries.empty())
		mUnreadEntries.pop();

	mHash++;
}

void Log::Clear(LogVerbosity verbosity, u32 category)
{
	RecursiveLock lock(mMutex);

	Vector<LogEntry> newEntries;
	for(auto& entry : mEntries)
	{
		if(((verbosity == LogVerbosity::Any) || entry.GetVerbosity() == verbosity) &&
		   (category == (u32)-1 || entry.GetCategory() == category))
			continue;

		newEntries.push_back(entry);
	}

	mEntries = newEntries;

	Queue<LogEntry> newUnreadEntries;
	while(!mUnreadEntries.empty())
	{
		LogEntry entry = mUnreadEntries.front();
		mUnreadEntries.pop();

		if(((verbosity == LogVerbosity::Any) || entry.GetVerbosity() == verbosity) &&
		   (category == (u32)-1 || entry.GetCategory() == category))
			continue;

		newUnreadEntries.push(entry);
	}

	mUnreadEntries = newUnreadEntries;
	mHash++;
}

bool Log::GetUnreadEntry(LogEntry& entry)
{
	RecursiveLock lock(mMutex);

	if(mUnreadEntries.empty())
		return false;

	entry = mUnreadEntries.front();
	mUnreadEntries.pop();
	mEntries.push_back(entry);
	mHash++;

	return true;
}

bool Log::GetLastEntry(LogEntry& entry)
{
	if(mEntries.size() == 0)
		return false;

	entry = mEntries.back();
	return true;
}

Vector<LogEntry> Log::GetEntries() const
{
	RecursiveLock lock(mMutex);

	return mEntries;
}

bool Log::RegisterCategoryInternal(u32 id, const char* name)
{
	if(!CategoryExists(id))
	{
		sCategories.emplace(id, name);
		return true;
	}

	return false;
}

bool Log::CategoryExists(u32 id)
{
	return sCategories.find(id) != sCategories.end();
}

bool Log::GetCategoryName(u32 id, String& name)
{
	auto search = sCategories.find(id);
	if(search != sCategories.end())
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
		RecursiveLock lock(mMutex);

		for(auto& entry : mEntries)
			entries.push_back(entry);

		Queue<LogEntry> unreadEntries = mUnreadEntries;
		while(!unreadEntries.empty())
		{
			entries.push_back(unreadEntries.front());
			unreadEntries.pop();
		}
	}

	return entries;
}
