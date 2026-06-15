//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Allocators/B3DMemoryAllocator.h"
#include <string>

namespace b3d
{
	enum class LogVerbosity;
	struct UUID;

	/** @addtogroup String
	 *  @{
	 */

	/** Defines what type of data should be written during the time_t to String conversion. */
	enum class TimeToStringConversionType
	{
		Date = 0, /**< Only year, month and day */
		Time = 1, /**< Only hours, minutes and seconds */
		Full = 2 /**< Full date and time */
	};

	/** Basic string that uses framework's memory allocators. */
	template <typename T>
	using BasicString = std::basic_string<T, std::char_traits<T>, StdAlloc<T>>;

	/**	Basic string stream that uses framework's memory allocators. */
	template <typename T>
	using BasicStringStream = std::basic_stringstream<T, std::char_traits<T>, StdAlloc<T>>;

	/** Wide string used primarily for handling Unicode text (UTF-32 on Linux, UTF-16 on Windows, generally). */
	using WString = BasicString<wchar_t>;

	/** Narrow string used for handling narrow encoded text (either locale specific ANSI or UTF-8). */
	using String = BasicString<char>;

	/** Wide string used UTF-16 encoded strings. */
	using U16String = BasicString<char16_t>;

	/** Wide string used UTF-32 encoded strings. */
	using U32String = BasicString<char32_t>;

	/** Wide string stream used for primarily for constructing wide strings. */
	using WStringStream = BasicStringStream<wchar_t>;

	/** Wide string stream used for primarily for constructing narrow strings. */
	using StringStream = BasicStringStream<char>;

	/** Wide string stream used for primarily for constructing UTF-16 strings. */
	using U16StringStream = BasicStringStream<char16_t>;

	/** Wide string stream used for primarily for constructing UTF-32 strings. */
	using U32StringStream = BasicStringStream<char32_t>;

	/** Non-owning view of the string. */
	using StringView = std::string_view;

	/** Equivalent to String, except it avoids any dynamic allocations until the number of elements exceeds @p Count. */
	template <int Count>
	using SmallString = std::basic_string<char, std::char_traits<char>, StdAlloc<char>>; // TODO: Currently equivalent to String, need to implement the allocator

	/** Converts a narrow string to a wide string. */
	B3D_EXPORT WString ToWideString(const String& source);

	/**	Converts a narrow string to a wide string. */
	B3D_EXPORT WString ToWideString(const char* source);

