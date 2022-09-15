//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsColor.h"
#include "Math/BsMath.h"
#include "Math/BsMatrix3.h"
#include "Math/BsMatrix4.h"
#include "Math/BsQuaternion.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsVector2I.h"
#include "Error/BsException.h"
#include "Debug/BsLog.h"

namespace bs
{
	const String StringUtil::BLANK;
	const WString StringUtil::WBLANK;

	void StringUtil::Trim(String& str, bool left, bool right)
	{
		static const String delims = " \t\r";
		Trim(str, delims, left, right);
	}

	void StringUtil::Trim(WString& str, bool left, bool right)
	{
		static const WString delims = L" \t\r";
		Trim(str, delims, left, right);
	}

	void StringUtil::Trim(String& str, const String& delims, bool left, bool right)
	{
		if(right)
			str.erase(str.find_last_not_of(delims)+1); // trim right
		if(left)
			str.erase(0, str.find_first_not_of(delims)); // trim left
	}

	void StringUtil::Trim(WString& str, const WString& delims, bool left, bool right)
	{
		if(right)
			str.erase(str.find_last_not_of(delims)+1); // trim right
		if(left)
			str.erase(0, str.find_first_not_of(delims)); // trim left
	}

	Vector<String> StringUtil::Split(const String& str, const String& delims, unsigned int maxSplits)
	{
		return SplitInternal<char>(str, delims, maxSplits);
	}

	Vector<WString> StringUtil::Split(const WString& str, const WString& delims, unsigned int maxSplits)
	{
		return SplitInternal<wchar_t>(str, delims, maxSplits);
	}

	Vector<String> StringUtil::Tokenise(const String& str, const String& singleDelims, const String& doubleDelims, unsigned int maxSplits)
	{
		return TokeniseInternal<char>(str, singleDelims, doubleDelims, maxSplits);
	}

	Vector<WString> StringUtil::Tokenise(const WString& str, const WString& singleDelims, const WString& doubleDelims, unsigned int maxSplits)
	{
		return TokeniseInternal<wchar_t>(str, singleDelims, doubleDelims, maxSplits);
	}

	void StringUtil::ToLowerCase(String& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
	}

	void StringUtil::ToLowerCase(WString& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
	}

