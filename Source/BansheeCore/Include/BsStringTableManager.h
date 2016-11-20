//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsStringTable.h"

namespace bs
{
	/** @addtogroup Localization
	 *  @{
	 */

	/**
	 * Manages string tables used for localizing text. Allows you to add and remove different tables and change the active 
	 * language.
	 */
	class BS_CORE_EXPORT StringTableManager : public Module<StringTableManager>
	{
	public:
		StringTableManager();

		/** Gets the currently active language. */
		Language getActiveLanguage() const { return mActiveLanguage; }

		/** Changes the currently active language. Any newly created strings will use this value. */
		void setActiveLanguage(Language language);

		/** Returns the string table with the specified id. If the table doesn't exist new one is created. */
		HStringTable getTable(UINT32 id);

		/** Removes the string table with the specified id. */
		void removeTable(UINT32 id);

		/** Registers a new string table or replaces an old one at the specified id. */
		void setTable(UINT32 id, HStringTable table);

	private:
		Language mActiveLanguage;
		UnorderedMap<UINT32, HStringTable> mTables;
	};

	/** @} */
}