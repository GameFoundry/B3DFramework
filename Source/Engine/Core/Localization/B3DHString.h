//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Localization
	 *  @{
	 */

	/**
	 * String handle. Provides a wrapper around an Unicode string, primarily for localization purposes.
	 *
	 * Actual value for this string is looked up in a global string table based on the provided identifier string and
	 * currently active language. If such value doesn't exist then the identifier is used as is.
	 *
	 * Use {0}, {1}, etc. in the string value for values that might change dynamically.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportName(LocString), DocumentationGroup(Localization)) HString : public IScriptExportable
	{
	public:
		/**
		 * Creates a new localized string with the specified identifier. If the identifier doesn't previously exist in the
		 * string table, identifier value will also be used for initializing the default language version of the string.
		 *
		 * @param	identifier		String you can use for later referencing the localized string.
		 * @param	stringTableId	Unique identifier of the string table to retrieve the string from.
		 */
		B3D_SCRIPT_EXPORT()
		explicit HString(const String& identifier, u32 stringTableId = 0);

		/**
		 * Creates a new localized string with the specified identifier and sets the default language version of the
		 * string. If a string with that identifier already exists default language string will be updated.
		 *
		 * @param	identifier		String you can use for later referencing the localized string.
		 * @param	defaultString	Default string to assign to the specified identifier. Language to which it will be
		 *							assigned depends on the StringTable::DEFAULT_LANGUAGE value.
		 * @param	stringTableId	Unique identifier of the string table to retrieve the string from.
		 */
		B3D_SCRIPT_EXPORT()
		explicit HString(const String& identifier, const String& defaultString, u32 stringTableId = 0);

		/**
		 * Creates a new empty localized string.
		 *
		 * @param	stringTableId	Unique identifier of the string table to retrieve the string from.
		 */
		B3D_SCRIPT_EXPORT()
		HString(u32 stringTableId);

		/** Creates a new empty localized string. */
		B3D_SCRIPT_EXPORT()
		HString();

		HString(const HString& copy);
		~HString();

		HString& operator=(const HString& rhs);

		explicit operator const String&() const;

		B3D_SCRIPT_EXPORT(InteropOnly(true))
		const String& GetValue() const;

		/**
		 * Sets a value of a string parameter. Parameters are specified as bracketed values within the string itself
		 * (for example {0}, {1}) etc. Use ^ as an escape character.
		 *
		 * @param	index	Index of the parameter to set.
		 * @param	value	Value to assign to the parameter.
		 *
		 * @note	This is useful for strings that have dynamically changing values, like numbers, embedded in them.
		 */
		B3D_SCRIPT_EXPORT()
		void SetParameter(u32 index, const String& value);

		/** Returns an empty string. */
		static const HString& Dummy();

	private:
		SPtr<LocalizedStringData> mStringData;
		String* mParameters = nullptr;

		mutable bool mIsDirty = true;
		mutable String mCachedString;
		mutable String* mStringPtr = nullptr;
	};

	/** @} */
} // namespace b3d
