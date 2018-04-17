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

	void Path::swap(Path& path)
	{
		std::swap(mDirectories, path.mDirectories);
		std::swap(mFilename, path.mFilename);
		std::swap(mDevice, path.mDevice);
		std::swap(mNode, path.mNode);
		std::swap(mIsAbsolute, path.mIsAbsolute);
	}

	void Path::assign(const Path& path)
	{
		mDirectories = path.mDirectories;
		mFilename = path.mFilename;
		mDevice = path.mDevice;
		mNode = path.mNode;
		mIsAbsolute = path.mIsAbsolute;
	}

	void Path::assign(const String& pathStr, PathType type)
	{
		assign(pathStr.data(), (UINT32)pathStr.length(), type);
	}

	void Path::assign(const char* pathStr, PathType type)
	{
		assign(pathStr, (UINT32)strlen(pathStr), type);
	}

	void Path::assign(const char* pathStr, UINT32 numChars, PathType type)
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
	WString Path::toPlatformString() const
	{
		return UTF8::toWide(toString());
	}
#endif

	String Path::toString(PathType type) const
	{
		switch (type)
		{
		case PathType::Windows:
			return buildWindows();
		case PathType::Unix:
			return buildUnix();
		default:
#if BS_PLATFORM == BS_PLATFORM_WIN32
			return buildWindows();
#elif BS_PLATFORM == BS_PLATFORM_OSX || BS_PLATFORM == BS_PLATFORM_LINUX
			return buildUnix();
#else
			static_assert(false, "Unsupported platform for path.");
#endif
			break;
		}
	}

	Path Path::getParent() const
	{
		Path copy = *this;
		copy.makeParent();

		return copy;
	}

	Path Path::getAbsolute(const Path& base) const
	{
		Path copy = *this;
		copy.makeAbsolute(base);

		return copy;
	}

	Path Path::getRelative(const Path& base) const
	{
		Path copy = *this;
		copy.makeRelative(base);

		return copy;
	}

	Path Path::getDirectory() const
	{
		Path copy = *this;
		copy.mFilename.clear();

		return copy;
	}

	Path& Path::makeParent()
	{
		if (mFilename.empty())
		{
			if (mDirectories.empty())
			{
				if (!mIsAbsolute)
					mDirectories.push_back("..");
			}
			else
			{
				if (mDirectories.back() == "..")
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

	Path& Path::makeAbsolute(const Path& base)
	{
		if (mIsAbsolute)
			return *this;

		Path absDir = base.getDirectory();
		if (base.isFile())
			absDir.pushDirectory(base.mFilename);

		for (auto& dir : mDirectories)
			absDir.pushDirectory(dir);

		absDir.setFilename(mFilename);
		*this = absDir;

		return *this;
	}

	Path& Path::makeRelative(const Path& base)
	{
		if (!base.includes(*this))
			return *this;

		mDirectories.erase(mDirectories.begin(), mDirectories.begin() + base.mDirectories.size());

		// Sometimes a directory name can be interpreted as a file and we're okay with that. Check for that
		// special case.
		if (base.isFile())
		{
			if (mDirectories.size() > 0)
				mDirectories.erase(mDirectories.begin());
			else
				mFilename = "";
		}

		mDevice = "";
		mNode = "";
		mIsAbsolute = false;

		return *this;
	}

	bool Path::includes(const Path& child) const
	{
		if (mDevice != child.mDevice)
			return false;

		if (mNode != child.mNode)
			return false;

		auto iterParent = mDirectories.begin();
		auto iterChild = child.mDirectories.begin();

		for (; iterParent != mDirectories.end(); ++iterChild, ++iterParent)
		{
			if (iterChild == child.mDirectories.end())
				return false;

			if (!comparePathElem(*iterChild, *iterParent))
				return false;
		}

		if (!mFilename.empty())
		{
			if (iterChild == child.mDirectories.end())
			{
				if (child.mFilename.empty())
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

	bool Path::equals(const Path& other) const
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

		UINT32 myNumElements = (UINT32)mDirectories.size();
		UINT32 otherNumElements = (UINT32)other.mDirectories.size();

		if (!mFilename.empty())
			myNumElements++;

		if (!other.mFilename.empty())
			otherNumElements++;

		if (myNumElements != otherNumElements)
			return false;

		if(myNumElements > 0)
		{
			auto iterMe = mDirectories.begin();
			auto iterOther = other.mDirectories.begin();

			for(UINT32 i = 0; i < (myNumElements - 1); i++, ++iterMe, ++iterOther)
			{
				if (!comparePathElem(*iterMe, *iterOther))
					return false;
			}

			if (!mFilename.empty())
			{
				if (!other.mFilename.empty())
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
				if (!other.mFilename.empty())
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

	Path& Path::append(const Path& path)
	{
		if (!mFilename.empty())
			pushDirectory(mFilename);

		for (auto& dir : path.mDirectories)
			pushDirectory(dir);

		mFilename = path.mFilename;

		return *this;
	}

	void Path::setBasename(const String& basename)
	{
		mFilename = basename + getExtension();
	}

	void Path::setExtension(const String& extension)
	{
		StringStream stream;
		stream << getFilename(false);
		stream << extension;

		mFilename = stream.str();
	}

	String Path::getFilename(bool extension) const
	{
		if (extension)
			return mFilename;
		else
		{
			WString::size_type pos = mFilename.rfind(L'.');
			if (pos != WString::npos)
				return mFilename.substr(0, pos);
			else
				return mFilename;
		}
	}

	String Path::getExtension() const
	{
		WString::size_type pos = mFilename.rfind(L'.');
		if (pos != WString::npos)
			return mFilename.substr(pos);
		else
			return String();
	}

	const String& Path::getDirectory(UINT32 idx) const
	{
		if (idx >= (UINT32)mDirectories.size())
		{
			BS_EXCEPT(InvalidParametersException, "Index out of range: " + bs::toString(idx) + ". Valid range: [0, " +
					bs::toString((UINT32)mDirectories.size() - 1) + "]");
		}

		return mDirectories[idx];
	}

	const String& Path::getTail() const
	{
		if (isFile())
			return mFilename;
		else if (mDirectories.size() > 0)
			return mDirectories.back();
		else
			return StringUtil::BLANK;
	}

	void Path::clear()
	{
		mDirectories.clear();
		mDevice.clear();
		mFilename.clear();
		mNode.clear();
		mIsAbsolute = false;
	}

	void Path::throwInvalidPathException(const String& path) const
	{
		BS_EXCEPT(InvalidParametersException, "Incorrectly formatted path provided: " + path);
	}

	String Path::buildWindows() const
	{
		StringStream result;
		if (!mNode.empty())
		{
			result << "\\\\";
			result << mNode;
			result << "\\";
		}
		else if (!mDevice.empty())
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
		return result.str();
	}

	String Path::buildUnix() const
	{
		StringStream result;
		auto dirIter = mDirectories.begin();

		if (!mDevice.empty())
		{
			result << "/";
			result << mDevice;
			result << ":/";
		}
		else if (mIsAbsolute)
		{
			if (dirIter != mDirectories.end() && *dirIter == "~")
			{
				result << "~";
				dirIter++;
			}

			result << "/";
		}

		for (; dirIter != mDirectories.end(); ++dirIter)
		{
			result << *dirIter;
			result << "/";
		}

		result << mFilename;
		return result.str();
	}

	Path Path::operator+ (const Path& rhs) const
	{
		return Path::combine(*this, rhs);
	}

	Path& Path::operator+= (const Path& rhs)
	{
		return append(rhs);
	}

	bool Path::comparePathElem(const String& left, const String& right)
	{
		if (left.size() != right.size())
			return false;

		// TODO: Case sensitive/insensitive file path actually depends on used file-system but I'm not gonna check that
		for (UINT32 i = 0; i < (UINT32)left.size(); i++)
		{
			if (tolower(left[i]) != tolower(right[i]))
				return false;
		}

		return true;
	}

	Path Path::combine(const Path& left, const Path& right)
	{
		Path output = left;
		return output.append(right);
	}

	void Path::stripInvalid(String& path)
	{
		String illegalChars = "\\/:?\"<>|";

		for(auto& entry : path)
		{
			if(illegalChars.find(entry) != String::npos)
				entry = ' ';
		}
	}

	void Path::pushDirectory(const String& dir)
	{
		if (!dir.empty() && dir != ".")
		{
			if (dir == "..")
			{
				if (!mDirectories.empty() && mDirectories.back() != "..")
					mDirectories.pop_back();
				else
					mDirectories.push_back(dir);
			}
			else
				mDirectories.push_back(dir);
		}
	}
}
