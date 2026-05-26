//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFileSystemTestSuite.h"

#include "Debug/B3DDebug.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"

#include <algorithm>
#include <fstream>

using namespace b3d;

const String kTestDirectoryName = "FileSystemTestDirectory/";

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
	mTestDirectory = FileSystem::GetExecutableFolderPath() + kTestDirectoryName;
	if(FileSystem::Exists(mTestDirectory))
	{
		if(!B3D_ENSURE_LOG(false, "Directory '{0}' should not already exist; you should remove it manually.", kTestDirectoryName))
			return;
	}
	else
	{
		FileSystem::CreateFolder(mTestDirectory);
		B3D_TEST_ASSERT_MSG(FileSystem::Exists(mTestDirectory), "FileSystemTestSuite::StartUp(): test directory creation failed");
	}
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
	B3D_ADD_TEST(FileSystemTestSuite::TestCopyOverwriteExisting);
	B3D_ADD_TEST(FileSystemTestSuite::TestCopyNoOverwriteExisting);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetChildren);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetLastModifiedTime);
	B3D_ADD_TEST(FileSystemTestSuite::TestGetTempDirectoryPath);
	B3D_ADD_TEST(FileSystemTestSuite::TestStreamWriteReadRoundtrip);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileMissing);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileAsyncRead);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileAsyncUserMemory);
	B3D_ADD_TEST(FileSystemTestSuite::TestOpenFileAsyncEof);
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
	FileSystem::Remove(path);
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
