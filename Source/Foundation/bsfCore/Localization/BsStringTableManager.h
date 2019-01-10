//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Localization/BsStringTable.h"

namespace bs
{
	/** @addtogroup Localization
	 *  @{
	 */

	/**
	 * Manages string tables used for localizing text. Allows you to add and remove different tables and change the active 
	 * language.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(n:StringTables,m:Localization) StringTableManager : public Module<StringTableManager>
	{
	public:
		StringTableManager() = default;

		/** Determines the currently active language. Any newly created strings will use this value. */
		BS_SCRIPT_EXPORT(n:ActiveLanguage,pr:setter)
		void setActiveLanguage(Language language);

		/** @copydoc setActiveLanguage() */
		BS_SCRIPT_EXPORT(n:ActiveLanguage,pr:getter)
		Language getActiveLanguage() const { return mActiveLanguage; }

		/** 
		 * Returns the string table with the specified id. If the table doesn't exist new one is created.
		 *
		 * @param[in]	id		Identifier of the string table.
		 * @return				String table with the specified identifier.
		 */
		BS_SCRIPT_EXPORT()
		HStringTable getTable(UINT32 id);

		/** 
		 * Removes the string table with the specified id.
		 *
		 * @param[in]	id		Identifier of the string table.
		 */
		BS_SCRIPT_EXPORT()
		void removeTable(UINT32 id);

		/** 
		 * Registers a new string table or replaces an old one at the specified id.
		 *
		 * @param[in]	id		Identifier of the string table.
		 * @param[in]	table	New string table to assign to the specified identifier.
		 */
		BS_SCRIPT_EXPORT()
		void setTable(UINT32 id, const HStringTable& table);

	private:
		Language mActiveLanguage = StringTable::DEFAULT_LANGUAGE;
		UnorderedMap<UINT32, HStringTable> mTables;
	};

	/** Provides easier access to StringTableManager. */
	BS_CORE_EXPORT StringTableManager& gStringTableManager();

	/** @} */
}