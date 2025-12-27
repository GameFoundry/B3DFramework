//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Localization/B3DStringTableManager.h"

using namespace b3d;

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
	auto found = mTables.find(id);
	if(found != mTables.end())
		return found->second;

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

namespace b3d
{
	B3D_EXPORT StringTableManager& GetStringTableManager()
	{
		return StringTableManager::Instance();
	}
}
