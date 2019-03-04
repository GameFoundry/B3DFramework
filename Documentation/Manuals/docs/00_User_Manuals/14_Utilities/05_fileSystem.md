---
title: File system
---

# Paths
Instead of using strings for representing paths, bs::f uses the @bs::Path class. Aside from containing the path it provides a variety of other useful information and allows for path manipulation. It is recommended to always store paths using **Path** instead of strings.

~~~~~~~~~~~~~{.cpp}
Path myPath = "C:/Path/To/File.txt";
~~~~~~~~~~~~~

Some of the things you can do with a **Path**:
 - Retrieve the filename using @bs::Path::getFilename
 - Retrieve the filename extension using @bs::Path::getExtension
 - Get last element of path, either file or directory using @bs::Path::getTail
 - Iterate over directories, get drive, combine paths, convert relative to absolute paths and vice versa, and more. See the API reference for a complete list.
 
For example:
~~~~~~~~~~~~~{.cpp}
Path myPath = "C:/Path/To/File.txt";

String filename = myPath.getFilename(); // Returns filename, if the path has any
myPath.setExtension(".jpg"); // Path is now "C:/Path/To/File.jpg"
myPath.makeRelative("C:/Path"); // Path is now "To/File.jpg"

Path a("C:/Path/To/");
Path b("File.txt");
Path combined = a + b; // // Path is now "C:/Path/To/File.txt"
~~~~~~~~~~~~~

**Path** can always be converted back to a string by calling @bs::Path::toString.

~~~~~~~~~~~~~{.cpp}
Path path("C:/Path/To/");
String str = path.toString();
~~~~~~~~~~~~~
 
When setting paths be careful with setting backslashes or slashes at the end of the path. Path with a no backslash/slash on the end will be interpreted as a file path, and path with a backslash/slash will be interpreted as a folder path. For example:
 - "C:/MyFolder" - "MyFolder" interpreted as a file, **Path::getFilename()** returns "MyFolder"
 - "C:/MyFolder/" - "MyFolder" interpreted as a folder, **Path::getFilename()** returns an empty string
 
# File system
File system operations like opening, creating, deleting, moving, copying files/folders are provided by the @bs::FileSystem class. Check the API reference for a complete list of operations.

An example creating a folder and a file:
~~~~~~~~~~~~~{.cpp}
FileSystem::createDir("C:/Path/To/");

SPtr<DataStream> fileStream = FileSystem::createAndOpenFile("C:/Path/To/File.txt");
// Write to data stream (see below)
~~~~~~~~~~~~~

# Data streams
If you create or open a file you will receive a @bs::DataStream object. Data streams allow you to easily write to, or read from open files. 

~~~~~~~~~~~~~{.cpp}
SPtr<DataStream> fileStream = FileSystem::createAndOpenFile("C:/Path/To/File.txt");

// Write some string data
fileStream->writeString("Writing to a file");

// Write some binary data
UINT8* myBuffer = bs_alloc(1024);

// ... fill up the buffer with some data ...

fileStream->write(myBuffer, 1024);
fileStream->close();

bs_free(myBuffer);
~~~~~~~~~~~~~

Once you are done with a stream make sure to close it by calling @bs::DataStream::close. Stream will also be automatically closed when it goes out of scope.

Streams don't need to be read or written to sequentially, use @bs::DataStream::seek to move within any position of the stream, and @bs::DataStream::tell to find out the current position.

~~~~~~~~~~~~~{.cpp}
// Open the file we wrote in the previous example
SPtr<DataStream> fileStream = FileSystem::openFile("C:/Path/To/File.txt");

// Seek past the string we wrote
String str = "Writing to a file";
fileStream->seek(str.size());

// Read the byte data
UINT8* myBuffer = bs_alloc(1024);
fileStream->read(myBuffer, 1024);

fileStream->close();
bs_free(myBuffer);
~~~~~~~~~~~~~

Each time you read or write from the stream, the current read/write indices will advance. So subsequent calls to read/write will continue from the last position that was read/written.

Finally, use @bs::DataStream::size to find out the size of a stream in bytes.