	/** Converts a float to a string. */
	B3D_EXPORT WString ToWideString(float val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/** Converts a double to a string. */
	B3D_EXPORT WString ToWideString(double val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/** Converts a Radian to a string. */
	B3D_EXPORT WString ToWideString(const Radian& val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/** Converts a Degree to a string. */
	B3D_EXPORT WString ToWideString(const Degree& val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an int to a string. */
	B3D_EXPORT WString ToWideString(int val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an unsigned int to a string. */
	B3D_EXPORT WString ToWideString(unsigned int val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a long to a string. */
	B3D_EXPORT WString ToWideString(long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an unsigned long to a string. */
	B3D_EXPORT WString ToWideString(unsigned long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a 64bit int to a string. */
	B3D_EXPORT WString ToWideString(long long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an 64bit unsigned int to a string. */
	B3D_EXPORT WString ToWideString(unsigned long long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an narrow char unsigned to a string. */
	B3D_EXPORT WString ToWideString(char val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an wide bit char unsigned to a string. */
	B3D_EXPORT WString ToWideString(wchar_t val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**
	 * Converts a boolean to a string.
	 *
	 * @param	value	Value to convert.
	 * @param	yesNo	(optional) If set to true, result is "yes" or "no" instead of "true" or "false".
	 */
	B3D_EXPORT WString ToWideString(bool value, bool yesNo = false);

	/**
	 * Converts a 2 dimensional vector to a string.
	 *
	 * @note	Format is "x y".
	 */
	B3D_EXPORT WString ToWideString(const Vector2& val);

	/**
	 * Converts a 2 dimensional integer vector to a string.
	 *
	 * @note	Format is "x y".
	 */
	B3D_EXPORT WString ToWideString(const Vector2I& val);

	/**
	 * Converts a 3 dimensional vector to a string.
	 *
	 * @note	Format is "x y z".
	 */
	B3D_EXPORT WString ToWideString(const Vector3& val);

	/**
	 * Converts a 4 dimensional vector to a string.
	 *
	 * @note	Format is "x y z w".
	 */
	B3D_EXPORT WString ToWideString(const Vector4& val);

	/**
	 * Converts a 3x3 matrix to a string.
	 *
	 * @note	Format is "00 01 02 10 11 12 20 21 22".
	 */
	B3D_EXPORT WString ToWideString(const Matrix3& val);

	/**
	 * Converts a 4x4 matrix to a string.
	 *
	 * @note	Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33".
	 */
	B3D_EXPORT WString ToWideString(const Matrix4& val);

	/**
	 * Converts a Quaternion to a string.
	 *
	 * @note	Format is "w x y z".
	 */
	B3D_EXPORT WString ToWideString(const Quaternion& val);

	/**
	 * Converts a color to a string.
	 *
	 * @note	Format is "r g b a".
	 */
	B3D_EXPORT WString ToWideString(const Color& val);

	/** Converts a vector of strings into a single string where the substrings are delimited by spaces. */
	B3D_EXPORT WString ToWideString(const Vector<b3d::WString>& val);

	/** Converts a wide string to a narrow string. */
	B3D_EXPORT String ToString(const WString& source);

	/**	Converts a wide string to a narrow string. */
	B3D_EXPORT String ToString(const wchar_t* source);

	/**	Converts a float to a string. */
	B3D_EXPORT String ToString(float val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a double to a string. */
	B3D_EXPORT String ToString(double val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a Radian to a string. */
	B3D_EXPORT String ToString(const Radian& val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a Degree to a string. */
	B3D_EXPORT String ToString(const Degree& val, unsigned short precision = 6, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an int to a string. */
	B3D_EXPORT String ToString(int val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an unsigned int to a string. */
	B3D_EXPORT String ToString(unsigned int val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a long to a string. */
	B3D_EXPORT String ToString(long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an unsigned long to a string. */
	B3D_EXPORT String ToString(unsigned long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts a 64bit int to a string. */
	B3D_EXPORT String ToString(long long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**	Converts an 64bit unsigned int to a string. */
	B3D_EXPORT String ToString(unsigned long long val, unsigned short width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	/**
	 * Converts a boolean to a string.
	 *
	 * @param	value	true to value.
	 * @param	yesNo	(optional) If set to true, result is "yes" or "no" instead of "true" or "false".
	 */
	B3D_EXPORT String ToString(bool value, bool yesNo = false);

	/**
	 * Converts a 2 dimensional vector to a string.
	 *
	 * @note	Format is "x y".
	 */
	B3D_EXPORT String ToString(const Vector2& val);

	/**
	 * Converts a 2 dimensional integer vector to a string.
	 *
	 * @note	Format is "x y".
	 */
	B3D_EXPORT String ToString(const Vector2I& val);

	/**
	 * Converts a 3 dimensional vector to a string.
	 *
	 * @note	Format is "x y z".
	 */
	B3D_EXPORT String ToString(const Vector3& val);

	/**
	 * Converts a 4 dimensional vector to a string.
	 *
	 * @note	Format is "x y z w".
	 */
	B3D_EXPORT String ToString(const Vector4& val);

	/**
	 * Converts a 3x3 matrix to a string.
	 *
	 * @note	Format is "00 01 02 10 11 12 20 21 22".
	 */
	B3D_EXPORT String ToString(const Matrix3& val);

	/**
	 * Converts a 4x4 matrix to a string.
	 *
	 * @note	Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33".
	 */
	B3D_EXPORT String ToString(const Matrix4& val);

	/**
	 * Converts a Quaternion to a string.
	 *
	 * @note	Format is "w x y z".
	 */
	B3D_EXPORT String ToString(const Quaternion& val);

	/**
	 * Converts a color to a string.
	 *
	 * @note	Format is "r g b a".
	 */
	B3D_EXPORT String ToString(const Color& val);

	/** Converts an UUID to a string. */
	B3D_EXPORT String ToString(const UUID& val);

	/** Converts a path to a string. */
	B3D_EXPORT String ToString(const Path& val);

	/** Converts a log verbosity to a string. */
	B3D_EXPORT String ToString(const LogVerbosity& val);

	/**
	 *  Converts the std::time_t structure containing time data to the string.
	 *
	 *  @param	time		Variable representing stored time
	 *  @param	isUTC		Outputs the date and time in Coordinated Universal Time, otherwise in local time.
	 *  @param	useISO8601	Outputs the date and time in ISO 8601 format, otherwise it uses a custom format.
	 *  @param	type		Type of the conversion applied.
	 *
	 *  @return Converted time as a String.
	 *
	 *  @note
	 *  Available output formats:
	 *	    1. When the ISO 8601 format is used
	 *			- Date: [NumericalYear]-[NumericalMonth]-[NumericalDay]
	 *			- Time: [HH]::[MM]::[SS]
	 *			- Full: [NumericalYear]-[NumericalMonth]-[NumericalDay]T[HH]::[MM]::[SS]Z
	 *		2. When the custom format is used
	 *			- Date: [DayOfWeek], [Month] [NumericalDate], [NumericalYear]
	 *			- Time: [HH]::[MM]::[SS]
	 *			- Full: [DayOfWeek], [Month] [NumericalDate], [NumericalYear] [HH]::[MM]::[SS]
	 *  By default will output the local hour in custom format.
	 */
	B3D_EXPORT String TimeToString(std::time_t time, bool isUTC = false, bool useISO8601 = false, TimeToStringConversionType type = TimeToStringConversionType::Time);

	/**
	 * Converts a vector of strings into a single string where the substrings are delimited by spaces.
	 */
	B3D_EXPORT String ToString(const Vector<b3d::String>& val);

	/**
	 * Converts a String to a float.
	 *
	 * @note	0.0f if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT float ParseFloat(const String& val, float defaultValue = 0);

	/**
	 * Converts a String to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT i32 ParseI32(const String& val, i32 defaultValue = 0);

	/**
	 * Converts a String to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT u32 ParseU32(const String& val, u32 defaultValue = 0);

	/**
	 * Converts a String to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT i64 ParseI64(const String& val, i64 defaultValue = 0);

	/**
	 * Converts a String to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT u64 ParseU64(const String& val, u64 defaultValue = 0);

	/**
	 * Converts a String to a boolean.
	 *
	 * @note	Returns true if case-insensitive match of the start of the string matches "true", "yes" or "1",
	 *			false otherwise.
	 */
	B3D_EXPORT bool ParseBool(const String& val, bool defaultValue = 0);

	/** Checks the String is a valid number value. */
	B3D_EXPORT bool IsNumber(const String& val);

	/**
	 * Converts a WString to a float.
	 *
	 * @note	0.0f if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT float ParseFloat(const WString& val, float defaultValue = 0);

	/**
	 * Converts a WString to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT i32 ParseI32(const WString& val, i32 defaultValue = 0);

	/**
	 * Converts a WString to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT u32 ParseU32(const WString& val, u32 defaultValue = 0);

	/**
	 * Converts a WString to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT i64 ParseI64(const WString& val, i64 defaultValue = 0);

	/**
	 * Converts a WString to a whole number.
	 *
	 * @note	0 if the value could not be parsed, otherwise the numeric version of the string.
	 */
	B3D_EXPORT u64 ParseU64(const WString& val, u64 defaultValue = 0);

	/**
	 * Converts a WString to a boolean.
	 *
	 * @note	Returns true if case-insensitive match of the start of the string
	 *			matches "true", "yes" or "1", false otherwise.
	 */
	B3D_EXPORT bool ParseBool(const WString& val, bool defaultValue = 0);

	/**
	 * Checks the WString is a valid number value.
	 */
	B3D_EXPORT bool IsNumber(const WString& val);

	/** @} */
} // namespace b3d

#include "String/B3DStringFormat.h"

namespace b3d
{
	/** @addtogroup String
	 *  @{
	 */

	/** Utility class for manipulating Strings. */
	class B3D_EXPORT StringUtility
	{
	public:
		/** Removes any whitespace characters from beginning or end of the string. */
		static String Trim(const String& str, bool left = true, bool right = true);

		/** @copydoc StringUtility::Trim(String&, bool, bool) */
		static WString Trim(const WString& str, bool left = true, bool right = true);

		/** Removes any whitespace characters from beginning or end of the string view. */
		static StringView Trim(StringView str, bool left = true, bool right = true);

		/**	Removes specified characters from beginning or end of the string. */
		static String Trim(const String& str, const String& delims, bool left = true, bool right = true);

		/** @copydoc StringUtility::Trim(String&, const String&, bool, bool) */
		static WString Trim(const WString& str, const WString& delims, bool left = true, bool right = true);

		/** Removes specified characters from beginning or end of the string view. */
		static StringView Trim(StringView str, StringView delims, bool left = true, bool right = true);

		/**
		 * Returns a vector of strings containing all the substrings delimited by the provided delimiter characters.
		 *
		 * @param[in]	str		 	The string to split.
		 * @param[in]	delims   	(optional) Delimiter characters to split the string by. They will not
		 * 							be included in resulting substrings.
		 * @param[in]	maxSplits	(optional) The maximum number of splits to perform (0 for unlimited splits). If this
		 *							parameters is > 0, the splitting process will stop after this many splits, left to right.
		 */
		static Vector<String> Split(const String& str, const String& delims = "\t\n ", unsigned int maxSplits = 0);

		/** @copydoc StringUtility::Split(const String&, const String&, unsigned int) */
		static Vector<WString> Split(const WString& str, const WString& delims = L"\t\n ", unsigned int maxSplits = 0);

		/**
		 * Returns a vector of strings containing all the substrings delimited by the provided delimiter characters, or the
		 * double delimiters used for including normal delimiter characters in the tokenized string.
		 *
		 * @param[in]	str		 		The string to split.
		 * @param[in]	delims   		(optional) Delimiter characters to split the string by. They will not
		 * 								be included in resulting substrings.
		 * @param[in]	doubleDelims	(optional) Delimiter character you may use to surround other normal delimiters,
		 *								in order to include them in the tokensized string.
		 * @param[in]	maxSplits		(optional) The maximum number of splits to perform (0 for unlimited splits).
		 *								If this parameters is > 0, the splitting process will stop after this many splits,
		 *								left to right.
		 */
		static Vector<String> Tokenise(const String& str, const String& delims = "\t\n ", const String& doubleDelims = "\"", unsigned int maxSplits = 0);

		/** @copydoc StringUtility::Tokenise(const String&, const String&, const String&, unsigned int) */
		static Vector<WString> Tokenise(const WString& str, const WString& delims = L"\t\n ", const WString& doubleDelims = L"\"", unsigned int maxSplits = 0);

		/** Converts one or multiple 32-bit numbers into a literal hexidecimal representation. Each entry is separated with a desh. */
		static String HexToLiteral(const u32* input, u32 count);

		/** Converts one or multiple 64-bit numbers into a literal hexidecimal representation. Each entry is separated with a desh. */
		static String HexToLiteral(const u64* input, u32 count);

		/** Converts all the characters in the string to lower case. Does not handle UTF8 encoded strings. */
		static void ToLowerCase(String& str);

		/** Converts all the characters in the string to lower case. Does not handle UTF8 encoded strings. */
		static void ToLowerCase(WString& str);

		/** Converts all the characters in the string to upper case. Does not handle UTF8 encoded strings. */
		static void ToUpperCase(String& str);

		/**	Converts all the characters in the string to upper case. Does not handle UTF8 encoded strings. */
		static void ToUpperCase(WString& str);

		/**
		 * Returns whether the string begins with the pattern passed in.
		 *
		 * @param[in]	str		 	String to compare.
		 * @param[in]	pattern		Pattern to compare with.
		 * @param[in]	lowerCase	(optional) If true, the start of the string will be lower cased before comparison, and
		 *							the pattern should also be in lower case.
		 */
		static bool StartsWith(const String& str, const String& pattern, bool lowerCase = true);

		/** @copydoc StartsWith(const String&, const String&, bool) */
		static bool StartsWith(const WString& str, const WString& pattern, bool lowerCase = true);

		/**
		 * Returns whether the string end with the pattern passed in.
		 *
		 * @param[in]	str		 	String to compare.
		 * @param[in]	pattern		Pattern to compare with.
		 * @param[in]	lowerCase	(optional) If true, the start of the string will be lower cased before comparison, and
		 *							the pattern should also be in lower case.
		 */
		static bool EndsWith(const String& str, const String& pattern, bool lowerCase = true);

		/** @copydoc EndsWith(const String&, const String&, bool) */
		static bool EndsWith(const WString& str, const WString& pattern, bool lowerCase = true);

		/**
		 * Returns true if the string matches the provided pattern. Pattern may use a "*" wildcard for matching any
		 * characters.
		 *
		 * @param[in]	str			 	The string to test.
		 * @param[in]	pattern		 	Patterns to look for.
		 * @param[in]	caseSensitive	(optional) Should the match be case sensitive or not.
		 */
		static bool Match(const String& str, const String& pattern, bool caseSensitive = true);

		/** @copydoc Match(const String&, const String&, bool) */
		static bool Match(const WString& str, const WString& pattern, bool caseSensitive = true);

		/**
		 * Replace all instances of a substring with a another substring.
		 *
		 * @param[in]	source		   	String to search.
		 * @param[in]	replaceWhat	   	Substring to find and replace
		 * @param[in]	replaceWithWhat	Substring to replace with (the new sub-string)
		 *
		 * @return	An updated string with the substrings replaced.
		 */
		static const String ReplaceAll(const String& source, const String& replaceWhat, const String& replaceWithWhat);

		/** @copydoc ReplaceAll(const String&, const String&, const String&) */
		static const WString ReplaceAll(const WString& source, const WString& replaceWhat, const WString& replaceWithWhat);

		/** Removes the file extension from the provided string. If string has multiple extensions, only the last extension will be removed. */
		static String StripExtension(const String& input);

		/**
		 * Compares two strings. Returns true if the two compare equal.
		 *
		 * @param[in]	lhs				Left string to compare.
		 * @param[in]	rhs				Right string to compare.
		 * @param[in]	caseSensitive	If true the comparison will consider uppercase and lowercase characters different.
		 *								Note that case conversion does not handle UTF8 strings.
		 */
		template <class T1, class T2>
		static bool Compare(const T1& lhs, const T2& rhs, bool caseSensitive = true)
		{
			auto fnCaseInsensitiveEquals = [](char lhs, char rhs)
			{
				return std::tolower(lhs) == std::tolower(rhs);
			};

			if(caseSensitive)
				return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());

			return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), fnCaseInsensitiveEquals);
		}

		/**
		 * Compares two strings. Returns true if the two compare equal.
		 *
		 * @param[in]	lhs				Left string to compare.
		 * @param[in]	rhs				Right string to compare.
		 * @param[in]	caseSensitive	If true the comparison will consider uppercase and lowercase characters different.
		 *								Note that case conversion does not handle UTF8 strings.
		 */
		template <class T1>
		static bool Compare(const T1& lhs, const char* rhs, bool caseSensitive = true)
		{
			return Compare(lhs, StringView(rhs), caseSensitive);
		}

		/**
		 * Compares two strings. Returns 0 if the two compare equal, <0 if the value of the left string is lower than of
		 * the right string, or >0 if the value of the left string is higher than the right string.
		 *
		 * @param[in]	lhs				Left string to compare.
		 * @param[in]	rhs				Right string to compare.
		 * @param[in]	caseSensitive	If true the comparison will consider uppercase and lowercase characters different.
		 *								Note that case conversion does not handle UTF8 strings.
		 */
		template <class T>
		static int LexicographicalCompare(const BasicString<T>& lhs, const BasicString<T>& rhs, bool caseSensitive = true)
		{
			if(caseSensitive)
				return (int)lhs.compare(rhs);

			int size = (int)std::min(lhs.size(), rhs.size());
			for(int characterIndex = 0; characterIndex < size; characterIndex++)
			{
				if(toupper(lhs[characterIndex]) < toupper(rhs[characterIndex])) return -1;
				if(toupper(lhs[characterIndex]) > toupper(rhs[characterIndex])) return 1;
			}

			return (lhs.size() < rhs.size() ? -1 : (lhs.size() == rhs.size() ? 0 : 1));
		}

		/** @copydoc StringFormat::Format */
		template <class T, class... Args>
		static BasicString<T> Format(const BasicString<T>& source, Args&&... args)
		{
			return StringFormat::Format(source.c_str(), std::forward<Args>(args)...);
		}

		/** @copydoc StringFormat::Format */
		template <class T, class... Args>
		static BasicString<T> Format(const T* source, Args&&... args)
		{
			return StringFormat::Format(source, std::forward<Args>(args)...);
		}

		/** Constant blank string, useful for returning by ref where local does not exist. */
		static const String kBlank;

		/**	Constant blank wide string, useful for returning by ref where local does not exist. */
		static const WString kWblank;

	private:
		template <class T>
		static Vector<BasicString<T>> SplitInternal(const BasicString<T>& str, const BasicString<T>& delims, unsigned int maxSplits)
		{
			Vector<BasicString<T>> ret;
			// Pre-allocate some space for performance
			ret.reserve(maxSplits ? maxSplits + 1 : 10); // 10 is guessed capacity for most case

			unsigned int numSplits = 0;

			// Use STL methods
			size_t start, pos;
			start = 0;
			do
			{
				pos = str.find_first_of(delims, start);
				if(pos == start)
				{
					// Do nothing
					start = pos + 1;
				}
				else if(pos == BasicString<T>::npos || (maxSplits && numSplits == maxSplits))
				{
					// Copy the rest of the string
					ret.push_back(str.substr(start));
					break;
				}
				else
				{
					// Copy up to delimiter
					ret.push_back(str.substr(start, pos - start));
					start = pos + 1;
				}
				// parse up to next real data
				start = str.find_first_not_of(delims, start);
				++numSplits;
			}
			while(pos != BasicString<T>::npos);

			return ret;
		}

		template <class T>
		static Vector<BasicString<T>> TokeniseInternal(const BasicString<T>& str, const BasicString<T>& singleDelims, const BasicString<T>& doubleDelims, unsigned int maxSplits)
		{
			Vector<BasicString<T>> ret;
			// Pre-allocate some space for performance
			ret.reserve(maxSplits ? maxSplits + 1 : 10); // 10 is guessed capacity for most case

			unsigned int numSplits = 0;
			BasicString<T> delims = singleDelims + doubleDelims;

			// Use STL methods
			size_t start, pos;
			T curDoubleDelim = 0;
			start = 0;
			do
			{
				if(curDoubleDelim != 0)
				{
					pos = str.find(curDoubleDelim, start);
				}
				else
				{
					pos = str.find_first_of(delims, start);
				}

				if(pos == start)
				{
					T curDelim = str.at(pos);
					if(doubleDelims.find_first_of(curDelim) != BasicString<T>::npos)
					{
						curDoubleDelim = curDelim;
					}
					// Do nothing
					start = pos + 1;
				}
				else if(pos == BasicString<T>::npos || (maxSplits && numSplits == maxSplits))
				{
					if(curDoubleDelim != 0)
					{
						// Missing closer. Warn or throw exception?
					}
					// Copy the rest of the string
					ret.push_back(str.substr(start));
					break;
				}
				else
				{
					if(curDoubleDelim != 0)
					{
						curDoubleDelim = 0;
					}

					// Copy up to delimiter
					ret.push_back(str.substr(start, pos - start));
					start = pos + 1;
				}
				if(curDoubleDelim == 0)
				{
					// parse up to next real data
					start = str.find_first_not_of(singleDelims, start);
				}

				++numSplits;
			}
			while(pos != BasicString<T>::npos);

			return ret;
		}

		template <class T>
		static bool StartsWithInternal(const BasicString<T>& str, const BasicString<T>& pattern, bool lowerCase)
		{
			size_t thisLen = str.length();
			size_t patternLen = pattern.length();
			if(thisLen < patternLen || patternLen == 0)
				return false;

			BasicString<T> startOfThis = str.substr(0, patternLen);
			if(lowerCase)
				StringUtility::ToLowerCase(startOfThis);

			return (startOfThis == pattern);
		}

		template <class T>
		static bool EndsWithInternal(const BasicString<T>& str, const BasicString<T>& pattern, bool lowerCase)
		{
			size_t thisLen = str.length();
			size_t patternLen = pattern.length();
			if(thisLen < patternLen || patternLen == 0)
				return false;

			BasicString<T> endOfThis = str.substr(thisLen - patternLen, patternLen);
			if(lowerCase)
				StringUtility::ToLowerCase(endOfThis);

			return (endOfThis == pattern);
		}

		template <class T>
		static bool MatchInternal(const BasicString<T>& str, const BasicString<T>& pattern, bool caseSensitive)
		{
			BasicString<T> tmpStr = str;
			BasicString<T> tmpPattern = pattern;
			if(!caseSensitive)
			{
				StringUtility::ToLowerCase(tmpStr);
				StringUtility::ToLowerCase(tmpPattern);
			}

			typename BasicString<T>::const_iterator strIt = tmpStr.begin();
			typename BasicString<T>::const_iterator patIt = tmpPattern.begin();
			typename BasicString<T>::const_iterator lastWildCardIt = tmpPattern.end();
			while(strIt != tmpStr.end() && patIt != tmpPattern.end())
			{
				if(*patIt == '*')
				{
					lastWildCardIt = patIt;
					// Skip over looking for next character
					++patIt;
					if(patIt == tmpPattern.end())
					{
						// Skip right to the end since * matches the entire rest of the string
						strIt = tmpStr.end();
					}
					else
					{
						// scan until we find next pattern character
						while(strIt != tmpStr.end() && *strIt != *patIt)
							++strIt;
					}
				}
				else
				{
					if(*patIt != *strIt)
					{
						if(lastWildCardIt != tmpPattern.end())
						{
							// The last wildcard can match this incorrect sequence
							// rewind pattern to wildcard and keep searching
							patIt = lastWildCardIt;
							lastWildCardIt = tmpPattern.end();
						}
						else
						{
							// no wildwards left
							return false;
						}
					}
					else
					{
						++patIt;
						++strIt;
					}
				}
			}

			// If we reached the end of both the pattern and the string, we succeeded
			if(patIt == tmpPattern.end() && strIt == tmpStr.end())
				return true;
			else
				return false;
		}

		template <class T>
		static BasicString<T> ReplaceAllInternal(const BasicString<T>& source, const BasicString<T>& replaceWhat, const BasicString<T>& replaceWithWhat)
		{
			BasicString<T> result = source;
			typename BasicString<T>::size_type pos = 0;
			while(1)
			{
				pos = result.find(replaceWhat, pos);
				if(pos == BasicString<T>::npos) break;
				result.replace(pos, replaceWhat.size(), replaceWithWhat);
				pos += replaceWithWhat.size();
			}
			return result;
		}
	};

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for String. */
	template <>
	struct hash<b3d::String>
	{
		size_t operator()(const b3d::String& string) const
		{
			size_t hash = 0;
			for(size_t characterIndex = 0; characterIndex < string.size(); characterIndex++)
				hash = 65599 * hash + string[characterIndex];
			return hash ^ (hash >> 16);
		}
	};

	/**	Hash value generator for WString. */
	template <>
	struct hash<b3d::WString>
	{
		size_t operator()(const b3d::WString& string) const
		{
			size_t hash = 0;
			for(size_t characterIndex = 0; characterIndex < string.size(); characterIndex++)
				hash = 65599 * hash + string[characterIndex];
			return hash ^ (hash >> 16);
		}
	};
} // namespace std

/** @endcond */

namespace b3d
{
	/** @addtogroup String
	 *  @{
	 */

	/** Hash function that allows map lookups using String, StringView or const char*. */
	struct StringHash
	{
		using is_transparent = void;

		[[nodiscard]] size_t operator()(const char* value) const
		{
			return std::hash<StringView>{}(value);
		}

		[[nodiscard]] size_t operator()(StringView value) const
		{
			return std::hash<StringView>{}(value);
		}

		[[nodiscard]] size_t operator()(const String& value) const
		{
			return std::hash<String>{}(value);
		}
	};

	/** @} */
}
