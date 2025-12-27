//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/B3DPlatformDefines.h"
#include "String/B3DString.h"
#include "Utility/B3DUtil.h"

namespace b3d
{
	/** @addtogroup Filesystem
	 *  @{
	 */

	/** Calculates path hash, either from case sensitive or insensitive path. */
	template<bool CaseSensitive>
	class PathHashFunction
	{
	public:
		size_t operator()(const Path& a) const { return 0; }
	};

	/**
	 * Class for storing and manipulating file paths. Paths may be parsed from and to raw strings according to various
	 * platform specific path types.
	 *
	 * @note
	 * In order to allow the system to easily distinguish between file and directory paths, try to ensure that all directory
	 * paths end with a separator (\ or / depending on platform). System won't fail if you don't but it will be easier to
	 * misuse.
	 */
	class B3D_EXPORT Path
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
		 * @param	pathStr	String containing the path. Ideally this should be an UTF-8 encoded string in order to
		 *					support non-ANSI characters in the path.
		 * @param	type	If set to default path will be parsed according to the rules of the platform the application
		 *					is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		Path(const String& pathStr, PathType type = PathType::Default);

		/**
		 * Constructs a path by parsing the provided path null terminated string. Throws exception if provided path is
		 * not valid.
		 *
		 * @param	pathStr	Null-terminated string containing the path. Ideally this should be an UTF-8 encoded string
		 *					in order to support non-ANSI characters in the path.
		 * @param	type	If set to default path will be parsed according to the rules of the platform the application
		 *					is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		Path(const char* pathStr, PathType type = PathType::Default);
		Path(const Path& other);

		/**
		 * Assigns a path by parsing the provided path string. Path will be parsed according to the rules of the platform
		 * the application is being compiled to.
		 */
		Path& operator=(const String& pathStr);

		/**
		 * Assigns a path by parsing the provided path null terminated string. Path will be parsed according to the rules
		 * of the platform the application is being compiled to.
		 */
		Path& operator=(const char* pathStr);

		Path& operator=(const Path& path);

		/**
		 * Compares two paths and returns true if they match. Comparison is case insensitive and paths will be compared
		 * as-is, without canonization.
		 */
		bool operator==(const Path& path) const { return Equals(path); }

		/**
		 * Compares two paths and returns true if they don't match. Comparison is case insensitive and paths will be
		 * compared as-is, without canonization.
		 */
		bool operator!=(const Path& path) const { return !Equals(path); }

		/** Gets a directory name with the specified index from the path. */
		const String& operator[](u32 index) const { return GetDirectory(index); }

		/** Swap internal data with another Path object. */
		void Swap(Path& path);

		/**	Create a path from another Path object. */
		void Assign(const Path& path);

		/**
		 * Constructs a path by parsing the provided path string. Throws exception if provided path is not valid.
		 *
		 * @param	pathStr	String containing the path.
		 * @param	type	If set to default path will be parsed according to the rules of the platform the application
		 *					is being compiled to. Otherwise it will be parsed according to provided type.
		 */
		void Assign(const String& pathStr, PathType type = PathType::Default);

		/**
		 * Constructs a path by parsing the provided path null terminated string. Throws exception if provided path is not
		 * valid.
		 *
		 * @param	pathStr		Null-terminated string containing the path.
		 * @param	type		If set to default path will be parsed according to the rules of the platform the
		 *						application is being compiled to. Otherwise it will be parsed according to provided
		 *						type.
		 */
		void Assign(const char* pathStr, PathType type = PathType::Default);

		/**
		 * Converts the path in a string according to platform path rules.
		 *
		 * @param	type	If set to default path will be parsed according to the rules of the platform the application is
		 *					being compiled to. Otherwise it will be parsed according to provided type.
		 * @return			String representing the path using the UTF8 string encoding.
		 */
		String ToString(PathType type = PathType::Default) const;

		/**
		 * Converts the path to either a string or a wstring, doing The Right Thing for the current platform.
		 *
		 * This method is equivalent to toWString() on Windows, and to toString() elsewhere.
		 */
#if B3D_PLATFORM_WIN32
		WString ToPlatformString() const;
#else
		String toPlatformString() const
		{
			return toString();
		}
#endif

