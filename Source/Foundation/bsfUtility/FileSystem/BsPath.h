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
		void Swap(Path& path);

		/**	Create a path from another Path object. */
		void Assign(const Path& path);

		/**
		 * Constructs a path by parsing the provided path string. Throws exception if provided path is not valid.
		 *
		 * @param[in]	pathStr	String containing the path.
		 * @param[in]	type	If set to default path will be parsed according to the rules of the platform the application
		 *						is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		void Assign(const String& pathStr, PathType type = PathType::Default);

		/**
		 * Constructs a path by parsing the provided path null terminated string. Throws exception if provided path is not
		 * valid.
		 *
		 * @param[in]	pathStr		Null-terminated string containing the path.
		 * @param[in]	type		If set to default path will be parsed according to the rules of the platform the
		 *							application is being compiled to. Otherwise it will be parsed according to provided
		 *							type.
		 */
		void Assign(const char* pathStr, PathType type = PathType::Default);

		/**
		 * Converts the path in a string according to platform path rules.
		 *
		 * @param[in] type	If set to default path will be parsed according to the rules of the platform the application is
		 *					being compiled to. Otherwise it will be parsed according to provided type.
		 * @return			String representing the path using the UTF8 string encoding.
		 */
		String ToString(PathType type = PathType::Default) const;

		/**
		 * Converts the path to either a string or a wstring, doing The Right Thing for the current platform.
		 *
		 * This method is equivalent to toWString() on Windows, and to toString() elsewhere.
		 */
#if BS_PLATFORM == BS_PLATFORM_WIN32
		WString ToPlatformString() const;
#else
		String ToPlatformString() const { return toString(); }
