//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFileSystemTestSuite.h"

#include "Debug/B3DDebug.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Utility/B3DUUID.h"

#include <algorithm>
#include <atomic>
#include <fstream>
#include <thread>

using namespace b3d;

void CreateFile(Path path, String content)
{
	std::ofstream fs;
	fs.open(path.ToPlatformString().c_str());
	fs << content;
	fs.close();
}

void CreateEmptyFile(Path path)
{
	CreateFile(path, "");
}

// Binary-safe variant of CreateFile that doesn't normalise newlines, used by tests that need exact byte-pattern
// reproducibility (e.g. the async-read tests that verify each byte after a read-back).
void CreateBinaryFile(Path path, const u8* data, size_t size)
{
	std::ofstream fs;
	fs.open(path.ToPlatformString().c_str(), std::ios::binary | std::ios::trunc);
	if(size > 0)
		fs.write(reinterpret_cast<const char*>(data), (std::streamsize)size);
	fs.close();
}

String ReadFile(Path path)
{
	String content;
	std::ifstream fs;
	fs.open(path.ToPlatformString().c_str());
	fs >> content;
	fs.close();
	return content;
}

void FileSystemTestSuite::StartUp()
{
	mTestDirectory = FileSystem::GetTemporaryFolderPath() + ("FileSystemTestDirectory-" + UUIDGenerator::GenerateRandom().ToString() + "/");
	FileSystem::CreateFolder(mTestDirectory);
	B3D_TEST_ASSERT_MSG(FileSystem::Exists(mTestDirectory), "FileSystemTestSuite::StartUp(): test directory creation failed");
}

void FileSystemTestSuite::ShutDown()
{
	FileSystem::Remove(mTestDirectory, true);
	if(FileSystem::Exists(mTestDirectory))
	{
		B3D_LOG(Error, LogUnitTest, "FileSystemTestSuite failed to delete '{0}', you should remove it manually.", mTestDirectory);
	}
}

FileSystemTestSuite::FileSystemTestSuite()
	: TestSuite("FileSystemTestSuite")
{
	B3D_ADD_TEST(FileSystemTestSuite::TestSharedLifetime);
	B3D_ADD_TEST(FileSystemTestSuite::TestFileSharing);
	B3D_ADD_TEST(FileSystemTestSuite::TestFileSharingUnderHandlePressure);
	B3D_ADD_TEST(FileSystemTestSuite::TestConcurrentFileSharing);
	B3D_ADD_TEST(FileSystemTestSuite::TestExistsYesFile);
	B3D_ADD_TEST(FileSystemTestSuite::TestExistsYesDir);
	B3D_ADD_TEST(FileSystemTestSuite::TestExistsNo);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetFileSizeZero);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetFileSizeNotZero);
	B3D_ADD_TEST(FileSystemTestSuite::TestIsFileYes);
	B3D_ADD_TEST(FileSystemTestSuite::TestIsFileNo);
	B3D_ADD_TEST(FileSystemTestSuite::TestIsDirectoryYes);
	B3D_ADD_TEST(FileSystemTestSuite::TestIsDirectoryNo);
	B3D_ADD_TEST(FileSystemTestSuite::TestRemoveFile);
	B3D_ADD_TEST(FileSystemTestSuite::TestRemoveDirectory);
	B3D_ADD_TEST(FileSystemTestSuite::TestMove);
	B3D_ADD_TEST(FileSystemTestSuite::TestMoveOverwriteExisting);
	B3D_ADD_TEST(FileSystemTestSuite::TestMoveNoOverwriteExisting);
	B3D_ADD_TEST(FileSystemTestSuite::TestCopy);
	B3D_ADD_TEST(FileSystemTestSuite::TestCopyRecursive);
	B3D_ADD_TEST(FileSystemTestSuite::TestCopyOverwriteExisting);
	B3D_ADD_TEST(FileSystemTestSuite::TestCopyNoOverwriteExisting);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetChildren);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetLastModifiedTime);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetTempDirectoryPath);
	B3D_ADD_TEST(FileSystemTestSuite::TestStreamWriteReadRoundtrip);
	B3D_ADD_TEST(FileSystemTestSuite::TestMemoryStreamCustomDeleter);
	B3D_ADD_TEST(FileSystemTestSuite::TestMemoryStreamDeleterFixedCapacity);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileMissing);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileAsyncRead);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileAsyncUserMemory);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileAsyncEof);
	B3D_ADD_TEST(FileSystemTestSuite::TestAsyncConcurrentReads);
	B3D_ADD_TEST(FileSystemTestSuite::TestAsyncCloseWhileInFlight);
	B3D_ADD_TEST(FileSystemTestSuite::TestAsyncLargeFileChunkChaining);
	B3D_ADD_TEST(FileSystemTestSuite::TestAsyncFallbackWhenNotAsyncOpened);
	B3D_ADD_TEST(FileSystemTestSuite::TestAsyncEmptyFile);
}

