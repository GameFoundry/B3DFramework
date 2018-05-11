//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPlatformDefines.h"
#include "String/BsString.h"
#include "Utility/BsUtil.h"

namespace bs
{
	/** @addtogroup Filesystem
	 *  @{
	 */

	/**
	 * Class for storing and manipulating file paths. Paths may be parsed from and to raw strings according to various
	 * platform specific path types.
	 *
	 * @note
	 * In order to allow the system to easily distinguish between file and directory paths, try to ensure that all directory
	 * paths end with a separator (\ or / depending on platform). System won't fail if you don't but it will be easier to
	 * misuse.
	 */
	class BS_UTILITY_EXPORT Path
	{
	public:
		enum class PathType
		{
			Windows,
			Unix,
			Default
		};

	public:
		Path() = default;

		/**
		 * Constructs a path by parsing the provided path string. Throws exception if provided path is not valid.
		 *
		 * @param[in]	pathStr	String containing the path. Ideally this should be an UTF-8 encoded string in order to
		 *						support non-ANSI characters in the path.
		 * @param[in]	type	If set to default path will be parsed according to the rules of the platform the application
		 *						is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		Path(const String& pathStr, PathType type = PathType::Default);

		/**
		 * Constructs a path by parsing the provided path null terminated string. Throws exception if provided path is
		 * not valid.
		 *
		 * @param[in]	pathStr	Null-terminated string containing the path. Ideally this should be an UTF-8 encoded string
		 *						in order to support non-ANSI characters in the path.
		 * @param[in]	type	If set to default path will be parsed according to the rules of the platform the application
		 *						is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		Path(const char* pathStr, PathType type = PathType::Default);
		Path(const Path& other);

		/**
		 * Assigns a path by parsing the provided path string. Path will be parsed according to the rules of the platform
		 * the application is being compiled to.
		 */
		Path& operator= (const String& pathStr);

		/**
		 * Assigns a path by parsing the provided path null terminated string. Path will be parsed according to the rules
		 * of the platform the application is being compiled to.
		 */
		Path& operator= (const char* pathStr);

		Path& operator= (const Path& path);

		/**
		 * Compares two paths and returns true if they match. Comparison is case insensitive and paths will be compared
		 * as-is, without canonization.
		 */
		bool operator== (const Path& path) const { return equals(path); }

		/**
		 * Compares two paths and returns true if they don't match. Comparison is case insensitive and paths will be
		 * compared as-is, without canonization.
		 */
		bool operator!= (const Path& path) const { return !equals(path); }

		/** Gets a directory name with the specified index from the path. */
		const String& operator[] (UINT32 idx) const { return getDirectory(idx); }

		/** Swap internal data with another Path object. */
		void swap(Path& path);

		/**	Create a path from another Path object. */
		void assign(const Path& path);

		/**
		 * Constructs a path by parsing the provided path string. Throws exception if provided path is not valid.
		 *
		 * @param[in]	pathStr	String containing the path.
		 * @param[in]	type	If set to default path will be parsed according to the rules of the platform the application
		 *						is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		void assign(const String& pathStr, PathType type = PathType::Default);

		/**
		 * Constructs a path by parsing the provided path null terminated string. Throws exception if provided path is not
		 * valid.
		 *
		 * @param[in]	pathStr		Null-terminated string containing the path.
		 * @param[in]	type		If set to default path will be parsed according to the rules of the platform the
		 *							application is being compiled to. Otherwise it will be parsed according to provided
		 *							type.
		 */
		void assign(const char* pathStr, PathType type = PathType::Default);

		/**
		 * Converts the path in a string according to platform path rules.
		 *
		 * @param[in] type	If set to default path will be parsed according to the rules of the platform the application is
		 *					being compiled to. Otherwise it will be parsed according to provided type.
		 * @return			String representing the path using the UTF8 string encoding.
		 */
		String toString(PathType type = PathType::Default) const;

		/**
		 * Converts the path to either a string or a wstring, doing The Right Thing for the current platform.
		 *
		 * This method is equivalent to toWString() on Windows, and to toString() elsewhere.
		 */
#if BS_PLATFORM == BS_PLATFORM_WIN32
		WString toPlatformString() const;
#else
		String toPlatformString() const { return toString(); }
#endif

		/** Checks is the path a directory (contains no file-name). */
		bool isDirectory() const { return mFilename.empty(); }

		/** Checks does the path point to a file. */
		bool isFile() const { return !mFilename.empty(); }

		/** Checks is the contained path absolute. */
		bool isAbsolute() const { return mIsAbsolute; }

		/**
		 * Returns parent path. If current path points to a file the parent path will be the folder where the file is located.
		 * Or if it contains a directory the parent will be the parent directory. If no parent exists, same path will be
		 * returned.
		 */
		Path getParent() const;

		/**
		 * Returns an absolute path by appending the current path to the provided base. If path was already absolute no
		 * changes are made and copy of current path is returned. If base is not absolute, then the returned path will be
		 * made relative to base, but will not be absolute.
		 */
		Path getAbsolute(const Path& base) const;

