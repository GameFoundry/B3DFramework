//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "FileSystem/B3DDataStream.h"

namespace b3d
{
	/** @addtogroup Filesystem
	 *  @{
	 */

	/**
	 * Data stream backed by native POSIX file APIs (open/pread/pwrite). Used as the synchronous base class for the
	 * platform-specific Linux and macOS file streams.
	 *
	 * The base class does not provide a native asynchronous ReadAsync() override - it inherits the default synchronous
	 * implementation from DataStream. Derived classes override ReadAsync() to use a native asynchronous path
	 * (io_uring on Linux, dispatch_io on macOS) when FileAccessFlag::Async is set; if the async backend fails to
	 * initialize, the stream remains usable and falls back to this base's synchronous implementation.
	 */
	class UnixFileDataStream : public DataStream
	{
	public:
		/**
		 * Constructs a file stream.
		 *
		 * @param	filePath	Path of the file to open.
		 * @param	access		Combination of FileAccessFlag values determining how the file is accessed.
		 */
		UnixFileDataStream(const Path& filePath, FileAccessFlags access = FileAccessFlag::Read);
		~UnixFileDataStream() override;

		/** Opens the file stream. Must be called before any actions on the stream. Returns false if not successful. */
		virtual bool Open();
		bool IsFile() const override { return true; }
		bool IsReadable() const override { return mAccess.IsSet(FileAccessFlag::Read); }
		bool IsWriteable() const override { return mAccess.IsSet(FileAccessFlag::Write); }
		size_t Read(void* data, size_t byteCount) const override;
		size_t Write(const void* data, size_t byteCount) override;
		size_t Skip(size_t count) override;
		size_t Seek(size_t pos) override;
		size_t Tell() const override;
		bool Eof() const override;
		TShared<DataStream> Clone(bool copyData = true) const override;
		bool Flush() override;
		bool Close() override;
		const Path& GetPath() const override { return mPath; }

	protected:
		Path mPath;
		FileAccessFlags mAccess; /**< How the file was opened (read/write/async). */
		int mFd = -1; /**< POSIX file descriptor; -1 when the stream is closed. */
		mutable u64 mCursor = 0; /**< Current read/write byte offset from the start of the file. */
		mutable bool mEof = false; /**< Set once a read couldn't satisfy the full request (end of file reached). */
	};

	/** @} */
} // namespace b3d
