//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/BsLog.h"

#include "BsDebug.h"
#include "Error/BsException.h"

using namespace bs;

LogCategoryBase::LogCategoryBase(const char* name, LogVerbosity defaultMaximumVerbosity)
: mName(name), mDefaultMaximumVerbosity(defaultMaximumVerbosity), mMaximumVerbosity(defaultMaximumVerbosity)
{
	Log::RegisterCategory(*this);
}

LogCategoryBase::~LogCategoryBase()
{
	Log::UnregisterCategory(*this);
}

Log::~Log()
{
	Clear();
}

void Log::LogMessage(const String& message, LogVerbosity verbosity, const char* category)
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

void Log::Clear(const char* categoryName, LogVerbosity verbosity)
{
	RecursiveLock lock(mMutex);

	Vector<LogEntry> newEntries;
	for(auto& entry : mEntries)
	{
		if(((verbosity == LogVerbosity::Any) || entry.Verbosity == verbosity) && (categoryName == nullptr || entry.CategoryName == categoryName))
			continue;

		newEntries.push_back(entry);
	}

	mEntries = newEntries;

	Queue<LogEntry> newUnreadEntries;
	while(!mUnreadEntries.empty())
	{
		LogEntry entry = mUnreadEntries.front();
		mUnreadEntries.pop();

		if(((verbosity == LogVerbosity::Any) || entry.Verbosity == verbosity) && (categoryName == nullptr || entry.CategoryName == categoryName))
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

UnorderedMultimap<const char*, LogCategoryBase*>& Log::GetCategoriesMap()
{
	static UnorderedMultimap<const char*, LogCategoryBase*> sCategories;
	return sCategories;
}

void Log::RegisterCategory(LogCategoryBase& category)
{
	const char* name = category.GetName();

	auto foundRange = GetCategoriesMap().equal_range(name);
	for(auto it = foundRange.first; it != foundRange.second; ++it)
	{
		if(B3D_ENSURE(it->second != &category))
			continue;

		return;
	}

	GetCategoriesMap().insert(std::make_pair(name, &category));
}

void Log::UnregisterCategory(LogCategoryBase& category)
{
	const char* name = category.GetName();

	auto foundRange = GetCategoriesMap().equal_range(name);
	for(auto it = foundRange.first; it != foundRange.second;)
	{
		if(it->second != &category)
		{
			++it;
			continue;
		}

		it = GetCategoriesMap().erase(it);
	}
}

void Log::SetCategoryMaximumVerbosity(const char* name, LogVerbosity maximumVerbosity)
{
	auto foundRange = GetCategoriesMap().equal_range(name);
	for(auto it = foundRange.first; it != foundRange.second;)
	{
		it->second->SetMaximumVerbosity(maximumVerbosity);
	}
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