		/**
		 * Returns a relative path by making the current path relative to the provided base. Base must be a part of the
		 * current path. If base is not a part of the path no changes are made and a copy of the current path is returned.
		 */
		Path getRelative(const Path& base) const;

		/**
		 * Returns the path as a path to directory. If path was pointing to a file, the filename is removed, otherwise no
		 * changes are made and exact copy is returned.
		 */
		Path getDirectory() const;

		/**
		 * Makes the path the parent of the current path. If current path points to a file the parent path will be the
		 * folder where the file is located. Or if it contains a directory the parent will be the parent directory. If no
		 * parent exists, same path will be returned.
		 */
		Path& makeParent();

		/**
		 * Makes the current path absolute by appending it to base. If path was already absolute no changes are made and
		 * copy of current path is returned. If base is not absolute, then the returned path will be made relative to base,
		 * but will not be absolute.
		 */
		Path& makeAbsolute(const Path& base);

		/**
		 * Makes the current path relative to the provided base. Base must be a part of the current path. If base is not
		 * a part of the path no changes are made and a copy of the current path is returned.
		 */
		Path& makeRelative(const Path& base);

		/** Appends another path to the end of this path. */
		Path& append(const Path& path);

		/**
		 * Checks if the current path contains the provided path. Comparison is case insensitive and paths will be compared
		 * as-is, without canonization.
		 */
		bool includes(const Path& child) const;

		/**
		 * Compares two paths and returns true if they match. Comparison is case insensitive and paths will be compared
		 * as-is, without canonization.
		 */
		bool equals(const Path& other) const;

		/** Change or set the filename in the path. */
		void setFilename(const String& filename) { mFilename = filename; }

		/**
		 * Change or set the base name in the path. Base name changes the filename by changing its base to the provided
		 * value but keeping extension intact.
		 */
		void setBasename(const String& basename);

		/**
		 * Change or set the extension of the filename in the path.
		 *
		 * @param[in]	extension	Extension with a leading ".".
		 */
		void setExtension(const String& extension);

		/**
		 * Returns a filename in the path.
		 *
		 * @param[in]	extension	If true, returned filename will contain an extension.
		 */
		String getFilename(bool extension = true) const;

		/** Returns file extension with the leading ".". */
		String getExtension() const;

		/** Gets the number of directories in the path. */
		UINT32 getNumDirectories() const { return (UINT32)mDirectories.size(); }

		/** Gets a directory name with the specified index from the path. */
		const String& getDirectory(UINT32 idx) const;

		/** Returns path device (for example drive, volume, etc.) if one exists in the path. */
		const String& getDevice() const { return mDevice; }

		/** Returns path node (for example network name) if one exists in the path. */
		const String& getNode() const { return mNode; }

		/**
		 * Gets last element in the path, filename if it exists, otherwise the last directory. If no directories exist
		 * returns device or node.
		 */
		const String& getTail() const;

		/** Clears the path to nothing. */
		void clear();

		/** Returns true if no path has been set. */
		bool isEmpty() const { return mDirectories.empty() && mFilename.empty() && mDevice.empty() && mNode.empty(); }

		/** Concatenates two paths. */
		Path operator+ (const Path& rhs) const;

		/** Concatenates two paths. */
		Path& operator+= (const Path& rhs);

		/** Compares two path elements (filenames, directory names, etc.). */
		static bool comparePathElem(const String& left, const String& right);

		/** Combines two paths and returns the result. Right path should be relative. */
		static Path combine(const Path& left, const Path& right);

		/** Strips invalid characters from the provided string and replaces them with empty spaces. */
		static void stripInvalid(String& path);

		static const Path BLANK;
	private:
		/**
		 * Constructs a path by parsing the provided raw string data. Throws exception if provided path is not valid.
		 *
		 * @param[in]	pathStr		String containing the path.
		 * @param[in]	numChars	Number of character in the provided path string.
		 * @param[in]	type		If set to default path will be parsed according to the rules of the platform the
		 *							application is being compiled to. Otherwise it will be parsed according to provided
		 *							type.
		 */
		void assign(const char* pathStr, UINT32 numChars, PathType type = PathType::Default);