		/** Checks is the path a directory (contains no file-name). */
		bool IsDirectory() const { return mFilename.empty(); }

		/** Checks does the path point to a file. */
		bool IsFile() const { return !mFilename.empty(); }

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

		/** Checks if the current path contains the provided path. */
		bool Includes(const Path& child, bool caseSensitive = false) const;

		/** Compares two paths and returns true if they match. */
		bool Equals(const Path& other, bool caseSensitive = false) const;

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
		 * @param	extension	Extension with a leading ".".
		 */
		void SetExtension(const String& extension);

		/** Returns a filename with extension. */
		const String& GetFilename() const { return mFilename; }

		/**
		 * Returns a filename in the path.
		 *
		 * @param	extension	If true, returned filename will contain an extension.
		 */
		String GetFilename(bool extension) const;

		/** Returns file extension with the leading ".". */
		String GetExtension() const;

		/** Gets the number of directories in the path. */
		u32 GetDirectoryCount() const { return (u32)mDirectories.size(); }

		/** Gets a directory name with the specified index from the path. */
		const String& GetDirectory(u32 index) const;

		/** Returns path device (for example drive, volume, etc.) if one exists in the path. */
		const String& GetDevice() const { return mDevice; }

		/** Returns path node (for example network name) if one exists in the path. */
		const String& GetNode() const { return mNode; }

		/** Gets last element in the path, filename if it exists, otherwise the last directory. */
		const String& GetTail() const;

		/** Removes the last element in the path: filename if it exists, otherwise the last directory. Returns the removed value. */
		String PopTail();

		/** Returns the portion of the path containing the first @p directoryCount directories. */
		Path GetSubPath(u32 directoryCount) const;

		/** Add new directory to the end of the path. */
		void PushDirectory(const String& dir);

		/** Removes the last directory from the end of the path. */
		void PopDirectory();

		/** Clears the path to nothing. */
		void Clear();

		/** Returns true if no path has been set. */
		bool IsEmpty() const { return mDirectories.empty() && mFilename.empty() && mDevice.empty() && mNode.empty(); }

		/** Concatenates two paths. */
		Path operator+(const Path& rhs) const;

		/** Concatenates two paths. */
		Path& operator+=(const Path& rhs);

		/** Compares two path elements (filenames, directory names, etc.). */
		static bool ComparePathElem(const String& left, const String& right, bool caseSensitive = false);

		/** Combines two paths and returns the result. Right path should be relative. */
		static Path Combine(const Path& left, const Path& right);

		/** Combines three paths and returns the result. Right path should be relative. */
		static Path Combine(const Path& left, const Path& middle, const Path& right);

		/** Strips invalid characters from the provided string and replaces them with empty spaces. */
		static void StripInvalid(String& path);

		static const Path kBlank;

	private:
		/**
		 * Constructs a path by parsing the provided raw string data. Throws exception if provided path is not valid.
		 *
		 * @param	pathStr			String containing the path.
		 * @param	characterCount	Number of character in the provided path string.
		 * @param	type			If set to default path will be parsed according to the rules of the platform the
		 *							application is being compiled to. Otherwise it will be parsed according to provided
		 *							type.
		 */
		void Assign(const char* pathStr, u32 characterCount, PathType type = PathType::Default);