void FileSystemTestSuite::TestSharedLifetime()
{
	const Path temporaryFolder = FileSystem::GetTemporaryFolderPath();
	FileSystem::StartUp();
	FileSystem::StartUp();
	FileSystem::ShutDown();
	FileSystem::ShutDown();
	B3D_TEST_ASSERT(FileSystem::GetTemporaryFolderPath() == temporaryFolder);

	const Path path = mTestDirectory + "shared-lifetime";
	const String content = "Filesystem services survive nested ownership.";
	CreateFile(path, content);

	const TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);
	if(stream == nullptr)
		return;

	TAsyncOp<TShared<MemoryDataStream>> read = stream->ReadAsync(0, content.size());
	read.BlockUntilComplete();
	const TShared<MemoryDataStream> data = read.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	if(data != nullptr)
		B3D_TEST_ASSERT(String((const char*)data->Data(), data->Size()) == content);
}

void FileSystemTestSuite::TestFileSharing()
{
	const Path path = mTestDirectory + "file-sharing";
	CreateFile(path, "unchanged");
	const FileAccessFlags readWrite = FileAccessFlag::Read | FileAccessFlag::Write;
	LoggingScope logScope(*this);
	logScope.IgnoreWarning("Failed to open file");
	logScope.IgnoreError("Failed to open file");
	logScope.IgnoreError("Failed to acquire");

	TShared<DataStream> firstReader = FileSystem::OpenFile(path);
	TShared<DataStream> secondReader = FileSystem::OpenFile(path);
	B3D_TEST_ASSERT(firstReader != nullptr && secondReader != nullptr);
	B3D_TEST_ASSERT(FileSystem::OpenFile(path, readWrite) == nullptr);
	firstReader = nullptr;
	B3D_TEST_ASSERT(FileSystem::OpenFile(path, readWrite) == nullptr);
	secondReader = nullptr;

	TShared<DataStream> writer = FileSystem::OpenFile(path, readWrite);
	B3D_TEST_ASSERT(writer != nullptr);
	if(writer == nullptr)
		return;
	B3D_TEST_ASSERT(FileSystem::OpenFile(path) == nullptr);
	B3D_TEST_ASSERT(FileSystem::OpenFile(path, readWrite) == nullptr);
	B3D_TEST_ASSERT(writer->Close());
	B3D_TEST_ASSERT(writer->Close());
	B3D_TEST_ASSERT(FileSystem::OpenFile(path) != nullptr);

	// Deliberately shared writers and readers must all opt into sharing.
	writer = FileSystem::OpenFile(path, readWrite | FileAccessFlag::Shared);
	TShared<DataStream> sharedReader = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Shared);
	TShared<DataStream> sharedWriter = FileSystem::OpenFile(path, readWrite | FileAccessFlag::Shared);
	B3D_TEST_ASSERT(writer != nullptr && sharedReader != nullptr && sharedWriter != nullptr);
	writer = nullptr;
	sharedReader = nullptr;
	sharedWriter = nullptr;
	B3D_TEST_ASSERT(FileSystem::OpenFile(path, readWrite) != nullptr);

	const Path missingPath = mTestDirectory + "sharing-failed-open";
	B3D_TEST_ASSERT(FileSystem::OpenFile(missingPath) == nullptr);
	B3D_TEST_ASSERT(FileSystem::CreateAndOpenFile(missingPath) != nullptr);
}

