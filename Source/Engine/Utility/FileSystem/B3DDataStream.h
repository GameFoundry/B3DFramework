//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include <istream>

namespace b3d
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
	class B3D_EXPORT DataStream
	{
	public:
		enum AccessMode
		{
			READ = 1,
			WRITE = 2
		};

	public:
		/** Creates an unnamed stream. */
		DataStream(u16 accessMode = READ)
			: mAccess(accessMode)
		{}

		/** Creates a named stream. */
		DataStream(const String& name, u16 accessMode = READ)
			: mName(name), mAccess(accessMode) {}

		virtual ~DataStream() = default;

		const String& GetName() const { return mName; }

		u16 GetAccessMode() const { return mAccess; }

		virtual bool IsReadable() const { return (mAccess & READ) != 0; }

		virtual bool IsWriteable() const { return (mAccess & WRITE) != 0; }

		/** Checks whether the stream reads/writes from a file system. */
		virtual bool IsFile() const = 0;

		/** Reads data from the buffer and copies it to the specified value. */
		template <typename T>
		DataStream& operator>>(T& val);

		/**
		 * Read the requisite number of bytes from the stream, stopping at the end of the file. Advances
		 * the read pointer.
		 *
		 * @param	outData		Pre-allocated buffer to read the data into.
		 * @param	byteCount	Number of bytes to read.
		 * @return				Number of bytes actually read.
		 *
		 * @note	Stream must be created with READ access mode.
		 */
		virtual size_t Read(void* outData, size_t byteCount) const = 0;

		/**
		 * Write the requisite number of bytes to the stream and advance the write pointer.
		 *
		 * @param	data		Buffer containing bytes to write.
		 * @param	byteCount	Number of bytes to write.
		 * @return				Number of bytes actually written.
		 *
		 * @note	Stream must be created with WRITE access mode.
		 */
		virtual size_t Write(const void* data, size_t byteCount) { return 0; }

		/**
		 * Reads bits from the stream into the provided buffer from the current cursor location and advances the cursor.
		 * If the stream doesn't support per-bit reads, data size will be rounded up to nearest byte.
		 *
		 * @param	outData	Buffer to read the data from. Must have enough capacity to store @p count bits.
		 * @param	count	Number of bits to read.
		 * @return			Number of bits actually read.
		 *
		 * @note	Stream must be created with READ access mode.
		 */
		virtual size_t ReadBits(uint8_t* outData, uint32_t count);

		/**
		 * Writes bits from the provided buffer into the stream at the current cursor location and advances the cursor.
		 * If the stream doesn't support per-bit writes, data size will be rounded up to nearest byte.
		 *
		 * @param	data	Buffer to write the data from. Must have enough capacity to store @p count bits.
		 * @param	count	Number of bits to write.
		 * @return			Number of bits actually written.
		 *
		 * @note	Stream must be created with WRITE access mode.
		 */
		virtual size_t WriteBits(const uint8_t* data, uint32_t count);

		/**
		 * Writes the provided narrow string to the steam. String is convered to the required encoding before being written.
		 *
		 * @param	string		String containing narrow characters to write, encoded as UTF8.
		 * @param	encoding	Encoding to convert the string to before writing.
		 */
		virtual void WriteString(const String& string, StringEncoding encoding = StringEncoding::UTF8);

		/**
		 * Writes the provided wide string to the steam. String is convered to the required encoding before being written.
		 *
		 * @param	string		String containing wide characters to write, encoded as specified by platform for
		 * 						wide characters.
		 * @param	encoding	Encoding to convert the string to before writing.
		 */
		virtual void WriteString(const WString& string, StringEncoding encoding = StringEncoding::UTF8);

		/**
		 * Returns a string containing the entire stream.
		 *
		 * @return	String data encoded as UTF-8.
		 *
		 * @note	This is a convenience method for text streams only, allowing you to retrieve a String object containing
		 *			all the data in the stream.
		 */
		virtual String GetAsString();

		/**
		 * Returns a wide string containing the entire stream.
		 *
		 * @return	Wide string encoded as specified by current platform.
		 *
		 * @note	This is a convenience method for text streams only, allowing you to retrieve a WString object
		 *			containing all the data in the stream.
		 */
		virtual WString GetAsWString();

		/** Skip a defined number of bytes. Returns the actual number of bytes skipped. */
		virtual size_t Skip(size_t count) = 0;

		/** Repositions the read or write cursor to the specified byte. Returns the actual byte the cursor has been placed at, in case end has been reached earlier*/
		virtual size_t Seek(size_t pos) = 0;

		/** Returns the current cursor byte offset from beginning. */
		virtual size_t Tell() const = 0;

		/**
		 * Aligns the read/write cursor to a byte boundary. @p count determines the alignment in bytes. Note the
		 * requested alignment might not be achieved if count > 1 and it would move the cursor past the capacity of the
		 * buffer, as the cursor will be clamped to buffer end regardless of alignment.
		 */
		virtual void Align(uint32_t count = 1);

		/** Returns true if the stream has reached the end. */
		virtual bool Eof() const = 0;

		/** Returns the total size of the data to be read from the stream, or 0 if this is indeterminate for this stream. */
		size_t Size() const { return mSize; }

		/**
		 * Creates a copy of this stream.
		 *
		 * @param	copyData	If true the internal stream data will be copied as well, otherwise it will just
		 *						reference the data from the original stream (in which case the caller must ensure the
		 *						original stream outlives the clone). This is not relevant for file streams.
		 */
		virtual SPtr<DataStream> Clone(bool copyData = true) const = 0;

		/** Flushes the stream, writing any buffer data to the destination. Returns false if some error occurred and data was not written correctly. Does not need to be called if stream was just read from. */
		virtual bool Flush() = 0;

		/** Closes the stream. This makes further operations invalid. Executes a Flush() before closing, and passes the return value from Flush(). */
		virtual bool Close() = 0;

	protected:
		static const u32 kStreamTempSize;

		String mName;
		size_t mSize = 0;
		u16 mAccess;
	};

	/** Data stream for handling data from memory. Data is stored in a memory block that is either owned by the stream (freed when stream goes out of scope), or owned externally. */
	class B3D_EXPORT MemoryDataStream : public DataStream
	{
	public:
		/**
		 * Initializes an empty memory stream. As data is written the stream will grow its internal memory storage
		 * automatically.
		 */
		MemoryDataStream();

		/**
		 * Initializes a stream with some initial capacity. If more bytes than capacity is written, the stream will
		 * grow its internal memory storage.
		 *
		 * @param	capacity	Number of bytes to initially allocate for the internal memory storage.
		 */
		MemoryDataStream(size_t capacity);

		/**
		 * Wrap an existing memory chunk in a stream.
		 *
		 * @param 	memory		Memory to wrap the data stream around.
		 * @param	size		Size of the memory chunk in bytes.
		 */
		MemoryDataStream(void* memory, size_t size);

		/**
		 * Create a stream which pre-buffers the contents of another stream. Data from the other buffer will be entirely
		 * read and stored in an internal buffer.
		 */
		MemoryDataStream(const MemoryDataStream& other);

		/**
		 * Create a stream which pre-buffers the contents of another stream. Data from the other buffer will be entirely
		 * read and stored in an internal buffer.
		 */
		MemoryDataStream(const SPtr<DataStream>& other);

		/** Inherits the data from the provided stream, invalidating the source stream. */
		MemoryDataStream(MemoryDataStream&& other);
		~MemoryDataStream();

		MemoryDataStream& operator=(const MemoryDataStream& other);
		MemoryDataStream& operator=(MemoryDataStream&& other);

		/** Get a pointer to the start of the memory block this stream holds. */
		uint8_t* Data() const { return mData; }

		/** Get a pointer to the current position in the memory block this stream holds. */
		uint8_t* Cursor() const { return mCursor; }

		bool IsFile() const override { return false; }
		size_t Read(void* data, size_t byteCount) const override;
		size_t Write(const void* data, size_t byteCount) override;
		size_t Skip(size_t byteCount) override;
		size_t Seek(size_t position) override;
		size_t Tell() const override;
		bool Eof() const override;
		SPtr<DataStream> Clone(bool copyData = true) const override;
		bool Flush() override { return true; }
		bool Close() override;

		/**
		 * Disowns the internal memory buffer, ensuring it wont be released when the stream goes out of scope.
		 * The caller becomes responsible for freeing the internal data buffer.
		 */
		uint8_t* DisownMemory()
		{
			mOwnsMemory = false;
			return mData;
		}

	protected:
		/** Reallocates the internal buffer making enough room for @p byteCount. */
		void ReallocateBuffer(size_t byteCount);

		/**
		 * Ensures the underlying buffer has enough space to store @p size bytes. This will be calculated starting from the current offset.
		 * If the new size exceeds current capacity and buffer supports resizing, it will be increased to fit the new size. If the buffer
		 * cannot be resized, the amount of bytes that fit into the current buffer will be returned by this method.
		 */
		size_t EnsureEnoughSpace(size_t size);

		uint8_t* mData = nullptr;
		mutable uint8_t* mCursor = nullptr;
		uint8_t* mEnd = nullptr;
		size_t mCapacity = 0;

		bool mOwnsMemory = true;
	};

	/** Data stream for handling data from standard streams. */
	class B3D_EXPORT FileDataStream final : public DataStream
	{
	public:
		/**
		 * Constructs a file stream.
		 *
		 * @param	filePath	Path of the file to open.
		 * @param	accessMode	Determines should the file be opened in read, write or read/write mode.
		 */
		FileDataStream(const Path& filePath, AccessMode accessMode = READ);
		~FileDataStream() override;

		/** Opens the file stream. Must be called before any actions on the stream. Returns false if not successful. */
		bool Open();
		bool IsFile() const override { return true; }
		size_t Read(void* data, size_t byteCount) const override;
		size_t Write(const void* data, size_t byteCount) override;
		size_t Skip(size_t count) override;
		size_t Seek(size_t pos) override;
		size_t Tell() const override;
		bool Eof() const override;
		SPtr<DataStream> Clone(bool copyData = true) const override;
		bool Flush() override;
		bool Close() override;

		/** Returns the path of the file opened by the stream. */
		const Path& GetPath() const { return mPath; }

	protected:
		Path mPath;
		std::fstream mFileStream;
	};

	/** @} */
} // namespace b3d
