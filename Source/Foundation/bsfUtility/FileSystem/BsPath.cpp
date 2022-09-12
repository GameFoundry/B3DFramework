//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include <Debug/BsDebug.h>
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Error/BsException.h"
#include "String/BsUnicode.h"

namespace bs
{
	const Path Path::BLANK = Path();

	Path::Path(const String& pathStr, PathType type)
	{
		assign(pathStr, type);
	}

	Path::Path(const char* pathStr, PathType type)
	{
		assign(pathStr);
	}

	Path::Path(const Path& other)
	{
		assign(other);
	}

	Path& Path::operator= (const Path& path)
	{
		assign(path);
		return *this;
	}

	Path& Path::operator= (const String& pathStr)
	{
		assign(pathStr);
		return *this;
	}

	Path& Path::operator= (const char* pathStr)
	{
		assign(pathStr);
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
		assign(pathStr.Data(), (UINT32)pathStr.length(), type);
	}

	void Path::Assign(const char* pathStr, PathType type)
	{
		assign(pathStr, (UINT32)strlen(pathStr), type);
	}

	void Path::Assign(const char* pathStr, UINT32 numChars, PathType type)
	{
		switch (type)
		{
		case PathType::Windows:
			parseWindows(pathStr, numChars);
			break;
		case PathType::Unix:
			parseUnix(pathStr, numChars);
			break;
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			parseWindows(pathStr, numChars);
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			parseUnix(pathStr, numChars);
#else
			static_assert(false, "Unsupported platform for path.");
#endif
			break;
		}
	}

#if BS_PLATFORM == BS_PLATFORM_WIN32
	WString Path::ToPlatformString() const
	{
		return UTF8::ToWide(toString());
	}
#endif

