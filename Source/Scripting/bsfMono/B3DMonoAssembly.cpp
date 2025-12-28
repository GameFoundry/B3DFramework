//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoAssembly.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "B3DMonoUtil.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Error/B3DException.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/image.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#endif

namespace b3d {
size_t MonoAssembly::ClassId::Hash::operator()(const MonoAssembly::ClassId& v) const
{
	size_t genInstanceAddr = (size_t)v.GenericInstance;

	size_t seed = 0;
	B3DCombineHash(seed, v.NamespaceName);
	B3DCombineHash(seed, v.Name);
	B3DCombineHash(seed, genInstanceAddr);

	return seed;
}

bool MonoAssembly::ClassId::Equals::operator()(const MonoAssembly::ClassId& a, const MonoAssembly::ClassId& b) const
{
	return a.Name == b.Name && a.NamespaceName == b.NamespaceName && a.GenericInstance == b.GenericInstance;
}

MonoAssembly::ClassId::ClassId(const String& namespaceName, String name, ::MonoClass* genericInstance)
	: NamespaceName(namespaceName), Name(name), GenericInstance(genericInstance)
{
}

MonoAssembly::MonoAssembly(const Path& path, const String& name)
	: mName(name), mPath(path), mMonoImage(nullptr), mMonoAssembly(nullptr), mDebugData(nullptr), mIsLoaded(false), mIsDependency(false), mHaveCachedClassList(false)
{
}

MonoAssembly::~MonoAssembly()
{
	Unload();
}

void MonoAssembly::Load()
{
	if(mIsLoaded)
		Unload();

	// Load assembly from memory because mono_domain_assembly_open keeps a lock on the file
	SPtr<DataStream> assemblyStream = FileSystem::OpenFile(mPath, true);
	if(assemblyStream == nullptr)
	{
		B3D_LOG(Error, LogScript, "Cannot load assembly at path \"{0}\" because the file doesn't exist", mPath);
		return;
	}

	u32 assemblySize = (u32)assemblyStream->Size();
	char* assemblyData = (char*)B3DStackAllocate(assemblySize);
	assemblyStream->Read(assemblyData, assemblySize);

	String imageName = mPath.GetFilename();

	MonoImageOpenStatus status = MONO_IMAGE_OK;
	MonoImage* image = mono_image_open_from_data_with_name(assemblyData, assemblySize, true, &status, false, imageName.c_str());
	B3DStackFree(assemblyData);

	if(status != MONO_IMAGE_OK || image == nullptr)
	{
		B3D_LOG(Error, LogScript, "Failed loading image data for assembly \"{0}\"", mPath);
		return;
	}

	// Load MDB file
#if B3D_DEBUG
	Path mdbPath = mPath;
	mdbPath.SetExtension(mdbPath.GetExtension() + ".mdb");

	if(FileSystem::Exists(mdbPath))
	{
		SPtr<DataStream> mdbStream = FileSystem::OpenFile(mdbPath, true);

		if(mdbStream != nullptr)
		{
			u32 mdbSize = (u32)mdbStream->Size();
			mDebugData = (u8*)B3DAllocate(mdbSize);
			mdbStream->Read(mDebugData, mdbSize);

			mono_debug_open_image_from_memory(image, mDebugData, mdbSize);
		}
	}
#endif

	mMonoAssembly = mono_assembly_load_from_full(image, imageName.c_str(), &status, false);
	if(status != MONO_IMAGE_OK || mMonoAssembly == nullptr)
	{
		B3D_LOG(Error, LogScript, "Failed loading assembly \"{0}\"", mPath);
		return;
	}

	mMonoImage = image;
	if(mMonoImage == nullptr)
	{
		B3D_EXCEPT(InvalidParametersException, "Cannot get script assembly image.");
	}

	mIsLoaded = true;
	mIsDependency = false;
}

void MonoAssembly::LoadFromImage(MonoImage* image)
{
	::MonoAssembly* monoAssembly = mono_image_get_assembly(image);
	if(monoAssembly == nullptr)
	{
		B3D_EXCEPT(InvalidParametersException, "Cannot get assembly from image.");
	}

	mMonoAssembly = monoAssembly;
	mMonoImage = image;

	mIsLoaded = true;
	mIsDependency = true;
}

void MonoAssembly::Unload()
{
	if(!mIsLoaded)
		return;

	for(auto& entry : mClassesByRaw)
		B3DDelete(entry.second);

	mClasses.clear();
	mClassesByRaw.clear();
	mCachedClassList.clear();
	mHaveCachedClassList = false;

	if(!mIsDependency)
	{
		if(mDebugData != nullptr)
		{
			mono_debug_close_image(mMonoImage);

			B3DFree(mDebugData);
			mDebugData = nullptr;
		}

		if(mMonoImage != nullptr)
		{
			// Make sure to close the image, otherwise when we try to re-load this assembly the Mono will return the cached
			// image
			mono_image_close(mMonoImage);
			mMonoImage = nullptr;
		}

		mMonoAssembly = nullptr;
		mIsLoaded = false;
	}
}

void MonoAssembly::Invoke(const String& functionName)
{
	MonoMethodDesc* methodDesc = mono_method_desc_new(functionName.c_str(), false);

	if(methodDesc != nullptr)
	{
		::MonoMethod* entry = mono_method_desc_search_in_image(methodDesc, mMonoImage);

		if(entry != nullptr)
		{
			MonoObject* exception = nullptr;
			mono_runtime_invoke(entry, nullptr, nullptr, &exception);

			MonoUtil::ThrowIfException(exception);
		}
	}
}

MonoClass* MonoAssembly::GetClass(const String& namespaceName, const String& name) const
{
	if(!mIsLoaded)
		B3D_EXCEPT(InvalidStateException, "Trying to use an unloaded assembly.");

	MonoAssembly::ClassId classId(namespaceName, name);
	auto iterFind = mClasses.find(classId);

	if(iterFind != mClasses.end())
		return iterFind->second;

	::MonoClass* monoClass = mono_class_from_name(mMonoImage, namespaceName.c_str(), name.c_str());
	if(monoClass == nullptr)
		return nullptr;

	MonoClass* newClass = new(B3DAllocate<MonoClass>()) MonoClass(namespaceName, name, monoClass, this);
	mClasses[classId] = newClass;
	mClassesByRaw[monoClass] = newClass;

	return newClass;
}

MonoClass* MonoAssembly::GetClass(::MonoClass* rawMonoClass) const
{
	if(!mIsLoaded)
		B3D_EXCEPT(InvalidStateException, "Trying to use an unloaded assembly.");

	if(rawMonoClass == nullptr)
		return nullptr;

	auto iterFind = mClassesByRaw.find(rawMonoClass);

	if(iterFind != mClassesByRaw.end())
		return iterFind->second;

	String ns;
	String typeName;
	MonoUtil::GetClassName(rawMonoClass, ns, typeName);

	// Verify the class is actually part of this assembly
	MonoImage* classImage = mono_class_get_image(rawMonoClass);
	if(classImage != mMonoImage)
		return nullptr;

	MonoClass* newClass = new(B3DAllocate<MonoClass>()) MonoClass(ns, typeName, rawMonoClass, this);
	mClassesByRaw[rawMonoClass] = newClass;

	MonoAssembly::ClassId classId(ns, typeName);
	mClasses[classId] = newClass;

	return newClass;
}

MonoClass* MonoAssembly::GetClass(const String& ns, const String& typeName, ::MonoClass* rawMonoClass) const
{
	if(!mIsLoaded)
		B3D_EXCEPT(InvalidStateException, "Trying to use an unloaded assembly.");

	if(rawMonoClass == nullptr)
		return nullptr;

	auto iterFind = mClassesByRaw.find(rawMonoClass);

	if(iterFind != mClassesByRaw.end())
		return iterFind->second;

	MonoClass* newClass = new(B3DAllocate<MonoClass>()) MonoClass(ns, typeName, rawMonoClass, this);

	mClassesByRaw[rawMonoClass] = newClass;

	if(!IsGenericClass(typeName)) // No point in referencing generic types by name as all instances share it
	{
		MonoAssembly::ClassId classId(ns, typeName);
		mClasses[classId] = newClass;
	}

	return newClass;
}

const Vector<MonoClass*>& MonoAssembly::GetAllClasses() const
{
	if(mHaveCachedClassList)
		return mCachedClassList;

	mCachedClassList.clear();
	Stack<MonoClass*> todo;

	MonoAssembly* corlib = MonoManager::Instance().GetAssembly("corlib");
	MonoClass* compilerGeneratedAttrib = corlib->GetClass("System.Runtime.CompilerServices", "CompilerGeneratedAttribute");

	int numRows = mono_image_get_table_rows(mMonoImage, MONO_TABLE_TYPEDEF);

	for(int i = 1; i < numRows; i++) // Skip Module
	{
		::MonoClass* monoClass = mono_class_get(mMonoImage, (i + 1) | MONO_TOKEN_TYPE_DEF);

		String ns;
		String type;
		MonoUtil::GetClassName(monoClass, ns, type);

		MonoClass* curClass = GetClass(ns, type);
		if(curClass != nullptr)
		{
			// Skip compiler generates classes
			if(curClass->HasAttribute(compilerGeneratedAttrib))
				continue;

			// Get nested types if it has any
			todo.push(curClass);
			while(!todo.empty())
			{
				MonoClass* curNestedClass = todo.top();
				todo.pop();

				void* iter = nullptr;
				do
				{
					::MonoClass* rawNestedClass = mono_class_get_nested_types(curNestedClass->GetInternalClass(), &iter);
					if(rawNestedClass == nullptr)
						break;

					String nestedType = curNestedClass->GetTypeName() + "+" + mono_class_get_name(rawNestedClass);

					MonoClass* nestedClass = GetClass(ns, nestedType, rawNestedClass);
					if(nestedClass != nullptr)
					{
						// Skip compiler generated classes
						if(nestedClass->HasAttribute(compilerGeneratedAttrib))
							continue;

						mCachedClassList.push_back(nestedClass);
						todo.push(nestedClass);
					}
				}
				while(true);
			}

			mCachedClassList.push_back(curClass);
		}
	}

	mHaveCachedClassList = true;

	return mCachedClassList;
}

bool MonoAssembly::IsGenericClass(const String& name) const
{
	// By CIL convention generic classes have ` separating their name and
	// number of generic parameters
	auto iterFind = std::find(name.rbegin(), name.rend(), '`');

	return iterFind != name.rend();
}
} // namespace b3d
