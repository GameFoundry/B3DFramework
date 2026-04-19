//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/B3DLog.h"

#include "B3DDebug.h"

using namespace b3d;

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

void Log::LogMessage(const String& message, LogVerbosity verbosity, const String& category)
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

void Log::Clear(const String& categoryName, LogVerbosity verbosity)
{
	RecursiveLock lock(mMutex);

	Vector<LogEntry> newEntries;
	for(auto& entry : mEntries)
	{
		if(((verbosity == LogVerbosity::Any) || entry.Verbosity == verbosity) && (categoryName.empty() || entry.CategoryName == categoryName))
			continue;

		newEntries.push_back(entry);
	}

	mEntries = newEntries;

	Queue<LogEntry> newUnreadEntries;
	while(!mUnreadEntries.empty())
	{
		LogEntry entry = mUnreadEntries.front();
		mUnreadEntries.pop();

		if(((verbosity == LogVerbosity::Any) || entry.Verbosity == verbosity) && (categoryName.empty() || entry.CategoryName == categoryName))
			continue;

		newUnreadEntries.push(entry);
	}

	mUnreadEntries = newUnreadEntries;
	mHash++;
}

bool Log::GetUnreadEntry(LogEntry& outEntry)
{
	RecursiveLock lock(mMutex);

	if(mUnreadEntries.empty())
		return false;

	outEntry = mUnreadEntries.front();
	mUnreadEntries.pop();
	mEntries.push_back(outEntry);
	mHash++;

	return true;
}

bool Log::GetLastEntry(LogEntry& outEntry)
{
	if(mEntries.size() == 0)
		return false;

	outEntry = mEntries.back();
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
