//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup String
	 *  @{
	 */

	/** Provides methods to converting between UTF-8 character encoding and other popular encodings. */
	class BS_UTILITY_EXPORT UTF8
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
		static String fromANSI(const String& input, const std::locale& locale = std::locale(""));

		/**
		 * Converts from an UTF-8 encoding into ANSI encoding in the specified locale.
		 * 
		 * @param[in]	input			Narrow string encoded as UTF-8 characters.
		 * @param[in]	locale			Locale that determines from which code page to generate the ANSI characters.
		 * @param[in]	invalidChar		Character that will be used when an Unicode character cannot be represented using
		 *								the selected ANSI code page.
		 * @return						ANSI encoded string in the specified locale.
		 */
		static String toANSI(const String& input, const std::locale& locale = std::locale(""), char invalidChar = 0);

		/**
		 * Converts from a system-specific wide character encoding into UTF-8.
		 * 
		 * @param[in]	input	Wide string to convert. Actual encoding is system specific be can be assumed to be UTF-16 on 
		 *						Windows and UTF-32 on Unix.
		 * @return				UTF-8 encoded string.
		 */
		static String fromWide(const WString& input);

		/**
		 * Converts from an UTF-8 encoding into system-specific wide character encoding.
		 * 
		 * @param[in]	input	Narrow string encoded as UTF-8 characters.
		 * @return				Wide string encoded in a system-specific manner. Actual encoding can be assumed to be UTF-16 
		 *						on Windows and UTF-32 and Unix.
		 */
		static WString toWide(const String& input);

		/**
		 * Converts from an UTF-16 encoding into UTF-8.
		 * 
		 * @param[in]	input	String encoded as UTF-16.
		 * @return				UTF-8 encoded string.
		 */
		static String fromUTF16(const U16String& input);

		/**
		 * Converts from an UTF-8 encoding into UTF-16.
		 * 
		 * @param[in]	input	String encoded as UTF-8.
		 * @return				UTF-16 encoded string.
		 */
		static U16String toUTF16(const String& input);

		/**
		 * Converts from an UTF-32 encoding into UTF-8.
		 * 
		 * @param[in]	input	String encoded as UTF-32.
		 * @return				UTF-8 encoded string.
		 */
		static String fromUTF32(const U32String& input);

		/**
		 * Converts from an UTF-8 encoding into UTF-32.
		 * 
		 * @param[in]	input	String encoded as UTF-8.
		 * @return				UTF-32 encoded string.
		 */
		static U32String toUTF32(const String& input);
	};

	/** @} */
}