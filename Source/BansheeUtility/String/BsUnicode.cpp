//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsUnicode.h"

namespace bs
{
	/** Converts an UTF-8 encoded character (possibly multibyte) into an UTF-32 character. */
	template<typename T>
	T UTF8To32(T begin, T end, char32_t& output, char32_t invalidChar = 0)
	{
		// Nothing to parse
		if (begin >= end)
			return begin;

		// Determine the number of bytes used by the character
		UINT32 numBytes;

		UINT8 firstByte = (UINT8)*begin;
		if (firstByte < 192)
			numBytes = 1;
		else if (firstByte < 224)
			numBytes = 2;
		else if (firstByte < 240)
			numBytes = 3;
		else if (firstByte < 248)
			numBytes = 4;
		else if (firstByte < 252)
			numBytes = 5;
		else // < 256
			numBytes = 6;

		// Not enough bytes were provided, invalid character
		if((begin + numBytes) > end)
		{
			output = invalidChar;
			return end;
		}

		// Decode the character
		output = 0;
		switch(numBytes)
		{
		case 6: output += (UINT8)(*begin); ++begin; output <<= 6;
		case 5: output += (UINT8)(*begin); ++begin; output <<= 6;
		case 4: output += (UINT8)(*begin); ++begin; output <<= 6;
		case 3: output += (UINT8)(*begin); ++begin; output <<= 6;
		case 2: output += (UINT8)(*begin); ++begin; output <<= 6;
		case 1: output += (UINT8)(*begin); ++begin;
		default: break;
		}

		constexpr UINT32 offsets[6] = { 0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080 };
		output -= offsets[numBytes - 1];

		return begin;
	}	

	/** Converts an UTF-32 encoded character into an (possibly multibyte) UTF-8 character. */
	template<typename T>
	T UTF32To8(char32_t input, T output, UINT32 maxElems, char invalidChar = 0)
	{
		// No place to write the character
		if (maxElems == 0)
			return output;

		// Check if character is valid
		if ((input > 0x0010FFFF) || ((input >= 0xD800) && (input <= 0xDBFF)))
		{
			*output = invalidChar;
			++output;

			return output;
		}

		// Determine the number of bytes used by the character
		UINT32 numBytes;
		if (input <  0x80)
			numBytes = 1;
		else if (input < 0x800)
			numBytes = 2;
		else if (input < 0x10000) 
			numBytes = 3;
		else // <= 0x0010FFFF 
			numBytes = 4;

		// Check if we have enough space
		if(numBytes > maxElems)
		{
			*output = invalidChar;
			++output;

			return output;
		}

		// Encode the character
		constexpr UINT8 headers[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

		char bytes[4];
		switch (numBytes)
		{
			case 4: bytes[3] = (char)((input | 0x80) & 0xBF); input >>= 6;
			case 3: bytes[2] = (char)((input | 0x80) & 0xBF); input >>= 6;
			case 2: bytes[1] = (char)((input | 0x80) & 0xBF); input >>= 6;
			case 1: bytes[0] = (char)(input | headers[numBytes]);
			default: break;
		}

		output = std::copy(bytes, bytes + numBytes, output);
		return output;
	}

	/** Converts an UTF-16 encoded character into an UTF-32 character. */
	template<typename T>
	T UTF16To32(T begin, T end, char32_t& output, char32_t invalidChar = 0)
	{
		// Nothing to parse
		if (begin >= end)
			return begin;

		char16_t firstElem = (char16_t)*begin;
		++begin;

		// Check if it's a surrogate pair
		if ((firstElem >= 0xD800) && (firstElem <= 0xDBFF))
		{
			// Invalid character
			if (begin >= end)
			{
				output = invalidChar;
				return end;
			}

			char32_t secondElem = (char32_t)*begin;
			++begin;

			if ((secondElem >= 0xDC00) && (secondElem <= 0xDFFF))
				output = (char32_t)(((firstElem - 0xD800) << 10) + (secondElem - 0xDC00) + 0x0010000);
			else // Invalid character
				output = invalidChar;
		}
		else
		{
			output = (char32_t)firstElem;
			return begin;
		}

		return begin;
	}	

	/** Converts an UTF-32 encoded character into an UTF-16 character. */
	template<typename T>
	T UTF32To16(char32_t input, T output, UINT32 maxElems, char16_t invalidChar = 0)
	{
		// No place to write the character
		if (maxElems == 0)
			return output;

		// Invalid character
		if (input > 0x0010FFFF)
		{
			*output = invalidChar;
			++output;

			return output;
		}

		// Can be encoded as a single element
		if (input <= 0xFFFF)
		{
			// Check if in valid range
			if ((input >= 0xD800) && (input <= 0xDFFF))
			{
				*output = invalidChar;
				++output;

				return output;
			}

			*output = (char16_t)input;
			++output;
		}
		else // Must be encoded as two elements
		{
			// Two elements won't fit
			if (maxElems < 2)
			{
				*output = invalidChar;
				++output;

				return output;
			}

			input -= 0x0010000;

			*output = (char16_t)((input >> 10) + 0xD800);
			++output;

			*output = (char16_t)((input & 0x3FFUL) + 0xDC00);
			++output;
		}

		return output;
	}

	template<typename T>
	T wideToUTF32(T begin, T end, char32_t& output, char32_t invalidChar = 0)
	{
		if (sizeof(wchar_t) == 4) // Assuming UTF-32 (i.e. Unix)
		{
			output = (char32_t)*begin;
			++begin;

			return begin;
		}
		else // Assuming UTF-16 (i.e. Windows)
			return UTF16To32(begin, end, output, invalidChar);

	}

	char32_t ANSIToUTF32(char input, const std::locale& locale = std::locale(""))
	{
		const std::ctype<wchar_t>& facet = std::use_facet<std::ctype<wchar_t>>(locale);

		// Note: Not exactly valid on Windows, since the input character could require a surrogate pair.
		//       Consider improving this if it ever becomes an issue.
		wchar_t wideChar = facet.widen(input);

		char32_t output;
		wideToUTF32(&wideChar, &wideChar + 1, output);

		return output;
	}

	template<typename T>
	T UTF32ToWide(char32_t input, T output, UINT32 maxElems, wchar_t invalidChar = 0)
	{
		if(sizeof(wchar_t) == 4) // Assuming UTF-32 (i.e. Unix)
		{
			*output = (wchar_t)input;
			++output;

			return output;
		}
		else // Assuming UTF-16 (i.e. Windows)
			return UTF32To16(input, output, maxElems, invalidChar);
	}

	char UTF32ToANSI(char32_t input, char invalidChar = 0, const std::locale& locale = std::locale(""))
	{
		const std::ctype<wchar_t>& facet = std::use_facet<std::ctype<wchar_t>>(locale);

		// Note: Same as above, not exactly correct as narrow() doesn't accept a surrogate pair
		return facet.narrow((wchar_t)input, invalidChar);
	}

	String UTF8::fromANSI(const String& input, const std::locale& locale)
	{
		String output;
		output.reserve(input.size());

		auto backInserter = std::back_inserter(output);

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char = ANSIToUTF32(*iter, locale);
			UTF32To8(u32char, backInserter, 4);

			++iter;
		}

		return output;
	}

