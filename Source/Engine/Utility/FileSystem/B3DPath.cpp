//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Debug/B3DDebug.h"
#include "B3DUtilityPrerequisites.h"
#include "String/B3DUnicode.h"

using namespace b3d;

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

void Path::Assign(const char* pathStr, u32 characterCount, PathType type)
{
	switch(type)
	{
	case PathType::Windows:
		ParseWindows(pathStr, characterCount);
		break;
	case PathType::Unix:
		ParseUnix(pathStr, characterCount);
		break;
	default:
#if B3D_PLATFORM_WIN32
		ParseWindows(pathStr, characterCount);
#elif B3D_PLATFORM_MACOS || B3D_PLATFORM_LINUX
		parseUnix(pathStr, characterCount);
#else
		static_assert(false, "Unsupported platform for path.");
#endif
		break;
	}
}

#if B3D_PLATFORM_WIN32
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
#if B3D_PLATFORM_WIN32
		return BuildWindows();
#elif B3D_PLATFORM_MACOS || B3D_PLATFORM_LINUX
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

bool Path::Includes(const Path& child, bool caseSensitive) const
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

		if(!ComparePathElem(*iterChild, *iterParent, caseSensitive))
			return false;
	}

	if(!mFilename.empty())
	{
		if(iterChild == child.mDirectories.end())
		{
			if(child.mFilename.empty())
				return false;

			if(!ComparePathElem(child.mFilename, mFilename, caseSensitive))
				return false;
		}
		else
		{
			if(!ComparePathElem(*iterChild, mFilename, caseSensitive))
				return false;
		}
	}

	return true;
}

bool Path::Equals(const Path& other, bool caseSensitive) const
{
	if(mIsAbsolute != other.mIsAbsolute)
		return false;

	if(mIsAbsolute)
	{
		if(!ComparePathElem(mDevice, other.mDevice, caseSensitive))
			return false;
	}

	if(!ComparePathElem(mNode, other.mNode, caseSensitive))
		return false;

	u32 myElementCount = (u32)mDirectories.size();
	u32 otherElementCount = (u32)other.mDirectories.size();

	if(!mFilename.empty())
		myElementCount++;

	if(!other.mFilename.empty())
		otherElementCount++;

	if(myElementCount != otherElementCount)
		return false;

	if(myElementCount > 0)
	{
		auto myDirectoryIterator = mDirectories.begin();
		auto otherDirectoryIterator = other.mDirectories.begin();

		for(u32 i = 0; i < (myElementCount - 1); i++, ++myDirectoryIterator, ++otherDirectoryIterator)
		{
			if(!ComparePathElem(*myDirectoryIterator, *otherDirectoryIterator, caseSensitive))
				return false;
		}

		if(!mFilename.empty())
		{
			if(!other.mFilename.empty())
			{
				if(!ComparePathElem(mFilename, other.mFilename, caseSensitive))
					return false;
			}
			else
			{
				if(!ComparePathElem(mFilename, *otherDirectoryIterator, caseSensitive))
					return false;
			}
		}
		else
		{
			if(!other.mFilename.empty())
			{
				if(!ComparePathElem(*myDirectoryIterator, other.mFilename, caseSensitive))
					return false;
			}
			else
			{
				if(!ComparePathElem(*myDirectoryIterator, *otherDirectoryIterator, caseSensitive))
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

const String& Path::GetDirectory(u32 index) const
{
	if(!B3D_ENSURE_LOG(index < (u32)mDirectories.size(), "Index out of range: {0}. Valid range: [0, {1}]", ::b3d::ToString(index), ::b3d::ToString((u32)mDirectories.size() - 1)))
		return StringUtility::kBlank;

	return mDirectories[index];
}

const String& Path::GetTail() const
{
	if(IsFile())
		return mFilename;
	else if(mDirectories.size() > 0)
		return mDirectories.back();
	else
		return StringUtility::kBlank;
}

String Path::PopTail()
{
	if(IsFile())
	{
		String filename = mFilename;
		mFilename = StringUtility::kBlank;
		return filename;
	}
	else if(!mDirectories.empty())
	{
		String directory = mDirectories.back();
		mDirectories.pop_back();
		return directory;
	}
	else
		return StringUtility::kBlank;
}

Path Path::GetSubPath(u32 directoryCount) const
{
	Path output;
	output.mDevice = mDevice;
	output.mNode = mNode;
	output.mIsAbsolute = mIsAbsolute;

	directoryCount = std::min(directoryCount, (u32)mDirectories.size());
	output.mDirectories.reserve(directoryCount);

	for(u32 directoryIndex = 0; directoryIndex < directoryCount; directoryIndex++)
		output.mDirectories.push_back(mDirectories[directoryIndex]);

	return output;
}

void Path::Clear()
{
	mDirectories.clear();
	mDevice.clear();
	mFilename.clear();
	mNode.clear();
	mIsAbsolute = false;
}

void Path::ReportInvalidPath(const String& path) const
{
	B3D_LOG(Fatal, LogFileSystem, "Incorrectly formatted path provided: {0}", path);
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

bool Path::ComparePathElem(const String& left, const String& right, bool caseSensitive)
{
	if(caseSensitive)
		return left == right;

	// Note: Might be more efficient to perform toLower character by character, and return as soon as comparison
	// fails. Instead of this way where we're allocating two temporary strings with dynamic memory. Although that
	// approach is problematic as well because UTF8 case conversion requires external library calls which might not
	// support single character conversion, so it might end up being less efficient.
	return UTF8::ToLower(left) == UTF8::ToLower(right);
}

Path Path::Combine(const Path& left, const Path& right)
{
	if(right.IsEmpty())
		return left;

	Path output = left;
	return output.Append(right);
}

Path Path::Combine(const Path& left, const Path& middle, const Path& right)
{
	Path output = left;
	output.Append(middle);
	output.Append(right);

	return output;
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

void Path::PopDirectory()
{
	if(mDirectories.empty())
		return;

	mDirectories.pop_back();
}

size_t PathHashFunction<true>::operator()(const Path& path) const
{
	size_t hash = 0;
	b3d::B3DCombineHash(hash, path.mFilename);
	b3d::B3DCombineHash(hash, path.mDevice);
	b3d::B3DCombineHash(hash, path.mNode);

	for(auto& directory : path.mDirectories)
		b3d::B3DCombineHash(hash, directory);

	return hash;
}

/** Calculates path hash from a case insensitive path. */
size_t PathHashFunction<false>::operator()(const Path& path) const
{
	size_t hash = 0;
	b3d::B3DCombineHash(hash, UTF8::ToLower(path.mFilename));
	b3d::B3DCombineHash(hash, UTF8::ToLower(path.mDevice));
	b3d::B3DCombineHash(hash, UTF8::ToLower(path.mNode));

	for(auto& directory : path.mDirectories)
		b3d::B3DCombineHash(hash, UTF8::ToLower(directory));

	return hash;
}
