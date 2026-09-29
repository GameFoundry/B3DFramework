//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDynamicLibrary.h"

using namespace b3d;

String DynamicLibrary::GetFileName(const StringView& baseName)
{
	// A bare base name never carries the prefix or extension, so normalizing it is equivalent to building it.
	return EnsureFileName(baseName);
}

String DynamicLibrary::EnsureFileName(const StringView& name)
{
	String fileName(name);

	// Append the extension (.dll, .dylib, .so, ...) unless it is already present.
	const String extension = String(".") + kExtension;
	const String::size_type extLength = extension.length();
	if (fileName.length() <= extLength || fileName.substr(fileName.length() - extLength) != extension)
		fileName.append(extension);

	// Prepend the platform prefix (e.g. "lib" on Unix) unless it is already present.
	if (kPrefix != nullptr)
	{
		const String prefix(kPrefix);
		const String::size_type filenameStart = fileName.find_last_of("/\\") + 1;
		if (fileName.compare(filenameStart, prefix.length(), prefix) != 0)
			fileName.insert(filenameStart, prefix);
	}

	return fileName;
}

DynamicLibrary::DynamicLibrary(String name)
	: mName(std::move(name))
{
	Load();
}

DynamicLibrary::~DynamicLibrary()
{
	Unload();
}
