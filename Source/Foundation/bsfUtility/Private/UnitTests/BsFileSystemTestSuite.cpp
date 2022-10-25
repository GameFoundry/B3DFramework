//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/UnitTests/BsFileSystemTestSuite.h"

#include "Debug/BsDebug.h"
#include "Error/BsException.h"
#include "FileSystem/BsFileSystem.h"

#include <algorithm>
#include <fstream>

using namespace bs;

const String testDirectoryName = "FileSystemTestDirectory/";

void createFile(Path path, String content)
{
	std::ofstream fs;
	fs.open(path.toPlatformString().c_str());
	fs << content;
	fs.close();
}

void createEmptyFile(Path path)
{
	createFile(path, "");
}

String readFile(Path path)
{
	String content;
	std::ifstream fs;
	fs.open(path.toPlatformString().c_str());
	fs >> content;
	fs.close();
	return content;
}

void FileSystemTestSuite::StartUp()
{
	mTestDirectory = FileSystem::GetWorkingDirectoryPath() + testDirectoryName;
	if(FileSystem::Exists(mTestDirectory))
	{
		BS_EXCEPT(InternalErrorException, String("Directory '") + testDirectoryName + "' should not already exist; you should remove it manually.");
	}
	else
	{
		FileSystem::CreateDir(mTestDirectory);
		BS_TEST_ASSERT_MSG(FileSystem::Exists(mTestDirectory), "FileSystemTestSuite::StartUp(): test directory creation failed");
	}
}

void FileSystemTestSuite::ShutDown()
{
	FileSystem::Remove(mTestDirectory, true);
	if(FileSystem::Exists(mTestDirectory))
	{
		LOGERR("FileSystemTestSuite failed to delete '" + mTestDirectory.toString() + "', you should remove it manually.");
	}
}

FileSystemTestSuite::FileSystemTestSuite()
{
	BS_ADD_TEST(FileSystemTestSuite::testExists_yes_file);
	BS_ADD_TEST(FileSystemTestSuite::testExists_yes_dir);
	BS_ADD_TEST(FileSystemTestSuite::testExists_no);
	BS_ADD_TEST(FileSystemTestSuite::testGetFileSize_zero);
	BS_ADD_TEST(FileSystemTestSuite::testGetFileSize_not_zero);
	BS_ADD_TEST(FileSystemTestSuite::testIsFile_yes);
	BS_ADD_TEST(FileSystemTestSuite::testIsFile_no);
	BS_ADD_TEST(FileSystemTestSuite::testIsDirectory_yes);
	BS_ADD_TEST(FileSystemTestSuite::testIsDirectory_no);
	BS_ADD_TEST(FileSystemTestSuite::testRemove_file);
	BS_ADD_TEST(FileSystemTestSuite::testRemove_directory);
	BS_ADD_TEST(FileSystemTestSuite::testMove);
	BS_ADD_TEST(FileSystemTestSuite::testMove_overwrite_existing);
	BS_ADD_TEST(FileSystemTestSuite::testMove_no_overwrite_existing);
	BS_ADD_TEST(FileSystemTestSuite::testCopy);
	BS_ADD_TEST(FileSystemTestSuite::testCopy_overwrite_existing);
	BS_ADD_TEST(FileSystemTestSuite::testCopy_no_overwrite_existing);
	BS_ADD_TEST(FileSystemTestSuite::testGetChildren);
	BS_ADD_TEST(FileSystemTestSuite::testGetLastModifiedTime);
	BS_ADD_TEST(FileSystemTestSuite::testGetTempDirectoryPath);
}

void FileSystemTestSuite::testExists_yes_file()
{
	Path path = mTestDirectory + "plop";
	createEmptyFile(path);
	BS_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path);
}

void FileSystemTestSuite::testExists_yes_dir()
{
	Path path = mTestDirectory + "plop/";
	FileSystem::CreateDir(path);
	BS_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path);
}

void FileSystemTestSuite::testExists_no()
{
	BS_TEST_ASSERT(!FileSystem::Exists(Path("this-file-does-not-exist")));
}

void FileSystemTestSuite::testGetFileSize_zero()
{
	Path path = mTestDirectory + "file-size-test-1";
	createEmptyFile(path);
	BS_TEST_ASSERT(FileSystem::GetFileSize(path) == 0);
	FileSystem::Remove(path);
}

void FileSystemTestSuite::testGetFileSize_not_zero()
{
	Path path = mTestDirectory + "file-size-test-2";
	createFile(path, "0123456789");
	BS_TEST_ASSERT(FileSystem::GetFileSize(path) == 10);
	FileSystem::Remove(path);
}

void FileSystemTestSuite::testIsFile_yes()
{
	Path path = mTestDirectory + "some-file-1";
	createEmptyFile(path);
	BS_TEST_ASSERT(FileSystem::IsFile(path));
}

void FileSystemTestSuite::testIsFile_no()
{
	Path path = mTestDirectory + "some-directory-1/";
	FileSystem::CreateDir(path);
	BS_TEST_ASSERT(!FileSystem::IsFile(path));
}

void FileSystemTestSuite::testIsDirectory_yes()
{
	Path path = mTestDirectory + "some-directory-2/";
	FileSystem::CreateDir(path);
	BS_TEST_ASSERT(FileSystem::IsDirectory(path));
}

void FileSystemTestSuite::testIsDirectory_no()
{
	Path path = mTestDirectory + "some-file-2";
	createEmptyFile(path);
	BS_TEST_ASSERT(!FileSystem::IsDirectory(path));
}