void FileSystemTestSuite::TestFileSharingUnderHandlePressure()
{
	const Path path = mTestDirectory + "sharing-protected";
	const Path otherPath = mTestDirectory + "sharing-other";
	const String content = "A live reader remains protected.";
	CreateFile(path, content);
	CreateFile(otherPath, "other");
	TShared<DataStream> reader = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(reader != nullptr);
	if(reader == nullptr)
		return;

	Vector<TShared<DataStream>> otherReaders;
	otherReaders.reserve(300);
	for(u32 readerIndex = 0; readerIndex < 300; ++readerIndex)
	{
		TShared<DataStream> otherReader = FileSystem::OpenFile(otherPath);
		B3D_TEST_ASSERT(otherReader != nullptr);
		if(otherReader == nullptr)
			return;
		otherReaders.push_back(std::move(otherReader));
	}

	{
		LoggingScope logScope(*this);
		logScope.ExpectWarning("Failed to open file");
		logScope.IgnoreError("Failed to open file");
		logScope.IgnoreError("Failed to acquire");
		B3D_TEST_ASSERT(FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Write) == nullptr);
	}

	TAsyncOp<TShared<MemoryDataStream>> read = reader->ReadAsync(0, content.size());
	read.BlockUntilComplete();
	const TShared<MemoryDataStream> data = read.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	if(data != nullptr)
		B3D_TEST_ASSERT(String((const char*)data->Data(), data->Size()) == content);
	reader = nullptr;
	B3D_TEST_ASSERT(FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Write) != nullptr);
}

void FileSystemTestSuite::TestConcurrentFileSharing()
{
	const Path path = mTestDirectory + "sharing-concurrent";
	CreateFile(path, "unchanged");
	LoggingScope logScope(*this);
	logScope.IgnoreWarning("Failed to open file");
	logScope.IgnoreError("Failed to open file");
	logScope.IgnoreError("Failed to acquire");

	for(u32 iteration = 0; iteration < 16; ++iteration)
	{
		std::atomic<bool> start(false);
		TShared<DataStream> writers[2];
		auto fnOpen = [&path, &start, &writers](u32 index)
		{
			while(!start.load())
				std::this_thread::yield();
			writers[index] = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Write);
		};
		std::thread first(fnOpen, 0);
		std::thread second(fnOpen, 1);
		start.store(true);
		first.join();
		second.join();
		B3D_TEST_ASSERT((writers[0] != nullptr) != (writers[1] != nullptr));
	}
}

