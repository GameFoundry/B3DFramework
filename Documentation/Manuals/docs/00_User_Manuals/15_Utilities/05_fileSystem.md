---
title: File system
---

# Paths
Instead of using strings for representing paths, framework uses the @b3d::Path class. Aside from containing the path it provides a variety of other useful information and allows for path manipulation. It is recommended to always store paths using **Path** instead of strings.

~~~~~~~~~~~~~{.cpp}
Path myPath = "C:/Path/To/File.txt";
~~~~~~~~~~~~~

Some of the things you can do with a **Path**:
 - Retrieve the filename using @b3d::Path::GetFilename
 - Retrieve the filename extension using @b3d::Path::GetExtension
 - Get last element of path, either file or directory using @b3d::Path::GetTail
 - Iterate over directories, get drive, combine paths, convert relative to absolute paths and vice versa, and more. See the API reference for a complete list.

For example:
~~~~~~~~~~~~~{.cpp}
Path myPath = "C:/Path/To/File.txt";

String filename = myPath.GetFilename(); // Returns filename, if the path has any
myPath.SetExtension(".jpg"); // Path is now "C:/Path/To/File.jpg"
myPath.MakeRelative("C:/Path"); // Path is now "To/File.jpg"

Path firstPath("C:/Path/To/");
Path secondPath("File.txt");
Path combinedPath = firstPath + secondPath; // Path is now "C:/Path/To/File.txt"
~~~~~~~~~~~~~

**Path** can always be converted back to a string by calling @b3d::Path::ToString.

~~~~~~~~~~~~~{.cpp}
Path path("C:/Path/To/");
String pathString = path.ToString();
~~~~~~~~~~~~~

When setting paths be careful with setting backslashes or slashes at the end of the path. Path with a no backslash/slash on the end will be interpreted as a file path, and path with a backslash/slash will be interpreted as a folder path. For example:
 - "C:/MyFolder" - "MyFolder" interpreted as a file, **Path::GetFilename()** returns "MyFolder"
 - "C:/MyFolder/" - "MyFolder" interpreted as a folder, **Path::GetFilename()** returns an empty string
 
# File system
File system operations like opening, creating, deleting, moving, copying files/folders are provided by the @b3d::FileSystem class. Check the API reference for a complete list of operations.

An example creating a folder and a file:
~~~~~~~~~~~~~{.cpp}
FileSystem::CreateFolder("C:/Path/To/");

SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile("C:/Path/To/File.txt");
// Write to data stream (see below)
~~~~~~~~~~~~~

# Data streams
If you create or open a file you will receive a @b3d::DataStream object. Data streams allow you to easily write to, or read from open files.

~~~~~~~~~~~~~{.cpp}
SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile("C:/Path/To/File.txt");

// Write some string data
fileStream->WriteString("Writing to a file");

// Write some binary data
u8* dataBuffer = B3DAllocate(1024);

// ... fill up the buffer with some data ...

fileStream->Write(dataBuffer, 1024);
fileStream->Close();

B3DFree(dataBuffer);
~~~~~~~~~~~~~

Once you are done with a stream make sure to close it by calling @b3d::DataStream::Close. Stream will also be automatically closed when it goes out of scope.

Streams don't need to be read or written to sequentially, use @b3d::DataStream::Seek to move within any position of the stream, and @b3d::DataStream::Tell to find out the current position.

~~~~~~~~~~~~~{.cpp}
// Open the file we wrote in the previous example
SPtr<DataStream> fileStream = FileSystem::OpenFile("C:/Path/To/File.txt");

// Seek past the string we wrote
String writtenString = "Writing to a file";
fileStream->Seek(writtenString.size());

// Read the byte data
u8* dataBuffer = B3DAllocate(1024);
fileStream->Read(dataBuffer, 1024);

fileStream->Close();
B3DFree(dataBuffer);
~~~~~~~~~~~~~

Each time you read or write from the stream, the current read/write indices will advance. So subsequent calls to read/write will continue from the last position that was read/written.

Finally, use @b3d::DataStream::Size to find out the size of a stream in bytes.
