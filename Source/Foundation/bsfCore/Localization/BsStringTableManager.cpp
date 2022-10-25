//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Localization/BsStringTableManager.h"

using namespace bs;

void StringTableManager::SetActiveLanguage(Language language)
{
	if(language != mActiveLanguage)
	{
		mActiveLanguage = language;

		for(auto& tablePair : mTables)
			tablePair.second->SetActiveLanguage(language);
	}
}

HStringTable StringTableManager::GetTable(u32 id)
{
	auto iterFind = mTables.find(id);
	if(iterFind != mTables.end())
		return iterFind->second;

	HStringTable newTable = StringTable::Create();
	SetTable(id, newTable);

	return newTable;
}

void StringTableManager::RemoveTable(u32 id)
{
	mTables.erase(id);
}

void StringTableManager::SetTable(u32 id, const HStringTable& table)
{
	mTables[id] = table;

	if(table != nullptr)
		table->SetActiveLanguage(mActiveLanguage);
}

StringTableManager& gStringTableManager()
{
	return StringTableManager::Instance();
}