void FileSystemTestSuite::TestExistsYesFile()
{
	Path path = mTestDirectory + "plop";
	CreateEmptyFile(path);
	B3D_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestExistsYesDir()
{
	Path path = mTestDirectory + "plop/";
	FileSystem::CreateFolder(path);
	B3D_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestExistsNo()
{
	B3D_TEST_ASSERT(!FileSystem::Exists(Path("this-file-does-not-exist")));
}

void FileSystemTestSuite::TestGetFileSizeZero()
{
	Path path = mTestDirectory + "file-size-test-1";
	CreateEmptyFile(path);
	B3D_TEST_ASSERT(FileSystem::GetFileSize(path) == 0);
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestGetFileSizeNotZero()
{
	Path path = mTestDirectory + "file-size-test-2";
	CreateFile(path, "0123456789");
	B3D_TEST_ASSERT(FileSystem::GetFileSize(path) == 10);
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestIsFileYes()
{
	Path path = mTestDirectory + "some-file-1";
	CreateEmptyFile(path);
	B3D_TEST_ASSERT(FileSystem::IsFile(path));
}

void FileSystemTestSuite::TestIsFileNo()
{
	Path path = mTestDirectory + "some-directory-1/";
	FileSystem::CreateFolder(path);
	B3D_TEST_ASSERT(!FileSystem::IsFile(path));
}

void FileSystemTestSuite::TestIsDirectoryYes()
{
	Path path = mTestDirectory + "some-directory-2/";
	FileSystem::CreateFolder(path);
	B3D_TEST_ASSERT(FileSystem::IsFolder(path));
}

void FileSystemTestSuite::TestIsDirectoryNo()
{
	Path path = mTestDirectory + "some-file-2";
	CreateEmptyFile(path);
	B3D_TEST_ASSERT(!FileSystem::IsFolder(path));
}

void FileSystemTestSuite::TestRemoveFile()
{
	Path path = mTestDirectory + "file-to-remove";
	CreateEmptyFile(path);
	B3D_TEST_ASSERT(FileSystem::Exists(path));
	B3D_TEST_ASSERT(FileSystem::Remove(path));
	B3D_TEST_ASSERT(!FileSystem::Exists(path));
}

void FileSystemTestSuite::TestRemoveDirectory()
{
	Path path = mTestDirectory + "directory-to-remove/";
	FileSystem::CreateFolder(path);
	B3D_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path, true);
	B3D_TEST_ASSERT(!FileSystem::Exists(path));
}

void FileSystemTestSuite::TestMove()
{
	Path source = mTestDirectory + "move-source-1";
	Path destination = mTestDirectory + "move-destination-1";
	CreateFile(source, "move-data-source-1");
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(!FileSystem::Exists(destination));
	FileSystem::Move(source, destination);
	B3D_TEST_ASSERT(!FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	B3D_TEST_ASSERT(ReadFile(destination) == "move-data-source-1");
}

void FileSystemTestSuite::TestMoveOverwriteExisting()
{
	Path source = mTestDirectory + "move-source-2";
	Path destination = mTestDirectory + "move-destination-2";
	CreateFile(source, "move-data-source-2");
	CreateFile(destination, "move-data-destination-2");
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	FileSystem::Move(source, destination, true);
	B3D_TEST_ASSERT(!FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	B3D_TEST_ASSERT(ReadFile(destination) == "move-data-source-2");
}

void FileSystemTestSuite::TestMoveNoOverwriteExisting()
{
	Path source = mTestDirectory + "move-source-3";
	Path destination = mTestDirectory + "move-destination-3";
	CreateFile(source, "move-data-source-3");
	CreateFile(destination, "move-data-destination-3");
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));

	LoggingScope logScope(*this);
	logScope.ExpectWarning("Move operation failed");
	FileSystem::Move(source, destination, false);

	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	B3D_TEST_ASSERT(ReadFile(destination) == "move-data-destination-3");
}

void FileSystemTestSuite::TestCopy()
{
	Path source = mTestDirectory + "copy-source-1";
	Path destination = mTestDirectory + "copy-destination-1";
	CreateFile(source, "copy-data-source-1");
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(!FileSystem::Exists(destination));
	FileSystem::Copy(source, destination);
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	B3D_TEST_ASSERT(ReadFile(source) == "copy-data-source-1");
	B3D_TEST_ASSERT(ReadFile(destination) == "copy-data-source-1");
}

void FileSystemTestSuite::TestCopyRecursive()
{
	// Build a small nested tree under the test directory, copy the whole thing, and verify every file landed at the
	// expected destination path with the expected contents. Exercises FileSystem::Copy's directory-recursion path.
	Path source = mTestDirectory + "copy-recursive-source/";
	Path destination = mTestDirectory + "copy-recursive-destination/";

	FileSystem::CreateFolder(source);
	FileSystem::CreateFolder(source + "sub/");
	FileSystem::CreateFolder(source + "sub/deeper/");
	CreateFile(source + "a.txt", "alpha");
	CreateFile(source + "b.txt", "bravo");
	CreateFile(source + "sub/c.txt", "charlie");
	CreateFile(source + "sub/deeper/d.txt", "delta");

	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(!FileSystem::Exists(destination));

	const bool ok = FileSystem::Copy(source, destination);
	B3D_TEST_ASSERT(ok);

	// Source must still be present (Copy, not Move) and every child file must exist at the mirrored destination path
	// with the correct content.
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::IsFolder(destination));
	B3D_TEST_ASSERT(FileSystem::IsFolder(destination + "sub/"));
	B3D_TEST_ASSERT(FileSystem::IsFolder(destination + "sub/deeper/"));
	B3D_TEST_ASSERT(ReadFile(destination + "a.txt") == "alpha");
	B3D_TEST_ASSERT(ReadFile(destination + "b.txt") == "bravo");
	B3D_TEST_ASSERT(ReadFile(destination + "sub/c.txt") == "charlie");
	B3D_TEST_ASSERT(ReadFile(destination + "sub/deeper/d.txt") == "delta");
}

void FileSystemTestSuite::TestCopyOverwriteExisting()
{
	Path source = mTestDirectory + "copy-source-2";
	Path destination = mTestDirectory + "copy-destination-2";
	CreateFile(source, "copy-data-source-2");
	CreateFile(destination, "copy-data-destination-2");
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	FileSystem::Copy(source, destination, true);
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	B3D_TEST_ASSERT(ReadFile(source) == "copy-data-source-2");
	B3D_TEST_ASSERT(ReadFile(destination) == "copy-data-source-2");
}

void FileSystemTestSuite::TestCopyNoOverwriteExisting()
{
	Path source = mTestDirectory + "copy-source-3";
	Path destination = mTestDirectory + "copy-destination-3";
	CreateFile(source, "copy-data-source-3");
	CreateFile(destination, "copy-data-destination-3");
	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));

	LoggingScope logScope(*this);
	logScope.ExpectWarning("Copy operation failed");
	FileSystem::Copy(source, destination, false);

	B3D_TEST_ASSERT(FileSystem::Exists(source));
	B3D_TEST_ASSERT(FileSystem::Exists(destination));
	B3D_TEST_ASSERT(ReadFile(source) == "copy-data-source-3");
	B3D_TEST_ASSERT(ReadFile(destination) == "copy-data-destination-3");
}

