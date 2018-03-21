//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include <istream>

namespace bs 
{
	/** @addtogroup Filesystem
	 *  @{
	 */

	/** Supported encoding types for strings. */
	enum class StringEncoding
	{
		UTF8 = 1,
		UTF16 = 2
	};

	/**
	 * General purpose class used for encapsulating the reading and writing of data from and to various sources using a 
	 * common interface.
	 */
	class BS_UTILITY_EXPORT DataStream
	{
	public:
		enum AccessMode
		{
			READ = 1, 
			WRITE = 2
		};

	public:
		/** Creates an unnamed stream. */
		DataStream(UINT16 accessMode = READ) 
			: mAccess(accessMode) 
		{ }

		/** Creates a named stream. */
		DataStream(const String& name, UINT16 accessMode = READ) 
			:mName(name), mAccess(accessMode) {}

		virtual ~DataStream() = default;

		const String& getName(void) { return mName; }
		UINT16 getAccessMode() const { return mAccess; }

		virtual bool isReadable() const { return (mAccess & READ) != 0; }
		virtual bool isWriteable() const { return (mAccess & WRITE) != 0; }
		virtual bool isFile() const = 0;

		/** Reads data from the buffer and copies it to the specified value. */
		template<typename T> DataStream& operator>>(T& val);

		/**
		 * Read the requisite number of bytes from the stream, stopping at the end of the file. Advances
		 * the read pointer.
		 *
		 * @param[in]	buf		Pre-allocated buffer to read the data into.
		 * @param[in]	count	Number of bytes to read.
		 * @return				Number of bytes actually read.
		 * 			
		 * @note	Stream must be created with READ access mode.
		 */
		virtual size_t read(void* buf, size_t count) = 0;

		/**
		 * Write the requisite number of bytes to the stream and advance the write pointer.
		 *
		 * @param[in]	buf		Buffer containing bytes to write.
		 * @param[in]	count	Number of bytes to write.
		 * @return				Number of bytes actually written.
		 * 			
		 * @note	Stream must be created with WRITE access mode.
		 */
		virtual size_t write(const void* buf, size_t count) { return 0; }

		/**
		 * Writes the provided narrow string to the steam. String is convered to the required encoding before being written.
		 * 			
		 * @param[in]	string		String containing narrow characters to write, encoded as UTF8.
		 * @param[in]	encoding	Encoding to convert the string to before writing.
		 */
		virtual void writeString(const String& string, StringEncoding encoding = StringEncoding::UTF8);

		/**
		 * Writes the provided wide string to the steam. String is convered to the required encoding before being written.
		 * 			
		 * @param[in]	string		String containing wide characters to write, encoded as specified by platform for 
		 * 							wide characters.
		 * @param[in]	encoding	Encoding to convert the string to before writing.
		 */
		virtual void writeString(const WString& string, StringEncoding encoding = StringEncoding::UTF8);

		/**
		 * Returns a string containing the entire stream.
		 *
		 * @return	String data encoded as UTF-8. 
		 *
		 * @note	This is a convenience method for text streams only, allowing you to retrieve a String object containing 
		 *			all the data in the stream.
		 */
		virtual String getAsString();

		/**
		 * Returns a wide string containing the entire stream.
		 *
		 * @return	Wide string encoded as specified by current platform.
		 *
		 * @note	This is a convenience method for text streams only, allowing you to retrieve a WString object 
		 *			containing all the data in the stream.
		 */
		virtual WString getAsWString();

		/**
		 * Skip a defined number of bytes. This can also be a negative value, in which case the file pointer rewinds a 
		 * defined number of bytes.
		 */
		virtual void skip(size_t count) = 0;
	
		/** Repositions the read point to a specified byte. */
		virtual void seek(size_t pos) = 0;
		
		/** Returns the current byte offset from beginning. */
		virtual size_t tell() const = 0;

		/** Returns true if the stream has reached the end. */
		virtual bool eof() const = 0;

		/** Returns the total size of the data to be read from the stream, or 0 if this is indeterminate for this stream. */
		size_t size() const { return mSize; }

		/** 
		 * Creates a copy of this stream. 
		 *
		 * @param[in]	copyData	If true the internal stream data will be copied as well, otherwise it will just 
		 *							reference the data from the original stream (in which case the caller must ensure the
		 *							original stream outlives the clone). This is not relevant for file streams.
		 */
		virtual SPtr<DataStream> clone(bool copyData = true) const = 0;