		/** Parses a Windows path and stores the parsed data internally. Throws an exception if parsing fails. */
		template<class T>
		void parseWindows(const T* pathStr, UINT32 numChars)
		{
			clear();

			UINT32 idx = 0;
			BasicStringStream<T> tempStream;

			if (idx < numChars)
			{
				if (pathStr[idx] == '\\' || pathStr[idx] == '/')
				{
					mIsAbsolute = true;
					idx++;
				}
			}

			if (idx < numChars)
			{
				// Path starts with a node, a drive letter or is relative
				if (mIsAbsolute && (pathStr[idx] == '\\' || pathStr[idx] == '/')) // Node
				{
					idx++;

					tempStream.str(BasicString<T>());
					tempStream.clear();
					while (idx < numChars && pathStr[idx] != '\\' && pathStr[idx] != '/')
						tempStream << pathStr[idx++];

					setNode(tempStream.str());

					if (idx < numChars)
						idx++;
				}
				else // A drive letter or not absolute
				{
					T drive = pathStr[idx];
					idx++;

					if (idx < numChars && pathStr[idx] == ':')
					{
						if (mIsAbsolute || !((drive >= 'a' && drive <= 'z') || (drive >= 'A' && drive <= 'Z')))
							throwInvalidPathException(BasicString<T>(pathStr, numChars));

						mIsAbsolute = true;
						setDevice(String(1, drive));

						idx++;

						if (idx >= numChars || (pathStr[idx] != '\\' && pathStr[idx] != '/'))
							throwInvalidPathException(BasicString<T>(pathStr, numChars));

						idx++;
					}
					else
						idx--;
				}

				while (idx < numChars)
				{
					tempStream.str(BasicString<T>());
					tempStream.clear();
					while (idx < numChars && pathStr[idx] != '\\' && pathStr[idx] != '/')
					{
						tempStream << pathStr[idx];
						idx++;
					}

					if (idx < numChars)
						pushDirectory(tempStream.str());
					else
						setFilename(tempStream.str());

					idx++;
				}
			}
		}

		/** Parses a Unix path and stores the parsed data internally. Throws an exception if parsing fails. */
		template<class T>
		void parseUnix(const T* pathStr, UINT32 numChars)
		{
			clear();

			UINT32 idx = 0;
			BasicStringStream<T> tempStream;

			if (idx < numChars)
			{
				if (pathStr[idx] == '/')
				{
					mIsAbsolute = true;
					idx++;
				}
				else if (pathStr[idx] == '~')
				{
					idx++;
					if (idx >= numChars || pathStr[idx] == '/')
					{
						pushDirectory(String("~"));
						mIsAbsolute = true;
					}
					else
						idx--;
				}

				while (idx < numChars)
				{
					tempStream.str(BasicString<T>());
					tempStream.clear();
					while (idx < numChars && pathStr[idx] != '/')
					{
						tempStream << pathStr[idx];
						idx++;
					}

					if (idx < numChars)
					{
						if (mDirectories.empty())
						{
							BasicString<T> deviceStr = tempStream.str();
							if (!deviceStr.empty() && *(deviceStr.rbegin()) == ':')
							{
								setDevice(deviceStr.substr(0, deviceStr.length() - 1));
								mIsAbsolute = true;
							}
							else
							{
								pushDirectory(deviceStr);
							}
						}
						else
						{
							pushDirectory(tempStream.str());
						}
					}
					else
					{
						setFilename(tempStream.str());
					}

					idx++;
				}
			}
		}

		void setNode(const String& node) { mNode = node; }
		void setDevice(const String& device) { mDevice = device; }

		/** Build a Windows path string from internal path data. */
		String buildWindows() const;

		/** Build a Unix path string from internal path data. */
		String buildUnix() const;

		/** Add new directory to the end of the path. */
		void pushDirectory(const String& dir);

		/** Helper method that throws invalid path exception. */
		void throwInvalidPathException(const String& path) const;
	private:
		friend struct RTTIPlainType<Path>; // For serialization
		friend struct ::std::hash<bs::Path>;

		Vector<String> mDirectories;
		String mDevice;
		String mFilename;
		String mNode;
		bool mIsAbsolute = false;
	};

	/** @cond SPECIALIZATIONS */

	/**
	 * RTTIPlainType specialization for Path that allows paths be serialized as value types.
	 *
	 * @see		RTTIPlainType
	 */
	template<> struct RTTIPlainType<Path>
	{
		enum { id = TID_Path }; enum { hasDynamicSize = 1 };

		static void toMemory(const Path& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);
			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.mDevice, memory);
			memory = rttiWriteElem(data.mNode, memory);
			memory = rttiWriteElem(data.mFilename, memory);
			memory = rttiWriteElem(data.mIsAbsolute, memory);
			rttiWriteElem(data.mDirectories, memory);
		}

		static UINT32 fromMemory(Path& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.mDevice, memory);
			memory = rttiReadElem(data.mNode, memory);
			memory = rttiReadElem(data.mFilename, memory);
			memory = rttiReadElem(data.mIsAbsolute, memory);
			rttiReadElem(data.mDirectories, memory);

			return size;
		}

		static UINT32 getDynamicSize(const Path& data)
		{
			UINT64 dataSize = rttiGetElemSize(data.mDevice) + rttiGetElemSize(data.mNode) + rttiGetElemSize(data.mFilename) +
				rttiGetElemSize(data.mIsAbsolute) + rttiGetElemSize(data.mDirectories) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}
	};

	/** @endcond */
	/** @} */
}

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for Path. */
	template<>
	struct hash<bs::Path>
	{
		size_t operator()(const bs::Path& path) const
		{
			size_t hash = 0;
			bs::hash_combine(hash, path.mFilename);
			bs::hash_combine(hash, path.mDevice);
			bs::hash_combine(hash, path.mNode);

			for (auto& dir : path.mDirectories)
				bs::hash_combine(hash, dir);

			return hash;
		}
	};
}

/** @endcond */