#define B3D_CONTAINS(v, e) (std::find(v.begin(), v.end(), e) != v.end())

void FileSystemTestSuite::TestGetChildren()
{
	Path path = mTestDirectory + "get-children-test/";
	FileSystem::CreateFolder(path);
	FileSystem::CreateFolder(path + "foo/");
	FileSystem::CreateFolder(path + "bar/");
	FileSystem::CreateFolder(path + "baz/");
	CreateEmptyFile(path + "ga");
	CreateEmptyFile(path + "bu");
	CreateEmptyFile(path + "zo");
	CreateEmptyFile(path + "meu");
	Vector<Path> files, directories;
	FileSystem::GetChildren(path, files, directories);
	B3D_TEST_ASSERT(files.size() == 4);
	B3D_TEST_ASSERT(B3D_CONTAINS(files, path + "ga"));
	B3D_TEST_ASSERT(B3D_CONTAINS(files, path + "bu"));
	B3D_TEST_ASSERT(B3D_CONTAINS(files, path + "zo"));
	B3D_TEST_ASSERT(B3D_CONTAINS(files, path + "meu"));
	B3D_TEST_ASSERT(directories.size() == 3);
	B3D_TEST_ASSERT(B3D_CONTAINS(directories, path + "foo"));
	B3D_TEST_ASSERT(B3D_CONTAINS(directories, path + "bar"));
	B3D_TEST_ASSERT(B3D_CONTAINS(directories, path + "baz"));
}

void FileSystemTestSuite::TestGetLastModifiedTime()
{
	std::time_t beforeTime;
	time(&beforeTime);

	Path path = mTestDirectory + "blah1234";
	CreateFile(path, "blah");
	std::time_t mtime = FileSystem::GetLastModifiedTime(path);
	B3D_TEST_ASSERT(mtime >= beforeTime);
	B3D_TEST_ASSERT(mtime <= beforeTime + 10);
}

void FileSystemTestSuite::TestGetTempDirectoryPath()
{
	Path path = FileSystem::GetTemporaryFolderPath();
	/* No judging. */
	B3D_TEST_ASSERT(!path.ToString().empty());
}

