//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Localization/B3DStringTable.h"

namespace b3d
{
	/** @addtogroup Localization
	 *  @{
	 */

	/**
	 * Manages string tables used for localizing text. Allows you to add and remove different tables and change the active
	 * language.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportName(StringTables), DocumentationGroup(Localization)) StringTableManager : public Module<StringTableManager>
	{
	public:
		StringTableManager() = default;

		/** Determines the currently active language. Any newly created strings will use this value. */
		B3D_SCRIPT_EXPORT(ExportName(ActiveLanguage), Property(Setter))
		void SetActiveLanguage(Language language);

		/** @copydoc SetActiveLanguage() */
		B3D_SCRIPT_EXPORT(ExportName(ActiveLanguage), Property(Getter))
		Language GetActiveLanguage() const { return mActiveLanguage; }

		/**
		 * Returns the string table with the specified id. If the table doesn't exist new one is created.
		 *
		 * @param	id		Identifier of the string table.
		 * @return			String table with the specified identifier.
		 */
		B3D_SCRIPT_EXPORT()
		HStringTable GetTable(u32 id);

		/**
		 * Removes the string table with the specified id.
		 *
		 * @param	id		Identifier of the string table.
		 */
		B3D_SCRIPT_EXPORT()
		void RemoveTable(u32 id);

		/**
		 * Registers a new string table or replaces an old one at the specified id.
		 *
		 * @param	id		Identifier of the string table.
		 * @param	table	New string table to assign to the specified identifier.
		 */
		B3D_SCRIPT_EXPORT()
		void SetTable(u32 id, const HStringTable& table);

	private:
		Language mActiveLanguage = StringTable::kDefaultLanguage;
		UnorderedMap<u32, HStringTable> mTables;
	};

	/** Provides easier access to StringTableManager. */
	B3D_EXPORT StringTableManager& GetStringTableManager();

	/** @} */
} // namespace b3d
