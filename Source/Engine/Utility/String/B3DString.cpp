//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include <codecvt>
#include <locale>

#include "B3DUnicode.h"
#include "Image/B3DColor.h"
#include "Math/B3DMath.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Debug/B3DLog.h"

using namespace b3d;

const String StringUtility::kBlank;
const WString StringUtility::kWblank;

String StringUtility::Trim(const String& str, bool left, bool right)
{
	static const String kDelims = " \t\r";
	return Trim(str, kDelims, left, right);
}

WString StringUtility::Trim(const WString& str, bool left, bool right)
{
	static const WString kDelims = L" \t\r";
	return Trim(str, kDelims, left, right);
}

String StringUtility::Trim(const String& str, const String& delims, bool left, bool right)
{
	String output = str;

	if(right)
		output.erase(str.find_last_not_of(delims) + 1); // trim right
	if(left)
		output.erase(0, str.find_first_not_of(delims)); // trim left

	return output;
}

WString StringUtility::Trim(const WString& str, const WString& delims, bool left, bool right)
{
	WString output = str;

	if(right)
		output.erase(str.find_last_not_of(delims) + 1); // trim right
	if(left)
		output.erase(0, str.find_first_not_of(delims)); // trim left

	return output;
}

StringView StringUtility::Trim(StringView str, bool left, bool right)
{
	static constexpr StringView kDelims = " \t\r";
	return Trim(str, kDelims, left, right);
}

StringView StringUtility::Trim(StringView str, StringView delims, bool left, bool right)
{
	if (str.empty())
		return str;

	size_t start = 0;
	size_t end = str.size();

	if (left)
	{
		start = str.find_first_not_of(delims);
		if (start == StringView::npos)
			return StringView();
	}

	if (right)
	{
		end = str.find_last_not_of(delims);
		if (end == StringView::npos)
			return StringView();
		end++;
	}

	return str.substr(start, end - start);
}

Vector<String> StringUtility::Split(const String& str, const String& delims, unsigned int maxSplits)
{
	return SplitInternal<char>(str, delims, maxSplits);
}

Vector<WString> StringUtility::Split(const WString& str, const WString& delims, unsigned int maxSplits)
{
	return SplitInternal<wchar_t>(str, delims, maxSplits);
}

Vector<String> StringUtility::Tokenise(const String& str, const String& singleDelims, const String& doubleDelims, unsigned int maxSplits)
{
	return TokeniseInternal<char>(str, singleDelims, doubleDelims, maxSplits);
}

Vector<WString> StringUtility::Tokenise(const WString& str, const WString& singleDelims, const WString& doubleDelims, unsigned int maxSplits)
{
	return TokeniseInternal<wchar_t>(str, singleDelims, doubleDelims, maxSplits);
}

String StringUtility::HexToLiteral(const u32* input, u32 count)
{
	if(input == nullptr || count == 0)
		return kBlank;

	constexpr Array<char, 16> kHexadecimalToLiteral = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	String output;
	output.reserve((size_t)count * 8);

	for(u32 index = 0; index < count; index++)
	{
		for(i32 digitIndex = 7; digitIndex >= 0; --digitIndex)
		{
			const i32 hexadecimalValue = (input[index] >> (digitIndex * 4)) & 0xF;
			output.push_back(kHexadecimalToLiteral[hexadecimalValue]);
		}

		if(index < (count - 1))
			output.push_back('-');
	}

	return output;
}

String StringUtility::HexToLiteral(const u64* input, u32 count)
{
	if(input == nullptr || count == 0)
		return kBlank;

	constexpr Array<char, 16> kHexadecimalToLiteral = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	String output;
	output.reserve((size_t)count * 16);

	for(u32 index = 0; index < count; index++)
	{
		for(i32 digitIndex = 15; digitIndex >= 0; --digitIndex)
		{
			const i32 hexadecimalValue = (input[index] >> (digitIndex * 4)) & 0xF;
			output.push_back(kHexadecimalToLiteral[hexadecimalValue]);
		}

		if(index < (count - 1))
			output.push_back('-');
	}

	return output;
}

void StringUtility::ToLowerCase(String& str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
}

void StringUtility::ToLowerCase(WString& str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
}