	String UTF8::toANSI(const String& input, const std::locale& locale, char invalidChar)
	{
		String output;

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char;
			iter = UTF8To32(iter, input.end(), u32char, invalidChar);

			output.push_back(UTF32ToANSI(u32char, invalidChar, locale));
		}

		return output;
	}

	String UTF8::fromWide(const WString& input)
	{
		String output;
		output.reserve(input.size());

		auto backInserter = std::back_inserter(output);

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char;
			iter = wideToUTF32(iter, input.end(), u32char);
			UTF32To8(u32char, backInserter, 4);
		}

		return output;
	}

	WString UTF8::toWide(const String& input) 
	{
		WString output;
		auto backInserter = std::back_inserter(output);

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char;
			iter = UTF8To32(iter, input.end(), u32char);

			UTF32ToWide(u32char, backInserter, 2);
		}

		return output;
	}

	String UTF8::fromUTF16(const U16String& input)
	{
		String output;
		output.reserve(input.size());

		auto backInserter = std::back_inserter(output);

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char = 0;
			iter = UTF16To32(iter, input.end(), u32char);
			UTF32To8(u32char, backInserter, 4);
		}

		return output;
	}

	U16String UTF8::toUTF16(const String& input) 
	{
		U16String output;
		auto backInserter = std::back_inserter(output);

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char;
			iter = UTF8To32(iter, input.end(), u32char);

			UTF32To16(u32char, backInserter, 2);
		}

		return output;
	}

	String UTF8::fromUTF32(const U32String& input)
	{
		String output;
		output.reserve(input.size());

		auto backInserter = std::back_inserter(output);

		auto iter = input.begin();
		while(iter != input.end())
		{
			UTF32To8(*iter, backInserter, 4);

			++iter;
		}

		return output;
	}

	U32String UTF8::toUTF32(const String& input) 
	{
		U32String output;

		auto iter = input.begin();
		while(iter != input.end())
		{
			char32_t u32char;
			iter = UTF8To32(iter, input.end(), u32char);

			output.push_back(u32char);
		}

		return output;
	}
}