		/** Parses a Windows path and stores the parsed data internally. Throws an exception if parsing fails. */
		template <class T>
		void ParseWindows(const T* pathStr, u32 numChars)
		{
			Clear();

			u32 idx = 0;
			BasicStringStream<T> tempStream;

			if(idx < numChars)
			{
				if(pathStr[idx] == '\\' || pathStr[idx] == '/')
				{
					mIsAbsolute = true;
					idx++;
				}
			}

			if(idx < numChars)
			{
				// Path starts with a node, a drive letter or is relative
				if(mIsAbsolute && (pathStr[idx] == '\\' || pathStr[idx] == '/')) // Node
				{
					idx++;

					tempStream.str(BasicString<T>());
					tempStream.clear();
					while(idx < numChars && pathStr[idx] != '\\' && pathStr[idx] != '/')
						tempStream << pathStr[idx++];

					SetNode(tempStream.str());

					if(idx < numChars)
						idx++;
				}
				else // A drive letter or not absolute
				{
					T drive = pathStr[idx];
					idx++;

					if(idx < numChars && pathStr[idx] == ':')
					{
						if(mIsAbsolute || !((drive >= 'a' && drive <= 'z') || (drive >= 'A' && drive <= 'Z')))
							ThrowInvalidPathException(BasicString<T>(pathStr, numChars));

						mIsAbsolute = true;
						SetDevice(String(1, drive));

						idx++;

						if(idx >= numChars || (pathStr[idx] != '\\' && pathStr[idx] != '/'))
							ThrowInvalidPathException(BasicString<T>(pathStr, numChars));

						idx++;
					}
					else
						idx--;
				}

				while(idx < numChars)
				{
					tempStream.str(BasicString<T>());
					tempStream.clear();
					while(idx < numChars && pathStr[idx] != '\\' && pathStr[idx] != '/')
					{
						tempStream << pathStr[idx];
						idx++;
					}

					if(idx < numChars)
						PushDirectory(tempStream.str());
					else
						SetFilename(tempStream.str());

					idx++;
				}
			}
		}

		/** Parses a Unix path and stores the parsed data internally. Throws an exception if parsing fails. */
		template <class T>
		void ParseUnix(const T* pathStr, u32 numChars)
		{
			Clear();

			u32 idx = 0;
			BasicStringStream<T> tempStream;

			if(idx < numChars)
			{
				if(pathStr[idx] == '/')
				{
					mIsAbsolute = true;
					idx++;
				}
				else if(pathStr[idx] == '~')
				{
					idx++;
					if(idx >= numChars || pathStr[idx] == '/')
					{
						PushDirectory(String("~"));
						mIsAbsolute = true;
					}
					else
						idx--;
				}

				while(idx < numChars)
				{
					tempStream.str(BasicString<T>());
					tempStream.clear();
					while(idx < numChars && pathStr[idx] != '/')
					{
						tempStream << pathStr[idx];
						idx++;
					}

					if(idx < numChars)
					{
						if(mDirectories.empty())
						{
							BasicString<T> deviceStr = tempStream.str();
							if(!deviceStr.empty() && *(deviceStr.rbegin()) == ':')
							{
								SetDevice(deviceStr.substr(0, deviceStr.length() - 1));
								mIsAbsolute = true;
							}
							else
							{
								PushDirectory(deviceStr);
							}
						}
						else
						{
							PushDirectory(tempStream.str());
						}
					}
					else
					{
						SetFilename(tempStream.str());
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

		/** Helper method that throws invalid path exception. */
		void ThrowInvalidPathException(const String& path) const;

	private:
		friend struct RTTIPlainType<Path>; // For serialization
		friend struct ::std::hash<b3d::Path>;
		friend class PathHashFunction<false>;
		friend class PathHashFunction<true>;

		Vector<String> mDirectories;
		String mDevice;
		String mFilename;
		String mNode;
		bool mIsAbsolute = false;
	};

	/**	Compares two paths using either case sensitive or insensitive compare. */
	template<bool CaseSensitive>
	class PathEqualsFunction
	{
	public:
		bool operator()(const Path& a, const Path& b) const
		{
			return a.Equals(b, CaseSensitive);
		}
	};

	/** Calculates path hash from a case insensitive path. */
	template<>
	class B3D_EXPORT PathHashFunction<true>
	{
	public:
		size_t operator()(const Path& path) const;
	};

	/** Calculates path hash from a case insensitive path. */
	template<>
	class B3D_EXPORT PathHashFunction<false>
	{
	public:
		size_t operator()(const Path& path) const;
	};

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for Path. */
	template <>
	struct hash<b3d::Path>
	{
		size_t operator()(const b3d::Path& path) const
		{
			return b3d::PathHashFunction<false>()(path);
		}
	};
} // namespace std

/** @endcond */