void StringUtility::ToUpperCase(String& str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void StringUtility::ToUpperCase(WString& str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}

bool StringUtility::StartsWith(const String& str, const String& pattern, bool lowerCase)
{
	return StartsWithInternal<char>(str, pattern, lowerCase);
}

bool StringUtility::StartsWith(const WString& str, const WString& pattern, bool lowerCase)
{
	return StartsWithInternal<wchar_t>(str, pattern, lowerCase);
}

bool StringUtility::EndsWith(const String& str, const String& pattern, bool lowerCase)
{
	return EndsWithInternal<char>(str, pattern, lowerCase);
}

bool StringUtility::EndsWith(const WString& str, const WString& pattern, bool lowerCase)
{
	return EndsWithInternal<wchar_t>(str, pattern, lowerCase);
}

bool StringUtility::Match(const String& str, const String& pattern, bool caseSensitive)
{
	return MatchInternal<char>(str, pattern, caseSensitive);
}

bool StringUtility::Match(const WString& str, const WString& pattern, bool caseSensitive)
{
	return MatchInternal<wchar_t>(str, pattern, caseSensitive);
}

const String StringUtility::ReplaceAll(const String& source, const String& replaceWhat, const String& replaceWithWhat)
{
	return ReplaceAllInternal<char>(source, replaceWhat, replaceWithWhat);
}

const WString StringUtility::ReplaceAll(const WString& source, const WString& replaceWhat, const WString& replaceWithWhat)
{
	return ReplaceAllInternal<wchar_t>(source, replaceWhat, replaceWithWhat);
}

String StringUtility::StripExtension(const String& input)
{
	String::size_type position = input.rfind('.');
	if(position != String::npos)
		return input.substr(0, position);

	return input;
	
}

/************************************************************************/
/* 						VARIOUS TO STRING CONVERSIONS                   */
/************************************************************************/

namespace b3d
{
WString ToWString(const String& source)
{
	return WString(source.begin(), source.end());
}

WString ToWString(const char* source)
{
	return ToWString(String(source));
}

WString ToWString(float val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.precision(precision);
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(double val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.precision(precision);
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(const Radian& val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	return ToWString(val.GetValueInRadians(), precision, width, fill, flags);
}

WString ToWString(const Degree& val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	return ToWString(val.GetValueInDegrees(), precision, width, fill, flags);
}

WString ToWString(int val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(unsigned int val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(i64 val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(u64 val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(char val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(wchar_t val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	WStringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

WString ToWString(const Vector2& val)
{
	WStringStream stream;
	stream << val.X << L" " << val.Y;
	return stream.str();
}

WString ToWString(const Vector2I& val)
{
	WStringStream stream;
	stream << val.X << L" " << val.Y;
	return stream.str();
}

WString ToWString(const Vector3& val)
{
	WStringStream stream;
	stream << val.X << L" " << val.Y << L" " << val.Z;
	return stream.str();
}

WString ToWString(const Vector4& val)
{
	WStringStream stream;
	stream << val.X << L" " << val.Y << L" " << val.Z << L" " << val.W;
	return stream.str();
}

WString ToWString(const Matrix3& val)
{
	WStringStream stream;
	stream << val[0][0] << L" "
		   << val[0][1] << L" "
		   << val[0][2] << L" "
		   << val[1][0] << L" "
		   << val[1][1] << L" "
		   << val[1][2] << L" "
		   << val[2][0] << L" "
		   << val[2][1] << L" "
		   << val[2][2];
	return stream.str();
}

WString ToWString(bool value, bool yesNo)
{
	if(value)
	{
		if(yesNo)
		{
			return L"yes";
		}
		else
		{
			return L"true";
		}
	}
	else if(yesNo)
	{
		return L"no";
	}
	else
	{
		return L"false";
	}
}

WString ToWString(const Matrix4& val)
{
	WStringStream stream;
	stream << val[0][0] << L" "
		   << val[0][1] << L" "
		   << val[0][2] << L" "
		   << val[0][3] << L" "
		   << val[1][0] << L" "
		   << val[1][1] << L" "
		   << val[1][2] << L" "
		   << val[1][3] << L" "
		   << val[2][0] << L" "
		   << val[2][1] << L" "
		   << val[2][2] << L" "
		   << val[2][3] << L" "
		   << val[3][0] << L" "
		   << val[3][1] << L" "
		   << val[3][2] << L" "
		   << val[3][3];
	return stream.str();
}

WString ToWString(const Quaternion& val)
{
	WStringStream stream;
	stream << val.W << L" " << val.X << L" " << val.Y << L" " << val.Z;
	return stream.str();
}

WString ToWString(const Color& val)
{
	WStringStream stream;
	stream << val.R << L" " << val.G << L" " << val.B << L" " << val.A;
	return stream.str();
}

WString ToWString(const Vector<b3d::WString>& val)
{
	WStringStream stream;
	Vector<b3d::WString>::const_iterator it, iteratorEnd, iteratorBegin;
	iteratorBegin = val.begin();
	iteratorEnd = val.end();
	for(it = iteratorBegin; it != iteratorEnd; ++it)
	{
		if(it != iteratorBegin)
			stream << L" ";

		stream << *it;
	}
	return stream.str();
}

String ToString(const WString& source)
{
	return UTF8::FromWide(source);
}

String ToString(const wchar_t* source)
{
	return ToString(WString(source));
}

String ToString(float val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.precision(precision);
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(double val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.precision(precision);
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(const Radian& val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	return ToString(val.GetValueInRadians(), precision, width, fill, flags);
}

String ToString(const Degree& val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	return ToString(val.GetValueInDegrees(), precision, width, fill, flags);
}

String ToString(int val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(unsigned int val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(long val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(unsigned long val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(i64 val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(u64 val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	StringStream stream;
	stream.width(width);
	stream.fill(fill);
	if(flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}

String ToString(const Vector2& val)
{
	StringStream stream;
	stream << val.X << " " << val.Y;
	return stream.str();
}

String ToString(const Vector2I& val)
{
	StringStream stream;
	stream << val.X << " " << val.Y;
	return stream.str();
}

String ToString(const Vector3& val)
{
	StringStream stream;
	stream << val.X << " " << val.Y << " " << val.Z;
	return stream.str();
}

String ToString(const Vector4& val)
{
	StringStream stream;
	stream << val.X << " " << val.Y << " " << val.Z << " " << val.W;
	return stream.str();
}

String ToString(const Matrix3& val)
{
	StringStream stream;
	stream << val[0][0] << " "
		   << val[0][1] << " "
		   << val[0][2] << " "
		   << val[1][0] << " "
		   << val[1][1] << " "
		   << val[1][2] << " "
		   << val[2][0] << " "
		   << val[2][1] << " "
		   << val[2][2];
	return stream.str();
}

String ToString(bool value, bool yesNo)
{
	if(value)
	{
		if(yesNo)
		{
			return "yes";
		}
		else
		{
			return "true";
		}
	}
	else if(yesNo)
	{
		return "no";
	}
	else
	{
		return "false";
	}
}

String ToString(const Matrix4& val)
{
	StringStream stream;
	stream << val[0][0] << " "
		   << val[0][1] << " "
		   << val[0][2] << " "
		   << val[0][3] << " "
		   << val[1][0] << " "
		   << val[1][1] << " "
		   << val[1][2] << " "
		   << val[1][3] << " "
		   << val[2][0] << " "
		   << val[2][1] << " "
		   << val[2][2] << " "
		   << val[2][3] << " "
		   << val[3][0] << " "
		   << val[3][1] << " "
		   << val[3][2] << " "
		   << val[3][3];
	return stream.str();
}

String ToString(const Quaternion& val)
{
	StringStream stream;
	stream << val.W << " " << val.X << " " << val.Y << " " << val.Z;
	return stream.str();
}

String ToString(const Color& val)
{
	StringStream stream;
	stream << val.R << " " << val.G << " " << val.B << " " << val.A;
	return stream.str();
}

String ToString(const UUID& val)
{
	return val.ToString();
}

String ToString(const Path& val)
{
	return val.ToString();
}

String ToString(const Vector<b3d::String>& val)
{
	StringStream stream;
	Vector<b3d::String>::const_iterator it, iteratorEnd, iteratorBegin;
	iteratorBegin = val.begin();
	iteratorEnd = val.end();
	for(it = iteratorBegin; it != iteratorEnd; ++it)
	{
		if(it != iteratorBegin)
			stream << " ";

		stream << *it;
	}
	return stream.str();
}

String ToString(const LogVerbosity& val)
{
	switch(val)
	{
	case LogVerbosity::Fatal:
		return "Fatal";
	case LogVerbosity::Error:
		return "Error";
	case LogVerbosity::Warning:
		return "Warning";
	default:
	case LogVerbosity::Info:
		return "Info";
	case LogVerbosity::Verbose:
		return "Verbose";
	case LogVerbosity::VeryVerbose:
		return "Very verbose";
	}
}

String ToString(std::time_t val, bool isUTC, bool useISO8601, TimeToStringConversionType type)
{
	char out[100];
	String formatInput;
	if(useISO8601)
	{
		if(type == TimeToStringConversionType::Date)
			formatInput = "%F";
		else if(type == TimeToStringConversionType::Time)
			formatInput = "%T";
		else
			formatInput = "%FT%TZ";
	}
	else
	{
		if(type == TimeToStringConversionType::Date)
			formatInput = "%A, %B %d, %Y";
		else if(type == TimeToStringConversionType::Time)
			formatInput = "%T";
		else
			formatInput = "%A, %B %d, %Y %T";
	}

	if(isUTC)
		std::strftime(out, sizeof(out), formatInput.c_str(), std::gmtime(&val));
	else
		std::strftime(out, sizeof(out), formatInput.c_str(), std::localtime(&val));

	return String(out);
}

float ParseFloat(const String& val, float defaultValue)
{
	// Use istringstream for direct correspondence with toString
	StringStream str(val);
	float ret = defaultValue;
	str >> ret;

	return ret;
}

i32 ParseI32(const String& val, i32 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	StringStream str(val);
	i32 ret = defaultValue;
	str >> ret;

	return ret;
}

u32 ParseU32(const String& val, u32 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	StringStream str(val);
	u32 ret = defaultValue;
	str >> ret;

	return ret;
}

i64 ParseI64(const String& val, i64 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	StringStream str(val);
	i64 ret = defaultValue;
	str >> ret;

	return ret;
}

u64 ParseU64(const String& val, u64 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	StringStream str(val);
	u64 ret = defaultValue;
	str >> ret;

	return ret;
}

bool ParseBool(const String& val, bool defaultValue)
{
	if((StringUtility::StartsWith(val, "true") || StringUtility::StartsWith(val, "yes") || StringUtility::StartsWith(val, "1")))
		return true;
	else if((StringUtility::StartsWith(val, "false") || StringUtility::StartsWith(val, "no") || StringUtility::StartsWith(val, "0")))
		return false;
	else
		return defaultValue;
}

bool IsNumber(const String& val)
{
	StringStream str(val);
	float tst;
	str >> tst;
	return !str.fail() && str.eof();
}

float ParseFloat(const WString& val, float defaultValue)
{
	// Use istringstream for direct correspondence with toString
	WStringStream str(val);
	float ret = defaultValue;
	str >> ret;

	return ret;
}

i32 ParseI32(const WString& val, i32 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	WStringStream str(val);
	i32 ret = defaultValue;
	str >> ret;

	return ret;
}

u32 ParseU32(const WString& val, u32 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	WStringStream str(val);
	u32 ret = defaultValue;
	str >> ret;

	return ret;
}

i64 ParseI64(const WString& val, i64 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	WStringStream str(val);
	i64 ret = defaultValue;
	str >> ret;

	return ret;
}

u64 ParseU64(const WString& val, u64 defaultValue)
{
	// Use istringstream for direct correspondence with toString
	WStringStream str(val);
	u64 ret = defaultValue;
	str >> ret;

	return ret;
}

bool ParseBool(const WString& val, bool defaultValue)
{
	if((StringUtility::StartsWith(val, L"true") || StringUtility::StartsWith(val, L"yes") || StringUtility::StartsWith(val, L"1")))
		return true;
	else if((StringUtility::StartsWith(val, L"false") || StringUtility::StartsWith(val, L"no") || StringUtility::StartsWith(val, L"0")))
		return false;
	else
		return defaultValue;
}

bool IsNumber(const WString& val)
{
	WStringStream str(val);
	float tst;
	str >> tst;
	return !str.fail() && str.eof();
}
} // namespace b3d
