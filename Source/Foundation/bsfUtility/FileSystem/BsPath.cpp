//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include <Debug/BsDebug.h>
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Error/BsException.h"
#include "String/BsUnicode.h"

using namespace bs;

const Path Path::kBlank = Path();

Path::Path(const String& pathStr, PathType type)
{
	Assign(pathStr, type);
}

Path::Path(const char* pathStr, PathType type)
{
	Assign(pathStr);
}

Path::Path(const Path& other)
{
	Assign(other);
}

Path& Path::operator=(const Path& path)
{
	Assign(path);
	return *this;
}

Path& Path::operator=(const String& pathStr)
{
	Assign(pathStr);
	return *this;
}

Path& Path::operator=(const char* pathStr)
{
	Assign(pathStr);
	return *this;
}

void Path::Swap(Path& path)
{
	std::swap(mDirectories, path.mDirectories);
	std::swap(mFilename, path.mFilename);
	std::swap(mDevice, path.mDevice);
	std::swap(mNode, path.mNode);
	std::swap(mIsAbsolute, path.mIsAbsolute);
}

void Path::Assign(const Path& path)
{
	mDirectories = path.mDirectories;
	mFilename = path.mFilename;
	mDevice = path.mDevice;
	mNode = path.mNode;
	mIsAbsolute = path.mIsAbsolute;
}

void Path::Assign(const String& pathStr, PathType type)
{
	Assign(pathStr.data(), (u32)pathStr.length(), type);
}

void Path::Assign(const char* pathStr, PathType type)
{
	Assign(pathStr, (u32)strlen(pathStr), type);
}

void Path::Assign(const char* pathStr, u32 numChars, PathType type)
{
	switch(type)
	{
	case PathType::Windows:
		ParseWindows(pathStr, numChars);
		break;
	case PathType::Unix:
		ParseUnix(pathStr, numChars);
		break;
	default:
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
		ParseWindows(pathStr, numChars);
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS || B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
		parseUnix(pathStr, numChars);
#else
		static_assert(false, "Unsupported platform for path.");
#endif
		break;
	}
}

#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
WString Path::ToPlatformString() const
{
	return UTF8::ToWide(ToString());
}
#endif

String Path::ToString(PathType type) const
{
	switch(type)
	{
	case PathType::Windows:
		return BuildWindows();
	case PathType::Unix:
		return BuildUnix();
	default:
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
		return BuildWindows();
#elif B3D_PLATFORM == B3D_PLATFORM_ID_MACOS || B3D_PLATFORM == B3D_PLATFORM_ID_LINUX
		return buildUnix();
#else
		static_assert(false, "Unsupported platform for path.");
#endif
		break;
	}
}

Path Path::GetParent() const
{
	Path copy = *this;
	copy.MakeParent();

	return copy;
}

Path Path::GetAbsolute(const Path& base) const
{
	Path copy = *this;
	copy.MakeAbsolute(base);

	return copy;
}

Path Path::GetRelative(const Path& base) const
{
	Path copy = *this;
	copy.MakeRelative(base);

	return copy;
}

Path Path::GetDirectory() const
{
	Path copy = *this;
	copy.mFilename.clear();

	return copy;
}

Path& Path::MakeParent()
{
	if(mFilename.empty())
	{
		if(mDirectories.empty())
		{
			if(!mIsAbsolute)
				mDirectories.push_back("..");
		}
		else
		{
			if(mDirectories.back() == "..")
				mDirectories.push_back("..");
			else
				mDirectories.pop_back();
		}
	}
	else
	{
		mFilename.clear();
	}

	return *this;
}

Path& Path::MakeAbsolute(const Path& base)
{
	if(mIsAbsolute)
		return *this;

	Path absDir = base.GetDirectory();
	if(base.IsFile())
		absDir.PushDirectory(base.mFilename);

	for(auto& dir : mDirectories)
		absDir.PushDirectory(dir);

	absDir.SetFilename(mFilename);
	*this = absDir;

	return *this;
}

Path& Path::MakeRelative(const Path& base)
{
	if(!base.Includes(*this))
		return *this;

	mDirectories.erase(mDirectories.begin(), mDirectories.begin() + base.mDirectories.size());

	// Sometimes a directory name can be interpreted as a file and we're okay with that. Check for that
	// special case.
	if(base.IsFile())
	{
		if(mDirectories.size() > 0)
			mDirectories.erase(mDirectories.begin());
		else
			mFilename = "";
	}

	mDevice = "";
	mNode = "";
	mIsAbsolute = false;

	return *this;
}

bool Path::Includes(const Path& child) const
{
	if(mDevice != child.mDevice)
		return false;

	if(mNode != child.mNode)
		return false;

	auto iterParent = mDirectories.begin();
	auto iterChild = child.mDirectories.begin();

	for(; iterParent != mDirectories.end(); ++iterChild, ++iterParent)
	{
		if(iterChild == child.mDirectories.end())
			return false;

		if(!ComparePathElem(*iterChild, *iterParent))
			return false;
	}

	if(!mFilename.empty())
	{
		if(iterChild == child.mDirectories.end())
		{
			if(child.mFilename.empty())
				return false;

			if(!ComparePathElem(child.mFilename, mFilename))
				return false;
		}
		else
		{
			if(!ComparePathElem(*iterChild, mFilename))
				return false;
		}
	}

	return true;
}