#endif

		/** Checks is the path a directory (contains no file-name). */
		bool IsDirectory() const { return mFilename.Empty(); }

		/** Checks does the path point to a file. */
		bool IsFile() const { return !mFilename.Empty(); }

		/** Checks is the contained path absolute. */
		bool IsAbsolute() const { return mIsAbsolute; }

		/**
		 * Returns parent path. If current path points to a file the parent path will be the folder where the file is located.
		 * Or if it contains a directory the parent will be the parent directory. If no parent exists, same path will be
		 * returned.
		 */
		Path GetParent() const;

		/**
		 * Returns an absolute path by appending the current path to the provided base. If path was already absolute no
		 * changes are made and copy of current path is returned. If base is not absolute, then the returned path will be
		 * made relative to base, but will not be absolute.
		 */
		Path GetAbsolute(const Path& base) const;

		/**
		 * Returns a relative path by making the current path relative to the provided base. Base must be a part of the
		 * current path. If base is not a part of the path no changes are made and a copy of the current path is returned.
		 */
		Path GetRelative(const Path& base) const;

		/**
		 * Returns the path as a path to directory. If path was pointing to a file, the filename is removed, otherwise no
		 * changes are made and exact copy is returned.
		 */
		Path GetDirectory() const;

		/**
		 * Makes the path the parent of the current path. If current path points to a file the parent path will be the
		 * folder where the file is located. Or if it contains a directory the parent will be the parent directory. If no
		 * parent exists, same path will be returned.
		 */
		Path& MakeParent();

		/**
		 * Makes the current path absolute by appending it to base. If path was already absolute no changes are made and
		 * copy of current path is returned. If base is not absolute, then the returned path will be made relative to base,
		 * but will not be absolute.
		 */
		Path& MakeAbsolute(const Path& base);

		/**
		 * Makes the current path relative to the provided base. Base must be a part of the current path. If base is not
		 * a part of the path no changes are made and a copy of the current path is returned.
		 */
		Path& MakeRelative(const Path& base);

		/** Appends another path to the end of this path. */
		Path& Append(const Path& path);

		/**
		 * Checks if the current path contains the provided path. Comparison is case insensitive and paths will be compared
		 * as-is, without canonization.
		 */
		bool Includes(const Path& child) const;

		/**
		 * Compares two paths and returns true if they match. Comparison is case insensitive and paths will be compared
		 * as-is, without canonization.
		 */
		bool Equals(const Path& other) const;

		/** Change or set the filename in the path. */
		void SetFilename(const String& filename) { mFilename = filename; }

		/**
		 * Change or set the base name in the path. Base name changes the filename by changing its base to the provided
		 * value but keeping extension intact.
		 */
		void SetBasename(const String& basename);

		/**
		 * Change or set the extension of the filename in the path.
		 *
		 * @param[in]	extension	Extension with a leading ".".
		 */
		void SetExtension(const String& extension);

		/** Returns a filename with extension. */
		const String& GetFilename() const { return mFilename; }

		/**
		 * Returns a filename in the path.
		 *
		 * @param[in]	extension	If true, returned filename will contain an extension.
		 */
		String GetFilename(bool extension) const;

		/** Returns file extension with the leading ".". */
		String GetExtension() const;

		/** Gets the number of directories in the path. */
		UINT32 GetNumDirectories() const { return (UINT32)mDirectories.Size(); }

		/** Gets a directory name with the specified index from the path. */
		const String& GetDirectory(UINT32 idx) const;

		/** Returns path device (for example drive, volume, etc.) if one exists in the path. */
		const String& GetDevice() const { return mDevice; }

		/** Returns path node (for example network name) if one exists in the path. */
		const String& GetNode() const { return mNode; }

		/**
		 * Gets last element in the path, filename if it exists, otherwise the last directory. If no directories exist
		 * returns device or node.
		 */
		const String& GetTail() const;

		/** Clears the path to nothing. */
		void Clear();

		/** Returns true if no path has been set. */
		bool IsEmpty() const { return mDirectories.Empty() && mFilename.empty() && mDevice.empty() && mNode.empty(); }

		/** Concatenates two paths. */
		Path operator+ (const Path& rhs) const;

		/** Concatenates two paths. */
		Path& operator+= (const Path& rhs);

		/** Compares two path elements (filenames, directory names, etc.). */
		static bool ComparePathElem(const String& left, const String& right);

		/** Combines two paths and returns the result. Right path should be relative. */
		static Path Combine(const Path& left, const Path& right);

		/** Strips invalid characters from the provided string and replaces them with empty spaces. */
		static void StripInvalid(String& path);

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
		void Assign(const char* pathStr, UINT32 numChars, PathType type = PathType::Default);

		/** Parses a Windows path and stores the parsed data internally. Throws an exception if parsing fails. */
		template<class T>
		void ParseWindows(const T* pathStr, UINT32 numChars)
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

					tempStream.Str(BasicString<T>());
					tempStream.Clear();
					while (idx < numChars && pathStr[idx] != '\\' && pathStr[idx] != '/')
						tempStream << pathStr[idx++];

					setNode(tempStream.Str());

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
					tempStream.Str(BasicString<T>());
					tempStream.Clear();
					while (idx < numChars && pathStr[idx] != '\\' && pathStr[idx] != '/')
					{
						tempStream << pathStr[idx];
						idx++;
					}

					if (idx < numChars)
						pushDirectory(tempStream.Str());
					else
						setFilename(tempStream.Str());

					idx++;
				}
			}
		}

		/** Parses a Unix path and stores the parsed data internally. Throws an exception if parsing fails. */
		template<class T>
		void ParseUnix(const T* pathStr, UINT32 numChars)
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
					tempStream.Str(BasicString<T>());
					tempStream.Clear();
					while (idx < numChars && pathStr[idx] != '/')
					{
						tempStream << pathStr[idx];
						idx++;
					}

					if (idx < numChars)
					{
						if (mDirectories.Empty())
						{
							BasicString<T> deviceStr = tempStream.Str();
							if (!deviceStr.Empty() && *(deviceStr.rbegin()) == ':')
							{
								setDevice(deviceStr.Substr(0, deviceStr.length() - 1));
								mIsAbsolute = true;
							}
							else
							{
								pushDirectory(deviceStr);
							}
						}
						else
						{
							pushDirectory(tempStream.Str());
						}
					}
					else
					{
						setFilename(tempStream.Str());
					}

					idx++;
				}
			}
		}

		void SetNode(const String& node) { mNode = node; }
		void SetDevice(const String& device) { mDevice = device; }

		/** Build a Windows path string from internal path data. */
		String BuildWindows() const;

		/** Build a Unix path string from internal path data. */
		String BuildUnix() const;

		/** Add new directory to the end of the path. */
		void PushDirectory(const String& dir);

		/** Helper method that throws invalid path exception. */
		void ThrowInvalidPathException(const String& path) const;
	private:
		friend struct RTTIPlainType<Path>; // For serialization
		friend struct ::std::hash<bs::Path>;

		Vector<String> mDirectories;
		String mDevice;
		String mFilename;
		String mNode;
		bool mIsAbsolute = false;
	};

	/** @} */
}

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for Path. */
	template<>
	struct hash<bs::Path>
	{
		size_t Operator()(const bs::Path& path) const
		{
			size_t hash = 0;
			bs::bs_hash_combine(hash, path.mFilename);
			bs::bs_hash_combine(hash, path.mDevice);
			bs::bs_hash_combine(hash, path.mNode);

			for (auto& dir : path.mDirectories)
				bs::bs_hash_combine(hash, dir);

			return hash;
		}
	};
}

/** @endcond */
