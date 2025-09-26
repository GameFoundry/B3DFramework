//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup String
	 *  @{
	 */

	/** Provides methods to converting between UTF-8 character encoding and other popular encodings. */
	class B3D_EXPORT UTF8
	{
	public:
		/**
		 * Converts from an ANSI encoding in the specified locale into UTF-8.
		 *
		 * @param[in]	input	Narrow string encoded as ANSI characters. Characters are expected to be in the code page
		 *						specified by @p locale.
		 * @param[in]	locale	Locale that determines how are the ANSI characters interpreted.
		 * @return				UTF-8 encoded string.
		 */
		static String FromAnsi(const String& input, const std::locale& locale = std::locale(""));

		/**
		 * Converts from an UTF-8 encoding into ANSI encoding in the specified locale.
		 *
		 * @param[in]	input			Narrow string encoded as UTF-8 characters.
		 * @param[in]	locale			Locale that determines from which code page to generate the ANSI characters.
		 * @param[in]	invalidChar		Character that will be used when an Unicode character cannot be represented using
		 *								the selected ANSI code page.
		 * @return						ANSI encoded string in the specified locale.
		 */
		static String ToAnsi(const String& input, const std::locale& locale = std::locale(""), char invalidChar = 0);

		/**
		 * Converts from a system-specific wide character encoding into UTF-8.
		 *
		 * @param[in]	input	Wide string to convert. Actual encoding is system specific be can be assumed to be UTF-16 on
		 *						Windows and UTF-32 on Unix.
		 * @return				UTF-8 encoded string.
		 */
		static String FromWide(const WString& input);

		/**
		 * Converts from an UTF-8 encoding into system-specific wide character encoding.
		 *
		 * @param[in]	input	Narrow string encoded as UTF-8 characters.
		 * @return				Wide string encoded in a system-specific manner. Actual encoding can be assumed to be UTF-16
		 *						on Windows and UTF-32 and Unix.
		 */
		static WString ToWide(const String& input);

		/**
		 * Converts from an UTF-16 encoding into UTF-8.
		 *
		 * @param[in]	input	String encoded as UTF-16.
		 * @return				UTF-8 encoded string.
		 */
		static String FromUtF16(const U16String& input);

		/**
		 * Converts from an UTF-8 encoding into UTF-16.
		 *
		 * @param[in]	input	String encoded as UTF-8.
		 * @return				UTF-16 encoded string.
		 */
		static U16String ToUtF16(const String& input);

		/**
		 * Converts from an UTF-32 encoding into UTF-8.
		 *
		 * @param[in]	input	String encoded as UTF-32.
		 * @return				UTF-8 encoded string.
		 */
		static String FromUtF32(const U32String& input);

		/**
		 * Converts from an UTF-8 encoding into UTF-32.
		 *
		 * @param[in]	input	String encoded as UTF-8.
		 * @return				UTF-32 encoded string.
		 */
		static U32String ToUtF32(const String& input);

		/** Counts the number of characters in the provided UTF-8 input string. */
		static u32 Count(const String& input);

		/** Converts the provided UTF8 encoded string to lowercase. */
		static String ToLower(const String& input);

		/** Converts the provided UTF8 encoded string to uppercase. */
		static String ToUpper(const String& input);

		/**
		 * Returns the byte at which the character with the specified index starts. The string is expected to be in UTF-8
		 * encoding. If @p charIdx is out of range the method returns the index past the last byte in the string (same
		 * as the string length in bytes).
		 */
		static u32 CharToByteIndex(const String& input, u32 charIdx);

		/** Calculates the number of bytes taken up by the character at the specified position. */
		static u32 CharByteCount(const String& input, u32 charIdx);
	};

	/** @} */
} // namespace b3d