bool Path::Equals(const Path& other) const
{
	if(mIsAbsolute != other.mIsAbsolute)
		return false;

	if(mIsAbsolute)
	{
		if(!ComparePathElem(mDevice, other.mDevice))
			return false;
	}

	if(!ComparePathElem(mNode, other.mNode))
		return false;

	u32 myNumElements = (u32)mDirectories.size();
	u32 otherNumElements = (u32)other.mDirectories.size();

	if(!mFilename.empty())
		myNumElements++;

	if(!other.mFilename.empty())
		otherNumElements++;

	if(myNumElements != otherNumElements)
		return false;

	if(myNumElements > 0)
	{
		auto iterMe = mDirectories.begin();
		auto iterOther = other.mDirectories.begin();

		for(u32 i = 0; i < (myNumElements - 1); i++, ++iterMe, ++iterOther)
		{
			if(!ComparePathElem(*iterMe, *iterOther))
				return false;
		}

		if(!mFilename.empty())
		{
			if(!other.mFilename.empty())
			{
				if(!ComparePathElem(mFilename, other.mFilename))
					return false;
			}
			else
			{
				if(!ComparePathElem(mFilename, *iterOther))
					return false;
			}
		}
		else
		{
			if(!other.mFilename.empty())
			{
				if(!ComparePathElem(*iterMe, other.mFilename))
					return false;
			}
			else
			{
				if(!ComparePathElem(*iterMe, *iterOther))
					return false;
			}
		}
	}

	return true;
}

Path& Path::Append(const Path& path)
{
	if(!mFilename.empty())
		PushDirectory(mFilename);

	for(auto& dir : path.mDirectories)
		PushDirectory(dir);

	mFilename = path.mFilename;

	return *this;
}

void Path::SetBasename(const String& basename)
{
	mFilename = basename + GetExtension();
}

void Path::SetExtension(const String& extension)
{
	StringStream stream;
	stream << GetFilename(false);
	stream << extension;

	mFilename = stream.str();
}

String Path::GetFilename(bool extension) const
{
	if(extension)
		return mFilename;
	else
	{
		String::size_type pos = mFilename.rfind('.');
		if(pos != String::npos)
			return mFilename.substr(0, pos);
		else
			return mFilename;
	}
}

String Path::GetExtension() const
{
	String::size_type pos = mFilename.rfind('.');
	if(pos != String::npos)
		return mFilename.substr(pos);
	else
		return String();
}

const String& Path::GetDirectory(u32 idx) const
{
	if(idx >= (u32)mDirectories.size())
	{
		B3D_EXCEPT(InvalidParametersException, "Index out of range: " + bs::ToString(idx) + ". Valid range: [0, " + bs::ToString((u32)mDirectories.size() - 1) + "]");
	}

	return mDirectories[idx];
}

const String& Path::GetTail() const
{
	if(IsFile())
		return mFilename;
	else if(mDirectories.size() > 0)
		return mDirectories.back();
	else
		return StringUtil::kBlank;
}

void Path::Clear()
{
	mDirectories.clear();
	mDevice.clear();
	mFilename.clear();
	mNode.clear();
	mIsAbsolute = false;
}

void Path::ThrowInvalidPathException(const String& path) const
{
	B3D_EXCEPT(InvalidParametersException, "Incorrectly formatted path provided: " + path);
}

String Path::BuildWindows() const
{
	StringStream result;
	if(!mNode.empty())
	{
		result << "\\\\";
		result << mNode;
		result << "\\";
	}
	else if(!mDevice.empty())
	{
		result << mDevice;
		result << ":\\";
	}
	else if(mIsAbsolute)
	{
		result << "\\";
	}

	for(auto& dir : mDirectories)
	{
		result << dir;
		result << "\\";
	}

	result << mFilename;
	return result.str();
}

String Path::BuildUnix() const
{
	StringStream result;
	auto dirIter = mDirectories.begin();

	if(!mDevice.empty())
	{
		result << "/";
		result << mDevice;
		result << ":/";
	}
	else if(mIsAbsolute)
	{
		if(dirIter != mDirectories.end() && *dirIter == "~")
		{
			result << "~";
			dirIter++;
		}

		result << "/";
	}

	for(; dirIter != mDirectories.end(); ++dirIter)
	{
		result << *dirIter;
		result << "/";
	}

	result << mFilename;
	return result.str();
}

Path Path::operator+(const Path& rhs) const
{
	return Path::Combine(*this, rhs);
}

Path& Path::operator+=(const Path& rhs)
{
	return Append(rhs);
}

bool Path::ComparePathElem(const String& left, const String& right)
{
	// Note: Might be more efficient to perform toLower character by character, and return as soon as comparison
	// fails. Instead of this way where we're allocating two temporary strings with dynamic memory. Although that
	// approach is problematic as well because UTF8 case conversion requires external library calls which might not
	// support single character conversion, so it might end up being less efficient.
	return UTF8::ToLower(left) == UTF8::ToLower(right);
}

Path Path::Combine(const Path& left, const Path& right)
{
	Path output = left;
	return output.Append(right);
}

void Path::StripInvalid(String& path)
{
	String illegalChars = "\\/:?\"<>|";

	for(auto& entry : path)
	{
		if(illegalChars.find(entry) != String::npos)
			entry = ' ';
	}
}

void Path::PushDirectory(const String& dir)
{
	if(!dir.empty() && dir != ".")
	{
		if(dir == "..")
		{
			if(!mDirectories.empty() && mDirectories.back() != "..")
				mDirectories.pop_back();
			else
				mDirectories.push_back(dir);
		}
		else
			mDirectories.push_back(dir);
	}
}
