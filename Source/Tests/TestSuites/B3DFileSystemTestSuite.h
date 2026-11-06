//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class FileSystemTestSuite : public TestSuite
	{
	public:
		FileSystemTestSuite();
		void StartUp() override;
		void ShutDown() override;

	private:
		/** Verifies nested filesystem ownership leaves storage and asynchronous reads available. */
		void TestSharedLifetime();
		/** Verifies reader/writer sharing, release on close, and rollback after a failed open. */
		void TestFileSharing();
		/** Verifies a live stream remains protected and readable after many other files have been opened. */
		void TestFileSharingUnderHandlePressure();
		/** Verifies simultaneous exclusive opens cannot both succeed. */
		void TestConcurrentFileSharing();
		/** Verifies synchronous, asynchronous, chunked and concurrent reads of a large file all return the same bytes. */
		void TestReadPathsAgree();
		/** Verifies readers of many distinct files still read the right content after opening more files than descriptors allow. */
		void TestReadAfterDescriptorEviction();

		void TestExistsYesFile();
		void TestExistsYesDir();
		void TestExistsNo();
		void TestGetFileSizeZero();
		void TestGetFileSizeNotZero();
		void TestIsFileYes();
		void TestIsFileNo();
		void TestIsDirectoryYes();
		void TestIsDirectoryNo();
		void TestRemoveFile();
		void TestRemoveDirectory();
		void TestMove();
		void TestMoveOverwriteExisting();
		void TestMoveNoOverwriteExisting();
		void TestCopy();
		void TestCopyRecursive();
		void TestCopyOverwriteExisting();
		void TestCopyNoOverwriteExisting();
		void TestGetChildren();
		void TestGetLastModifiedTime();
		void TestGetTempDirectoryPath();
		void TestStreamWriteReadRoundtrip();
		void TestMemoryStreamCustomDeleter();
		void TestMemoryStreamDeleterFixedCapacity();
		void TestMemoryStreamSizedConstructor();
		void TestMemoryStreamCopy();
		void TestMemoryStreamSeekPastCapacity();
		void TestFileStreamClone();
		void TestSerializedDataBlockFileStream();
		void TestOpenFileMissing();
		void TestOpenFileAsyncRead();
		void TestOpenFileAsyncUserMemory();
		void TestOpenFileAsyncEof();
		void TestAsyncConcurrentReads();
		void TestAsyncCloseWhileInFlight();
		void TestAsyncLargeFileChunkChaining();
		void TestAsyncFallbackWhenNotAsyncOpened();
		void TestAsyncEmptyFile();

		Path mTestDirectory;
	};
} // namespace b3d