void FileSystemTestSuite::testRemove_file()
{
	Path path = mTestDirectory + "file-to-remove";
	createEmptyFile(path);
	BS_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path);
	BS_TEST_ASSERT(!FileSystem::Exists(path));
}

void FileSystemTestSuite::testRemove_directory()
{
	Path path = mTestDirectory + "directory-to-remove/";
	FileSystem::CreateDir(path);
	BS_TEST_ASSERT(FileSystem::Exists(path));
	FileSystem::Remove(path, true);
	BS_TEST_ASSERT(!FileSystem::Exists(path));
}

void FileSystemTestSuite::testMove()
{
	Path source = mTestDirectory + "move-source-1";
	Path destination = mTestDirectory + "move-destination-1";
	createFile(source, "move-data-source-1");
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(!FileSystem::Exists(destination));
	FileSystem::Move(source, destination);
	BS_TEST_ASSERT(!FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	BS_TEST_ASSERT(readFile(destination) == "move-data-source-1");
}

void FileSystemTestSuite::testMove_overwrite_existing()
{
	Path source = mTestDirectory + "move-source-2";
	Path destination = mTestDirectory + "move-destination-2";
	createFile(source, "move-data-source-2");
	createFile(destination, "move-data-destination-2");
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	FileSystem::Move(source, destination, true);
	BS_TEST_ASSERT(!FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	BS_TEST_ASSERT(readFile(destination) == "move-data-source-2");
}

void FileSystemTestSuite::testMove_no_overwrite_existing()
{
	Path source = mTestDirectory + "move-source-3";
	Path destination = mTestDirectory + "move-destination-3";
	createFile(source, "move-data-source-3");
	createFile(destination, "move-data-destination-3");
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	FileSystem::Move(source, destination, false);
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	BS_TEST_ASSERT(readFile(destination) == "move-data-destination-3");
}

void FileSystemTestSuite::testCopy()
{
	Path source = mTestDirectory + "copy-source-1";
	Path destination = mTestDirectory + "copy-destination-1";
	createFile(source, "copy-data-source-1");
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(!FileSystem::Exists(destination));
	FileSystem::Copy(source, destination);
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	BS_TEST_ASSERT(readFile(source) == "copy-data-source-1");
	BS_TEST_ASSERT(readFile(destination) == "copy-data-source-1");
}

void FileSystemTestSuite::testCopy_overwrite_existing()
{
	Path source = mTestDirectory + "copy-source-2";
	Path destination = mTestDirectory + "copy-destination-2";
	createFile(source, "copy-data-source-2");
	createFile(destination, "copy-data-destination-2");
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	FileSystem::Copy(source, destination, true);
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	BS_TEST_ASSERT(readFile(source) == "copy-data-source-2");
	BS_TEST_ASSERT(readFile(destination) == "copy-data-source-2");
}

void FileSystemTestSuite::testCopy_no_overwrite_existing()
{
	Path source = mTestDirectory + "copy-source-3";
	Path destination = mTestDirectory + "copy-destination-3";
	createFile(source, "copy-data-source-3");
	createFile(destination, "copy-data-destination-3");
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	FileSystem::Copy(source, destination, false);
	BS_TEST_ASSERT(FileSystem::Exists(source));
	BS_TEST_ASSERT(FileSystem::Exists(destination));
	BS_TEST_ASSERT(readFile(source) == "copy-data-source-3");
	BS_TEST_ASSERT(readFile(destination) == "copy-data-destination-3");
}

#define CONTAINS(v, e) (std::find(v.begin(), v.end(), e) != v.end())

void FileSystemTestSuite::testGetChildren()
{
	Path path = mTestDirectory + "get-children-test/";
	FileSystem::CreateDir(path);
	FileSystem::CreateDir(path + "foo/");
	FileSystem::CreateDir(path + "bar/");
	FileSystem::CreateDir(path + "baz/");
	createEmptyFile(path + "ga");
	createEmptyFile(path + "bu");
	createEmptyFile(path + "zo");
	createEmptyFile(path + "meu");
	Vector<Path> files, directories;
	FileSystem::GetChildren(path, files, directories);
	BS_TEST_ASSERT(files.size() == 4);
	BS_TEST_ASSERT(CONTAINS(files, path + "ga"));
	BS_TEST_ASSERT(CONTAINS(files, path + "bu"));
	BS_TEST_ASSERT(CONTAINS(files, path + "zo"));
	BS_TEST_ASSERT(CONTAINS(files, path + "meu"));
	BS_TEST_ASSERT(directories.size() == 3);
	BS_TEST_ASSERT(CONTAINS(directories, path + "foo"));
	BS_TEST_ASSERT(CONTAINS(directories, path + "bar"));
	BS_TEST_ASSERT(CONTAINS(directories, path + "baz"));
}

void FileSystemTestSuite::testGetLastModifiedTime()
{
	std::time_t beforeTime;
	time(&beforeTime);

	Path path = mTestDirectory + "blah1234";
	createFile(path, "blah");
	std::time_t mtime = FileSystem::GetLastModifiedTime(path);
	BS_TEST_ASSERT(mtime >= beforeTime);
	BS_TEST_ASSERT(mtime <= beforeTime + 10);
}

void FileSystemTestSuite::testGetTempDirectoryPath()
{
	Path path = FileSystem::GetTempDirectoryPath();
	/* No judging. */
	BS_TEST_ASSERT(!path.toString().empty());
}