	String Path::ToString(PathType type) const
	{
		switch (type)
		{
		case PathType::Windows:
			return BuildWindows();
		case PathType::Unix:
			return BuildUnix();
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			return BuildWindows();
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			return BuildUnix();
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
		copy.mFilename.Clear();

		return copy;
	}

	Path& Path::MakeParent()
	{
		if (mFilename.Empty())
		{
			if (mDirectories.Empty())
			{
				if (!mIsAbsolute)
					mDirectories.push_back("..");
			}
			else
			{
				if (mDirectories.Back() == "..")
					mDirectories.push_back("..");
				else
					mDirectories.pop_back();
			}
		}
		else
		{
			mFilename.Clear();
		}

		return *this;
	}

	Path& Path::MakeAbsolute(const Path& base)
	{
		if (mIsAbsolute)
			return *this;

		Path absDir = base.GetDirectory();
		if (base.IsFile())
			absDir.PushDirectory(base.mFilename);

		for (auto& dir : mDirectories)
			absDir.PushDirectory(dir);

		absDir.SetFilename(mFilename);
		*this = absDir;

		return *this;
	}

	Path& Path::MakeRelative(const Path& base)
	{
		if (!base.Includes(*this))
			return *this;

		mDirectories.Erase(mDirectories.begin(), mDirectories.begin() + base.mDirectories.size());

		// Sometimes a directory name can be interpreted as a file and we're okay with that. Check for that
		// special case.
		if (base.IsFile())
		{
			if (mDirectories.Size() > 0)
				mDirectories.Erase(mDirectories.begin());
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
		if (mDevice != child.mDevice)
			return false;

		if (mNode != child.mNode)
			return false;

		auto iterParent = mDirectories.Begin();
		auto iterChild = child.mDirectories.Begin();

		for (; iterParent != mDirectories.End(); ++iterChild, ++iterParent)
		{
			if (iterChild == child.mDirectories.End())
				return false;

			if (!comparePathElem(*iterChild, *iterParent))
				return false;
		}

		if (!mFilename.Empty())
		{
			if (iterChild == child.mDirectories.End())
			{
				if (child.mFilename.Empty())
					return false;

				if (!comparePathElem(child.mFilename, mFilename))
					return false;
			}
			else
			{
				if (!comparePathElem(*iterChild, mFilename))
					return false;
			}			
		}

		return true;
	}

	bool Path::Equals(const Path& other) const
	{
		if (mIsAbsolute != other.mIsAbsolute)
			return false;

		if (mIsAbsolute)
		{
			if (!comparePathElem(mDevice, other.mDevice))
				return false;
		}

		if (!comparePathElem(mNode, other.mNode))
			return false;

		UINT32 myNumElements = (UINT32)mDirectories.Size();
		UINT32 otherNumElements = (UINT32)other.mDirectories.Size();

		if (!mFilename.Empty())
			myNumElements++;

		if (!other.mFilename.Empty())
			otherNumElements++;

		if (myNumElements != otherNumElements)
			return false;

		if(myNumElements > 0)
		{
			auto iterMe = mDirectories.Begin();
			auto iterOther = other.mDirectories.Begin();

			for(UINT32 i = 0; i < (myNumElements - 1); i++, ++iterMe, ++iterOther)
			{
				if (!comparePathElem(*iterMe, *iterOther))
					return false;
			}

			if (!mFilename.Empty())
			{
				if (!other.mFilename.Empty())
				{
					if (!comparePathElem(mFilename, other.mFilename))
						return false;
				}
				else
				{
					if (!comparePathElem(mFilename, *iterOther))
						return false;
				}
			}
			else
			{
				if (!other.mFilename.Empty())
				{
					if (!comparePathElem(*iterMe, other.mFilename))
						return false;
				}
				else
				{
					if (!comparePathElem(*iterMe, *iterOther))
						return false;
				}
			}
		}

		return true;
	}

	Path& Path::Append(const Path& path)
	{
		if (!mFilename.Empty())
			pushDirectory(mFilename);

		for (auto& dir : path.mDirectories)
			pushDirectory(dir);

		mFilename = path.mFilename;

		return *this;
	}

	void Path::SetBasename(const String& basename)
	{
		mFilename = basename + getExtension();
	}

	void Path::SetExtension(const String& extension)
	{
		StringStream stream;
		stream << GetFilename(false);
		stream << extension;

		mFilename = stream.Str();
	}

	String Path::GetFilename(bool extension) const
	{
		if (extension)
			return mFilename;
		else
		{
			String::size_type pos = mFilename.Rfind('.');
			if (pos != String::npos)
				return mFilename.Substr(0, pos);
			else
				return mFilename;
		}
	}

	String Path::GetExtension() const
	{
		String::size_type pos = mFilename.Rfind('.');
		if (pos != String::npos)
			return mFilename.Substr(pos);
		else
			return String();
	}

	const String& Path::GetDirectory(UINT32 idx) const
	{
		if (idx >= (UINT32)mDirectories.Size())
		{
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + bs::toString(idx) + ". Valid range: [0, " +
					bs::toString((UINT32)mDirectories.Size() - 1) + "]");
		}

		return mDirectories[idx];
	}

	const String& Path::GetTail() const
	{
		if (isFile())
			return mFilename;
		else if (mDirectories.Size() > 0)
			return mDirectories.Back();
		else
			return StringUtil::BLANK;
	}

	void Path::Clear()
	{
		mDirectories.Clear();
		mDevice.Clear();
		mFilename.Clear();
		mNode.Clear();
		mIsAbsolute = false;
	}

	void Path::ThrowInvalidPathException(const String& path) const
	{
		BS_EXCEPT(InvalidParametersException, "Incorrectly formatted path provided: " + path);
	}

	String Path::BuildWindows() const
	{
		StringStream result;
		if (!mNode.Empty())
		{
			result << "\\\\";
			result << mNode;
			result << "\\";
		}
		else if (!mDevice.Empty())
		{
			result << mDevice;
			result << ":\\";
		}
		else if (mIsAbsolute)
		{
			result << "\\";
		}

		for (auto& dir : mDirectories)
		{
			result << dir;
			result << "\\";
		}

		result << mFilename;
		return result.Str();
	}

	String Path::BuildUnix() const
	{
		StringStream result;
		auto dirIter = mDirectories.Begin();

		if (!mDevice.Empty())
		{
			result << "/";
			result << mDevice;
			result << ":/";
		}
		else if (mIsAbsolute)
		{
			if (dirIter != mDirectories.End() && *dirIter == "~")
			{
				result << "~";
				dirIter++;
			}

			result << "/";
		}

		for (; dirIter != mDirectories.End(); ++dirIter)
		{
			result << *dirIter;
			result << "/";
		}

		result << mFilename;
		return result.Str();
	}

	Path Path::operator+ (const Path& rhs) const
	{
		return Path::Combine(*this, rhs);
	}

	Path& Path::operator+= (const Path& rhs)
	{
		return Append(rhs);
	}

	bool Path::ComparePathElem(const String& left, const String& right)
	{
		// Note: Might be more efficient to perform toLower character by character, and return as soon as comparison
		// fails. Instead of this way where we're allocating two temporary strings with dynamic memory. Although that
		// approach is problematic as well because UTF8 case conversion requires external library calls which might not
		// support single character conversion, so it might end up being less efficient.
		return UTF8::ToLower(left) == UTF8::toLower(right);
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
			if(illegalChars.Find(entry) != String::npos)
				entry = ' ';
		}
	}

	void Path::PushDirectory(const String& dir)
	{
		if (!dir.Empty() && dir != ".")
		{
			if (dir == "..")
			{
				if (!mDirectories.Empty() && mDirectories.back() != "..")
					mDirectories.pop_back();
				else
					mDirectories.push_back(dir);
			}
			else
				mDirectories.push_back(dir);
		}
	}
}