	void StringUtil::ToUpperCase(String& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), toupper);
	}

	void StringUtil::ToUpperCase(WString& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), toupper);
	}

	bool StringUtil::StartsWith(const String& str, const String& pattern, bool lowerCase)
	{
		return StartsWithInternal<char>(str, pattern, lowerCase);
	}

	bool StringUtil::StartsWith(const WString& str, const WString& pattern, bool lowerCase)
	{
		return StartsWithInternal<wchar_t>(str, pattern, lowerCase);
	}

	bool StringUtil::EndsWith(const String& str, const String& pattern, bool lowerCase)
	{
		return EndsWithInternal<char>(str, pattern, lowerCase);
	}

	bool StringUtil::EndsWith(const WString& str, const WString& pattern, bool lowerCase)
	{
		return EndsWithInternal<wchar_t>(str, pattern, lowerCase);
	}

	bool StringUtil::Match(const String& str, const String& pattern, bool caseSensitive)
	{
		return MatchInternal<char>(str, pattern, caseSensitive);
	}

	bool StringUtil::Match(const WString& str, const WString& pattern, bool caseSensitive)
	{
		return MatchInternal<wchar_t>(str, pattern, caseSensitive);
	}

	const String StringUtil::ReplaceAll(const String& source, const String& replaceWhat, const String& replaceWithWhat)
	{
		return ReplaceAllInternal<char>(source, replaceWhat, replaceWithWhat);
	}

	const WString StringUtil::ReplaceAll(const WString& source, const WString& replaceWhat, const WString& replaceWithWhat)
	{
		return ReplaceAllInternal<wchar_t>(source, replaceWhat, replaceWithWhat);
	}

	/************************************************************************/
	/* 						VARIOUS TO STRING CONVERSIONS                   */
	/************************************************************************/

	WString toWString(const String& source)
	{
		return WString(source.begin(), source.end());
	}

	WString toWString(const char* source)
	{
		return toWString(String(source));
	}

	WString toWString(float val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(double val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(const Radian& val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		return toWString(val.ValueRadians(), precision, width, fill, flags);
	}

	WString toWString(const Degree& val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		return toWString(val.ValueDegrees(), precision, width, fill, flags);
	}

	WString toWString(int val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(unsigned int val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(INT64 val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(UINT64 val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(char val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(wchar_t val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		WStringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	WString toWString(const Vector2& val)
	{
		WStringStream stream;
		stream << val.x << L" " << val.y;
		return stream.str();
	}

	WString toWString(const Vector2I& val)
	{
		WStringStream stream;
		stream << val.x << L" " << val.y;
		return stream.str();
	}

	WString toWString(const Vector3& val)
	{
		WStringStream stream;
		stream << val.x << L" " << val.y << L" " << val.z;
		return stream.str();
	}

	WString toWString(const Vector4& val)
	{
		WStringStream stream;
		stream << val.x << L" " << val.y << L" " << val.z << L" " << val.w;
		return stream.str();
	}

	WString toWString(const Matrix3& val)
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

	WString toWString(bool val, bool yesNo)
	{
		if (val)
		{
			if (yesNo)
			{
				return L"yes";
			}
			else
			{
				return L"true";
			}
		}
		else
			if (yesNo)
			{
				return L"no";
			}
			else
			{
				return L"false";
			}
	}

	WString toWString(const Matrix4& val)
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

	WString toWString(const Quaternion& val)
	{
		WStringStream stream;
		stream  << val.w << L" " << val.x << L" " << val.y << L" " << val.z;
		return stream.str();
	}

	WString toWString(const Color& val)
	{
		WStringStream stream;
		stream << val.r << L" " << val.g << L" " << val.b << L" " << val.a;
		return stream.str();
	}

	WString toWString(const Vector<bs::WString>& val)
	{
		WStringStream stream;
		Vector<bs::WString>::const_iterator i, iend, ibegin;
		ibegin = val.begin();
		iend = val.end();
		for (i = ibegin; i != iend; ++i)
		{
			if (i != ibegin)
				stream << L" ";

			stream << *i;
		}
		return stream.str();
	}

	String toString(const WString& source)
	{
		StringStream stream;
		for (auto& entry : source)
			stream << entry;

		return stream.str();
	}

	String toString(const wchar_t* source)
	{
		return toString(WString(source));
	}

	String toString(float val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		StringStream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	String toString(double val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		StringStream stream;
		stream.precision(precision);
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	String toString(const Radian& val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		return toString(val.ValueRadians(), precision, width, fill, flags);
	}

	String toString(const Degree& val, unsigned short precision,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		return toString(val.ValueDegrees(), precision, width, fill, flags);
	}

	String toString(int val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		StringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	String toString(unsigned int val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		StringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	String toString(INT64 val,
		unsigned short width, char fill, std::ios::fmtflags flags)
	{
		StringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	String toString(UINT64 val, unsigned short width, char fill, std::ios::fmtflags flags)
	{
		StringStream stream;
		stream.width(width);
		stream.fill(fill);
		if (flags)
			stream.setf(flags);
		stream << val;
		return stream.str();
	}

	String toString(const Vector2& val)
	{
		StringStream stream;
		stream << val.x << " " << val.y;
		return stream.str();
	}

	String toString(const Vector2I& val)
	{
		StringStream stream;
		stream << val.x << " " << val.y;
		return stream.str();
	}

	String toString(const Vector3& val)
	{
		StringStream stream;
		stream << val.x << " " << val.y << " " << val.z;
		return stream.str();
	}

	String toString(const Vector4& val)
	{
		StringStream stream;
		stream << val.x << " " << val.y << " " << val.z << " " << val.w;
		return stream.str();
	}

	String toString(const Matrix3& val)
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

	String toString(bool val, bool yesNo)
	{
		if (val)
		{
			if (yesNo)
			{
				return "yes";
			}
			else
			{
				return "true";
			}
		}
		else
			if (yesNo)
			{
				return "no";
			}
			else
			{
				return "false";
			}
	}

	String toString(const Matrix4& val)
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

	String toString(const Quaternion& val)
	{
		StringStream stream;
		stream  << val.w << " " << val.x << " " << val.y << " " << val.z;
		return stream.str();
	}

	String toString(const Color& val)
	{
		StringStream stream;
		stream << val.r << " " << val.g << " " << val.b << " " << val.a;
		return stream.str();
	}

	String toString(const UUID& val)
	{
		return val.ToString();
	}

	String toString(const Path& val)
	{
		return val.ToString();
	}

	String toString(const Vector<bs::String>& val)
	{
		StringStream stream;
		Vector<bs::String>::const_iterator i, iend, ibegin;
		ibegin = val.begin();
		iend = val.end();
		for (i = ibegin; i != iend; ++i)
		{
			if (i != ibegin)
				stream << " ";

			stream << *i;
		}
		return stream.str();
	}

	String toString(const LogVerbosity& val)
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
	
	String toString(std::time_t val, bool isUTC, bool useISO8601, TimeToStringConversionType type)
	{
		char out[100];
		String formatInput;
		if (useISO8601)
		{
			if (type == TimeToStringConversionType::Date)
				formatInput = "%F";
			else if (type == TimeToStringConversionType::Time)
				formatInput = "%T";
			else
				formatInput = "%FT%TZ";
		}
		else
		{
			if (type == TimeToStringConversionType::Date)
				formatInput = "%A, %B %d, %Y";
			else if (type == TimeToStringConversionType::Time)
				formatInput = "%T";
			else
				formatInput = "%A, %B %d, %Y %T";
		}
		
		if (isUTC)
			std::strftime(out, sizeof(out), formatInput.c_str(), std::gmtime(&val));
		else
			std::strftime(out, sizeof(out), formatInput.c_str(), std::localtime(&val));
		
		return String(out);
	}
	
	
	float parseFloat(const String& val, float defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		float ret = defaultValue;
		str >> ret;

		return ret;
	}

	INT32 parseINT32(const String& val, INT32 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		INT32 ret = defaultValue;
		str >> ret;

		return ret;
	}

	UINT32 parseUINT32(const String& val, UINT32 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		UINT32 ret = defaultValue;
		str >> ret;

		return ret;
	}

	INT64 parseINT64(const String& val, INT64 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		INT64 ret = defaultValue;
		str >> ret;

		return ret;
	}

	UINT64 parseUINT64(const String& val, UINT64 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		StringStream str(val);
		UINT64 ret = defaultValue;
		str >> ret;

		return ret;
	}

	bool parseBool(const String& val, bool defaultValue)
	{
		if ((StringUtil::StartsWith(val, "true") || StringUtil::StartsWith(val, "yes")
			|| StringUtil::StartsWith(val, "1")))
			return true;
		else if ((StringUtil::StartsWith(val, "false") || StringUtil::StartsWith(val, "no")
			|| StringUtil::StartsWith(val, "0")))
			return false;
		else
			return defaultValue;
	}

	bool isNumber(const String& val)
	{
		StringStream str(val);
		float tst;
		str >> tst;
		return !str.fail() && str.eof();
	}

	float parseFloat(const WString& val, float defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		WStringStream str(val);
		float ret = defaultValue;
		str >> ret;

		return ret;
	}

	INT32 parseINT32(const WString& val, INT32 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		WStringStream str(val);
		INT32 ret = defaultValue;
		str >> ret;

		return ret;
	}

	UINT32 parseUINT32(const WString& val, UINT32 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		WStringStream str(val);
		UINT32 ret = defaultValue;
		str >> ret;

		return ret;
	}

	INT64 parseINT64(const WString& val, INT64 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		WStringStream str(val);
		INT64 ret = defaultValue;
		str >> ret;

		return ret;
	}

	UINT64 parseUINT64(const WString& val, UINT64 defaultValue)
	{
		// Use istringstream for direct correspondence with toString
		WStringStream str(val);
		UINT64 ret = defaultValue;
		str >> ret;

		return ret;
	}

	bool parseBool(const WString& val, bool defaultValue)
	{
		if ((StringUtil::StartsWith(val, L"true") || StringUtil::StartsWith(val, L"yes")
			|| StringUtil::StartsWith(val, L"1")))
			return true;
		else if ((StringUtil::StartsWith(val, L"false") || StringUtil::StartsWith(val, L"no")
			|| StringUtil::StartsWith(val, L"0")))
			return false;
		else
			return defaultValue;
	}

	bool isNumber(const WString& val)
	{
		WStringStream str(val);
		float tst;
		str >> tst;
		return !str.fail() && str.eof();
	}

	void __string_throwDataOverflowException()
	{
		BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
	}
}
