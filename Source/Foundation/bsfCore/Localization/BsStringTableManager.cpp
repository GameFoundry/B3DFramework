//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Localization/BsStringTableManager.h"

namespace bs
{
	void StringTableManager::SetActiveLanguage(Language language)
	{
		if (language != mActiveLanguage)
		{
			mActiveLanguage = language;

			for (auto& tablePair : mTables)
				tablePair.second->setActiveLanguage(language);
		}
	}

	HStringTable StringTableManager::GetTable(UINT32 id)
	{
		auto iterFind = mTables.find(id);
		if (iterFind != mTables.end())
			return iterFind->second;

		HStringTable newTable = StringTable::create();
		setTable(id, newTable);

		return newTable;
	}

	void StringTableManager::RemoveTable(UINT32 id)
	{
		mTables.erase(id);
	}

	void StringTableManager::SetTable(UINT32 id, const HStringTable& table)
	{
		mTables[id] = table;

		if (table != nullptr)
			table->setActiveLanguage(mActiveLanguage);
	}

	StringTableManager& GStringTableManager()
	{
		return StringTableManager::Instance();
	}
}
