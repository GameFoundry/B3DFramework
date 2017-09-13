//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsUUID.h"
#include "Utility/BsPlatformUtility.h"
#include <chrono>

using namespace std::chrono;

namespace bs
{
	std::array<UINT8, 256> literalToHex()
	{
		std::array<UINT8, 256> output;
		output.fill(-1);

		output['0'] = 0;
		output['1'] = 1;
		output['2'] = 2;
		output['3'] = 3;
		output['4'] = 4;
		output['5'] = 5;
		output['6'] = 6;
		output['7'] = 7;
		output['8'] = 8;
		output['9'] = 9;
		output['a'] = 10;
		output['b'] = 11;
		output['c'] = 12;
		output['d'] = 13;
		output['e'] = 14;
		output['f'] = 15;

		return output;
	}

	static const std::array<char, 16> HEX_TO_LITERAL = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	static const std::array<UINT8, 256> LITERAL_TO_HEX = literalToHex();

	UUID UUID::EMPTY;

	UUID::UUID(const String& uuid)
	{
		memset(mData, 0, sizeof(mData));

		if (uuid.size() < 36)
			return;

		UINT32 idx = 0;

		// First group: 8 digits
		for(INT32 i = 7; i >= 0; --i)
		{
			char charVal = (char)tolower(uuid[idx++]);
			UINT8 hexVal = LITERAL_TO_HEX[charVal];

			mData[0] |= hexVal << (i * 4);
		}

		idx++;
		
		// Second group: 4 digits
		for(INT32 i = 7; i >= 4; --i)
		{
			char charVal = (char)tolower(uuid[idx++]);
			UINT8 hexVal = LITERAL_TO_HEX[charVal];

			mData[1] |= hexVal << (i * 4);
		}

		idx++;

		// Third group: 4 digits
		for(INT32 i = 3; i >= 0; --i)
		{
			char charVal = (char)tolower(uuid[idx++]);
			UINT8 hexVal = LITERAL_TO_HEX[charVal];

			mData[1] |= hexVal << (i * 4);
		}

		idx++;

		// Fourth group: 4 digits
		for(INT32 i = 7; i >= 4; --i)
		{
			char charVal = (char)tolower(uuid[idx++]);
			UINT8 hexVal = LITERAL_TO_HEX[charVal];

			mData[2] |= hexVal << (i * 4);
		}

		idx++;

		// Fifth group: 12 digits
		for(INT32 i = 3; i >= 0; --i)
		{
			char charVal = (char)tolower(uuid[idx++]);
			UINT8 hexVal = LITERAL_TO_HEX[charVal];

			mData[2] |= hexVal << (i * 4);
		}

		for(INT32 i = 7; i >= 0; --i)
		{
			char charVal = (char)tolower(uuid[idx++]);
			UINT8 hexVal = LITERAL_TO_HEX[charVal];

			mData[3] |= hexVal << (i * 4);
		}
	}

	String UUID::toString() const
	{
		UINT8 output[36];
		UINT32 idx = 0;

		// First group: 8 digits
		for(INT32 i = 7; i >= 0; --i)
		{
			UINT32 hexVal = (mData[0] >> (i * 4)) & 0xF;
			output[idx++] = HEX_TO_LITERAL[hexVal];
		}

		output[idx++] = '-';

		// Second group: 4 digits
		for(INT32 i = 7; i >= 4; --i)
		{
			UINT32 hexVal = (mData[1] >> (i * 4)) & 0xF;
			output[idx++] = HEX_TO_LITERAL[hexVal];
		}

		output[idx++] = '-';

		// Third group: 4 digits
		for(INT32 i = 3; i >= 0; --i)
		{
			UINT32 hexVal = (mData[1] >> (i * 4)) & 0xF;
			output[idx++] = HEX_TO_LITERAL[hexVal];
		}

		output[idx++] = '-';

		// Fourth group: 4 digits
		for(INT32 i = 7; i >= 4; --i)
		{
			UINT32 hexVal = (mData[2] >> (i * 4)) & 0xF;
			output[idx++] = HEX_TO_LITERAL[hexVal];
		}

		output[idx++] = '-';

		// Fifth group: 12 digits
		for(INT32 i = 3; i >= 0; --i)
		{
			UINT32 hexVal = (mData[2] >> (i * 4)) & 0xF;
			output[idx++] = HEX_TO_LITERAL[hexVal];
		}

		for(INT32 i = 7; i >= 0; --i)
		{
			UINT32 hexVal = (mData[3] >> (i * 4)) & 0xF;
			output[idx++] = HEX_TO_LITERAL[hexVal];
		}

		return String((const char*)output, 36);
	}

	UUID UUIDGenerator::generateRandom()
	{
		return PlatformUtility::generateUUID();
	}
};