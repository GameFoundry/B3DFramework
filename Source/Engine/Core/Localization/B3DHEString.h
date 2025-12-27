//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/**
	 * Helper class used for constructing HString%s that references the engine string table. Engine string table is just
	 * a separate string table so it doesn't conflict with the game string table.
	 */
	class B3D_EXPORT HEString
	{
	public:
		/**
		 * Creates a new localized string with the specified identifier in the engine string table. If the identifier
		 * doesn't previously exist in the string table, identifier value will also be used for initializing the default
		 * language version of the string.
		 *
		 * @param	identifier		String you can use for later referencing the localized string.
		 */
		HEString(const String& identifier);

		/**
		 * Creates a new localized string with the specified identifier in the engine string table and sets the default
		 * language version of the string. If a string with that identifier already exists default language string will be
		 * updated.
		 *
		 * @param	identifier		String you can use for later referencing the localized string.
		 * @param	defaultString	Default string to assign to the specified identifier. Language to which it
		 *							will be assigned depends on the StringTable::DEFAULT_LANGUAGE value.
		 */
		HEString(const String& identifier, const String& defaultString);

		/**	Creates a new empty localized string in the engine string table. */
		HEString();

		/**	Implicitly casts the editor string type to a generic string type. */
		operator HString() const;

	private:
		static const u32 kEngineStringTableId;

		HString mInternal;
	};

	/** @} */
} // namespace b3d