void FileSystemTestSuite::TestStreamWriteReadRoundtrip()
{
	Path path = mTestDirectory + "stream-roundtrip";
	const String content = "Hello, platform data stream!";

	{
		TShared<DataStream> out = FileSystem::CreateAndOpenFile(path);
		B3D_TEST_ASSERT(out != nullptr);
		const size_t written = out->Write(content.data(), content.size());
		B3D_TEST_ASSERT(written == content.size());
		out->Close();
	}

	{
		TShared<DataStream> in = FileSystem::OpenFile(path);
		B3D_TEST_ASSERT(in != nullptr);
		B3D_TEST_ASSERT(in->Size() == content.size());

		String buffer;
		buffer.resize(content.size());
		const size_t read = in->Read(&buffer[0], content.size());
		B3D_TEST_ASSERT(read == content.size());
		B3D_TEST_ASSERT(buffer == content);

		in->Seek(7);
		B3D_TEST_ASSERT(in->Tell() == 7);

		const size_t skipped = in->Skip(3);
		B3D_TEST_ASSERT(skipped == 3);
		B3D_TEST_ASSERT(in->Tell() == 10);

		String rest;
		rest.resize(content.size() - 10);
		in->Read(&rest[0], rest.size());
		B3D_TEST_ASSERT(rest == content.substr(10));

		in->Seek(content.size());
		u8 dummy = 0;
		const size_t readPastEnd = in->Read(&dummy, 1);
		B3D_TEST_ASSERT(readPastEnd == 0);
		B3D_TEST_ASSERT(in->Eof());

		in->Close();
	}

	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestMemoryStreamCustomDeleter()
{
	// The deleter must fire exactly once per owned buffer: on Close()/destruction of the owning stream, following the
	// ownership through moves, and never for copies (which get their own default-allocated buffer)
	u32 deleteCount = 0;
	const auto countingDeleter = [&deleteCount](void* memory) { deleteCount++; B3DFree(memory); };

	// Close() invokes the deleter; the destructor afterwards must not invoke it again
	{
		MemoryDataStream stream(B3DAllocate(16), 16, countingDeleter);
		stream.Close();
		B3D_TEST_ASSERT(deleteCount == 1);
	}
	B3D_TEST_ASSERT(deleteCount == 1);

	// Destructor alone invokes the deleter
	deleteCount = 0;
	{
		MemoryDataStream stream(B3DAllocate(16), 16, countingDeleter);
	}
	B3D_TEST_ASSERT(deleteCount == 1);

	// Move transfers the deleter: it fires once when the move target dies, not when the moved-from stream dies
	deleteCount = 0;
	{
		MemoryDataStream target;
		{
			MemoryDataStream source(B3DAllocate(16), 16, countingDeleter);
			target = std::move(source);
		}
		B3D_TEST_ASSERT(deleteCount == 0);
	}
	B3D_TEST_ASSERT(deleteCount == 1);

	// A copy gets its own default-allocated buffer and must not inherit the deleter
	deleteCount = 0;
	{
		MemoryDataStream copy;
		{
			MemoryDataStream source(B3DAllocate(16), 16, countingDeleter);
			source.Write("0123456789ABCDEF", 16);
			source.Seek(0);
			copy = source;
		}
		B3D_TEST_ASSERT(deleteCount == 1);

		char buffer[17] = {};
		B3D_TEST_ASSERT(copy.Read(buffer, 16) == 16);
		B3D_TEST_ASSERT(String(buffer) == "0123456789ABCDEF");
	}
	B3D_TEST_ASSERT(deleteCount == 1);

	// DisownMemory() discards the deleter along with ownership
	deleteCount = 0;
	void* disowned = nullptr;
	{
		MemoryDataStream stream(B3DAllocate(16), 16, countingDeleter);
		disowned = stream.DisownMemory();
	}
	B3D_TEST_ASSERT(deleteCount == 0);
	B3DFree(disowned);
}

void FileSystemTestSuite::TestMemoryStreamDeleterFixedCapacity()
{
	// A deleter-owned buffer can't be reallocated by the stream, so writes past the end must clamp (like a non-owning
	// stream) rather than grow the buffer
	u32 deleteCount = 0;
	{
		MemoryDataStream stream(B3DAllocate(8), 8, [&deleteCount](void* memory) { deleteCount++; B3DFree(memory); });

		const size_t written = stream.Write("0123456789ABCDEF", 16);
		B3D_TEST_ASSERT(written == 8);
		B3D_TEST_ASSERT(stream.Size() == 8);
		B3D_TEST_ASSERT(stream.Tell() == 8);

		// Further writes at the end produce nothing
		B3D_TEST_ASSERT(stream.Write("x", 1) == 0);

		stream.Seek(0);
		char buffer[9] = {};
		B3D_TEST_ASSERT(stream.Read(buffer, 8) == 8);
		B3D_TEST_ASSERT(String(buffer) == "01234567");
	}
	B3D_TEST_ASSERT(deleteCount == 1);
}

void FileSystemTestSuite::TestOpenFileMissing()
{
	Path path = mTestDirectory + "this-file-really-does-not-exist";

	LoggingScope logScope(*this);
	logScope.ExpectWarning("Failed to open file");

	TShared<DataStream> stream = FileSystem::OpenFile(path);
	B3D_TEST_ASSERT(stream == nullptr);
}

void FileSystemTestSuite::TestOpenFileAsyncRead()
{
	Path path = mTestDirectory + "async-read";
	const String content = "0123456789ABCDEFabcdef";
	CreateFile(path, content);

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);
	B3D_TEST_ASSERT(stream->Size() == content.size());

	TAsyncOp<TShared<MemoryDataStream>> op = stream->ReadAsync(0, content.size());
	op.BlockUntilComplete();

	TShared<MemoryDataStream> data = op.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	B3D_TEST_ASSERT(data->Size() == content.size());
	B3D_TEST_ASSERT(String((const char*)data->Data(), data->Size()) == content);

	// Read a slice from a non-zero offset.
	TAsyncOp<TShared<MemoryDataStream>> sliceOp = stream->ReadAsync(10, 6);
	sliceOp.BlockUntilComplete();

	TShared<MemoryDataStream> slice = sliceOp.GetReturnValue();
	B3D_TEST_ASSERT(slice != nullptr);
	B3D_TEST_ASSERT(slice->Size() == 6);
	B3D_TEST_ASSERT(String((const char*)slice->Data(), slice->Size()) == content.substr(10, 6));

	stream->Close();
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestOpenFileAsyncUserMemory()
{
	Path path = mTestDirectory + "async-usermem";
	const String content = "user-supplied-memory-test";
	CreateFile(path, content);

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);

	Vector<u8> buffer(content.size(), 0);
	DataRange range(buffer.data(), buffer.size());

	TAsyncOp<TShared<MemoryDataStream>> op = stream->ReadAsync(0, content.size(), range);
	op.BlockUntilComplete();

	TShared<MemoryDataStream> data = op.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	B3D_TEST_ASSERT(data->Size() == content.size());

	// The returned stream should wrap the caller-supplied memory, which should now hold the file contents.
	B3D_TEST_ASSERT(data->Data() == buffer.data());
	B3D_TEST_ASSERT(String((const char*)buffer.data(), content.size()) == content);

	stream->Close();
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestOpenFileAsyncEof()
{
	Path path = mTestDirectory + "async-eof";
	const String content = "short";
	CreateFile(path, content);

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);

	// Request more bytes than remain past the offset; should return only the available bytes.
	TAsyncOp<TShared<MemoryDataStream>> partialOp = stream->ReadAsync(3, 100);
	partialOp.BlockUntilComplete();

	TShared<MemoryDataStream> partial = partialOp.GetReturnValue();
	B3D_TEST_ASSERT(partial != nullptr);
	B3D_TEST_ASSERT(partial->Size() == content.size() - 3);
	B3D_TEST_ASSERT(String((const char*)partial->Data(), partial->Size()) == content.substr(3));

	// Read entirely past the end of file; should return an empty stream.
	TAsyncOp<TShared<MemoryDataStream>> eofOp = stream->ReadAsync(100, 10);
	eofOp.BlockUntilComplete();

	TShared<MemoryDataStream> eof = eofOp.GetReturnValue();
	B3D_TEST_ASSERT(eof != nullptr);
	B3D_TEST_ASSERT(eof->Size() == 0);

	stream->Close();
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestAsyncConcurrentReads()
{
	// Issue many ReadAsync calls back-to-back without blocking between them, then collect each result and verify the
	// slice content. On Linux this stresses the shared LinuxFileIOManager's MPSC submit path; on Win32 and macOS it
	// exercises the per-stream outstanding-read counter and concurrent completion-callback bookkeeping.
	Path path = mTestDirectory + "async-concurrent";

	constexpr size_t fileSize = 256 * 1024;
	Vector<u8> fileBytes(fileSize);
	for(size_t i = 0; i < fileSize; i++)
		fileBytes[i] = (u8)((i * 31u + 7u) & 0xff); // Cheap deterministic pattern.
	CreateBinaryFile(path, fileBytes.data(), fileBytes.size());

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);

	constexpr int numReads = 16;
	constexpr size_t sliceSize = fileSize / numReads;

	Vector<TAsyncOp<TShared<MemoryDataStream>>> ops;
	ops.reserve(numReads);
	for(int i = 0; i < numReads; i++)
		ops.push_back(stream->ReadAsync(i * sliceSize, sliceSize));

	for(int i = 0; i < numReads; i++)
	{
		ops[i].BlockUntilComplete();
		TShared<MemoryDataStream> data = ops[i].GetReturnValue();
		B3D_TEST_ASSERT(data != nullptr);
		B3D_TEST_ASSERT(data->Size() == sliceSize);

		const u8* bytes = (const u8*)data->Data();
		for(size_t j = 0; j < sliceSize; j++)
			B3D_TEST_ASSERT(bytes[j] == fileBytes[i * sliceSize + j]);
	}

	stream->Close();
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestAsyncCloseWhileInFlight()
{
	// Issue a handful of reads then immediately call Close() *before* blocking on any of them. Each op must finalize
	// (either with correct data or null if cancelled); the test fails only if Close hangs, an op never completes, or
	// completed data is corrupt. The reads are sized to give the kernel a fair chance of having work outstanding at
	// the moment Close runs, but the test stays meaningful even when every read happens to complete first.
	Path path = mTestDirectory + "async-close-in-flight";

	constexpr size_t fileSize = 1 * 1024 * 1024;
	Vector<u8> fileBytes(fileSize);
	for(size_t i = 0; i < fileSize; i++)
		fileBytes[i] = (u8)(i & 0xff);
	CreateBinaryFile(path, fileBytes.data(), fileBytes.size());

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);

	constexpr int numReads = 8;
	constexpr size_t readSize = fileSize / numReads;

	Vector<TAsyncOp<TShared<MemoryDataStream>>> ops;
	ops.reserve(numReads);
	for(int i = 0; i < numReads; i++)
		ops.push_back(stream->ReadAsync(i * readSize, readSize));

	// Close immediately, without blocking on any op. The contract is that Close drains every in-flight read; after
	// it returns, each op must have either finalized with correct data or finalized with a null result (cancelled).
	stream->Close();

	for(int i = 0; i < numReads; i++)
	{
		ops[i].BlockUntilComplete();
		TShared<MemoryDataStream> data = ops[i].GetReturnValue();
		if(data != nullptr)
		{
			B3D_TEST_ASSERT(data->Size() == readSize);
			const u8* bytes = (const u8*)data->Data();
			for(size_t j = 0; j < readSize; j++)
				B3D_TEST_ASSERT(bytes[j] == fileBytes[i * readSize + j]);
		}
		// Otherwise the read was cancelled by Close - also acceptable.
	}

	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestAsyncLargeFileChunkChaining()
{
	// Read a file larger than the Linux backend's 16 MiB per-chunk cap in a single ReadAsync call. The manager has to
	// chain at least two chunks for the request to complete. On Win32 and macOS the read is a single transfer (Win32
	// caps at 4 GiB, macOS GCD splits opaquely) but the test still validates that large reads return the right bytes.
	Path path = mTestDirectory + "async-large-chunk-chain";

	constexpr size_t fileSize = 24 * 1024 * 1024; // 24 MiB; exceeds the 16 MiB per-chunk cap on Linux.
	Vector<u8> fileBytes(fileSize);
	for(size_t i = 0; i < fileSize; i++)
		fileBytes[i] = (u8)((i * 131u + 11u) & 0xff);
	CreateBinaryFile(path, fileBytes.data(), fileBytes.size());

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);
	B3D_TEST_ASSERT(stream->Size() == fileSize);

	TAsyncOp<TShared<MemoryDataStream>> op = stream->ReadAsync(0, fileSize);
	op.BlockUntilComplete();

	TShared<MemoryDataStream> data = op.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	B3D_TEST_ASSERT(data->Size() == fileSize);

	// Compare a handful of byte windows rather than the whole file to keep the test cheap when it does pass; if the
	// chain is broken the boundaries are exactly where the failure would show up.
	const u8* bytes = (const u8*)data->Data();
	const size_t spotCheckOffsets[] = {0, 1, fileSize / 2 - 1, fileSize / 2, fileSize / 2 + 1, 16 * 1024 * 1024 - 1, 16 * 1024 * 1024, 16 * 1024 * 1024 + 1, fileSize - 2, fileSize - 1};
	for(size_t off : spotCheckOffsets)
		B3D_TEST_ASSERT(bytes[off] == fileBytes[off]);

	stream->Close();
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestAsyncFallbackWhenNotAsyncOpened()
{
	// Streams opened *without* FileAccessFlag::Async must still service ReadAsync calls by falling back to the
	// synchronous default DataStream::ReadAsync. The returned op completes inline with the correct data.
	Path path = mTestDirectory + "async-fallback";
	const String content = "fallback-path-via-sync-default";
	CreateFile(path, content);

	// No Async flag.
	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read);
	B3D_TEST_ASSERT(stream != nullptr);

	TAsyncOp<TShared<MemoryDataStream>> op = stream->ReadAsync(0, content.size());
	op.BlockUntilComplete();

	TShared<MemoryDataStream> data = op.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	B3D_TEST_ASSERT(data->Size() == content.size());
	B3D_TEST_ASSERT(String((const char*)data->Data(), data->Size()) == content);

	stream->Close();
	FileSystem::Remove(path);
}

void FileSystemTestSuite::TestAsyncEmptyFile()
{
	// ReadAsync on a zero-byte file must finalize cleanly with an empty MemoryDataStream rather than wedging or
	// allocating the requested-size buffer just to return zero bytes. Validates the at-EOF clamp-before-allocate path.
	Path path = mTestDirectory + "async-empty";
	CreateEmptyFile(path);

	TShared<DataStream> stream = FileSystem::OpenFile(path, FileAccessFlag::Read | FileAccessFlag::Async);
	B3D_TEST_ASSERT(stream != nullptr);
	B3D_TEST_ASSERT(stream->Size() == 0);

	TAsyncOp<TShared<MemoryDataStream>> op = stream->ReadAsync(0, 100);
	op.BlockUntilComplete();

	TShared<MemoryDataStream> data = op.GetReturnValue();
	B3D_TEST_ASSERT(data != nullptr);
	B3D_TEST_ASSERT(data->Size() == 0);

	stream->Close();
	FileSystem::Remove(path);
}
