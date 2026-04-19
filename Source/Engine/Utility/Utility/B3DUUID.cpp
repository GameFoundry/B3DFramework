//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DUUID.h"
#include "Utility/B3DPlatformUtility.h"

namespace
{
constexpr const char kHexToLiteral[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

constexpr const b3d::u8 kLiteralToHex[256] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   // 0 through 9 translate to 0  though 9
											   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   // A through F translate to 10 though 15
											   0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   // a through f translate to 10 though 15
											   0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
											   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
} // namespace

using namespace b3d;

const UUID UUID::kEmpty;

UUID::UUID(const String& uuid)
{
	memset(mData, 0, sizeof(mData));

	if(uuid.size() < 36)
		return;

	u32 index = 0;

	// First group: 8 digits
	for(i32 digitIndex = 7; digitIndex >= 0; --digitIndex)
	{
		char character = uuid[index++];
		u8 hexValue = kLiteralToHex[(int)character];

		mData[0] |= hexValue << (digitIndex * 4);
	}

	index++;

	// Second group: 4 digits
	for(i32 digitIndex = 7; digitIndex >= 4; --digitIndex)
	{
		char character = uuid[index++];
		u8 hexValue = kLiteralToHex[(int)character];

		mData[1] |= hexValue << (digitIndex * 4);
	}

	index++;

	// Third group: 4 digits
	for(i32 digitIndex = 3; digitIndex >= 0; --digitIndex)
	{
		char character = uuid[index++];
		u8 hexValue = kLiteralToHex[(int)character];

		mData[1] |= hexValue << (digitIndex * 4);
	}

	index++;

	// Fourth group: 4 digits
	for(i32 digitIndex = 7; digitIndex >= 4; --digitIndex)
	{
		char character = uuid[index++];
		u8 hexValue = kLiteralToHex[(int)character];

		mData[2] |= hexValue << (digitIndex * 4);
	}

	index++;

	// Fifth group: 12 digits
	for(i32 digitIndex = 3; digitIndex >= 0; --digitIndex)
	{
		char character = uuid[index++];
		u8 hexValue = kLiteralToHex[(int)character];

		mData[2] |= hexValue << (digitIndex * 4);
	}

	for(i32 digitIndex = 7; digitIndex >= 0; --digitIndex)
	{
		char character = uuid[index++];
		u8 hexValue = kLiteralToHex[(int)character];

		mData[3] |= hexValue << (digitIndex * 4);
	}
}

String UUID::ToString() const
{
	u8 output[36];
	u32 index = 0;

	// First group: 8 digits
	for(i32 digitIndex = 7; digitIndex >= 0; --digitIndex)
	{
		u32 hexValue = (mData[0] >> (digitIndex * 4)) & 0xF;
		output[index++] = kHexToLiteral[hexValue];
	}

	output[index++] = '-';

	// Second group: 4 digits
	for(i32 digitIndex = 7; digitIndex >= 4; --digitIndex)
	{
		u32 hexValue = (mData[1] >> (digitIndex * 4)) & 0xF;
		output[index++] = kHexToLiteral[hexValue];
	}

	output[index++] = '-';

	// Third group: 4 digits
	for(i32 digitIndex = 3; digitIndex >= 0; --digitIndex)
	{
		u32 hexValue = (mData[1] >> (digitIndex * 4)) & 0xF;
		output[index++] = kHexToLiteral[hexValue];
	}

	output[index++] = '-';

	// Fourth group: 4 digits
	for(i32 digitIndex = 7; digitIndex >= 4; --digitIndex)
	{
		u32 hexValue = (mData[2] >> (digitIndex * 4)) & 0xF;
		output[index++] = kHexToLiteral[hexValue];
	}

	output[index++] = '-';

	// Fifth group: 12 digits
	for(i32 digitIndex = 3; digitIndex >= 0; --digitIndex)
	{
		u32 hexValue = (mData[2] >> (digitIndex * 4)) & 0xF;
		output[index++] = kHexToLiteral[hexValue];
	}

	for(i32 digitIndex = 7; digitIndex >= 0; --digitIndex)
	{
		u32 hexValue = (mData[3] >> (digitIndex * 4)) & 0xF;
		output[index++] = kHexToLiteral[hexValue];
	}

	return String((const char*)output, 36);
}

UUID UUIDGenerator::GenerateRandom()
{
	return PlatformUtility::GenerateUuid();
}
