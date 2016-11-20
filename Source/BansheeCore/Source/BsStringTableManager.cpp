//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStringTableManager.h"

namespace bs
{
	StringTableManager::StringTableManager()
		:mActiveLanguage(StringTable::DEFAULT_LANGUAGE)
	{

	}

	void StringTableManager::setActiveLanguage(Language language)
	{
		if (language != mActiveLanguage)
		{
			mActiveLanguage = language;

			for (auto& tablePair : mTables)
				tablePair.second->setActiveLanguage(language);
		}
	}

	HStringTable StringTableManager::getTable(UINT32 id)
	{
		auto iterFind = mTables.find(id);
		if (iterFind != mTables.end())
			return iterFind->second;

		HStringTable newTable = StringTable::create();
		setTable(id, newTable);

		return newTable;
	}

	void StringTableManager::removeTable(UINT32 id)
	{
		mTables.erase(id);
	}

	void StringTableManager::setTable(UINT32 id, HStringTable table)
	{
		mTables[id] = table;

		if (table != nullptr)
			table->setActiveLanguage(mActiveLanguage);
	}
}