		/** Close the stream. This makes further operations invalid. */
		virtual void close() = 0;
		
	protected:
		static const UINT32 StreamTempSize;

		String mName;		
		size_t mSize = 0;
		UINT16 mAccess;
	};

	/** Data stream for handling data from memory. */
	class BS_UTILITY_EXPORT MemoryDataStream : public DataStream
	{		
	public:
		/**
		 * Allocates a new chunk of memory and wraps it in a stream.
		 *
		 * @param[in]	size		Size of the memory chunk in bytes.
		 */
		MemoryDataStream(size_t size);

		/**
		 * Wrap an existing memory chunk in a stream.
		 *
		 * @param[in] 	memory		Memory to wrap the data stream around.
		 * @param[in]	size		Size of the memory chunk in bytes.
		 * @param[in]	freeOnClose	Should the memory buffer be freed when the data stream goes out of scope.
		 */
		MemoryDataStream(void* memory, size_t size, bool freeOnClose = true);

		/**
		 * Create a stream which pre-buffers the contents of another stream. Data from the other buffer will be entirely 
		 * read and stored in an internal buffer.
		 *
		 * @param[in]	sourceStream		Stream to read data from.
		 */
		MemoryDataStream(DataStream& sourceStream);
		
		/**
		 * Create a stream which pre-buffers the contents of another stream. Data from the other buffer will be entirely 
		 * read and stored in an internal buffer.
		 *
		 * @param[in]	sourceStream		Stream to read data from.
		 */
		MemoryDataStream(const SPtr<DataStream>& sourceStream);

		~MemoryDataStream();

		bool isFile() const override { return false; }

		/** Get a pointer to the start of the memory block this stream holds. */
		UINT8* getPtr() const { return mData; }
		
		/** Get a pointer to the current position in the memory block this stream holds. */
		UINT8* getCurrentPtr() const { return mPos; }
		
		/** @copydoc DataStream::read */
		size_t read(void* buf, size_t count) override;

		/** @copydoc DataStream::write */
		size_t write(const void* buf, size_t count) override;

		/** @copydoc DataStream::skip */
		void skip(size_t count) override;
	
		/** @copydoc DataStream::seek */
		void seek(size_t pos) override;
		
		/** @copydoc DataStream::tell */
		size_t tell() const override;

		/** @copydoc DataStream::eof */
		bool eof() const override;

		/** @copydoc DataStream::clone */
		SPtr<DataStream> clone(bool copyData = true) const override;

		/** @copydoc DataStream::close */
		void close() override;

	protected:
		UINT8* mData;
		UINT8* mPos;
		UINT8* mEnd;

		bool mFreeOnClose;
	};

	/** Data stream for handling data from standard streams. */
	class BS_UTILITY_EXPORT FileDataStream : public DataStream
	{
	public:
		/**
		 * Construct a file stream.
		 *
		 * @param[in]	filePath	Path of the file to open.
		 * @param[in]	accessMode	Determines should the file be opened in read, write or read/write mode.
		 * @param[in]	freeOnClose	Determines should the internal stream be freed once the data stream is closed or goes 
		 *							out of scope.
		 */
		FileDataStream(const Path& filePath, AccessMode accessMode = READ, bool freeOnClose = true);

		~FileDataStream();

		bool isFile() const override { return true; }

		/** @copydoc DataStream::read */
		size_t read(void* buf, size_t count) override;

		/** @copydoc DataStream::write */
		size_t write(const void* buf, size_t count) override;

		/** @copydoc DataStream::skip */
		void skip(size_t count) override;
	
		/** @copydoc DataStream::seek */
		void seek(size_t pos) override;

		/** @copydoc DataStream::tell */
		size_t tell() const override;

		/** @copydoc DataStream::eof */
		bool eof() const override;

		/** @copydoc DataStream::clone */
		SPtr<DataStream> clone(bool copyData = true) const override;

		/** @copydoc DataStream::close */
		void close() override;

		/** Returns the path of the file opened by the stream. */
		const Path& getPath() const { return mPath; }

	protected:
		Path mPath;
		SPtr<std::istream> mInStream;
		SPtr<std::ifstream> mFStreamRO;
		SPtr<std::fstream> mFStream;
		bool mFreeOnClose;	
	};

	/** @} */
}

