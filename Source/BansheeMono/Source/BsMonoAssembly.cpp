//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsFileSystem.h"
#include "BsDataStream.h"
#include "BsException.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/mono-debug.h>

namespace bs
{
	size_t MonoAssembly::ClassId::Hash::operator()(const MonoAssembly::ClassId& v) const
	{
		size_t genInstanceAddr = (size_t)v.genericInstance;

		size_t seed = 0;
		hash_combine(seed, v.namespaceName);
		hash_combine(seed, v.name);
		hash_combine(seed, genInstanceAddr);

		return seed;
	}

	bool MonoAssembly::ClassId::Equals::operator()(const MonoAssembly::ClassId& a, const MonoAssembly::ClassId& b) const
	{
		return a.name == b.name && a.namespaceName == b.namespaceName && a.genericInstance == b.genericInstance;
	}

	MonoAssembly::ClassId::ClassId(const String& namespaceName, String name, ::MonoClass* genericInstance)
		:namespaceName(namespaceName), name(name), genericInstance(genericInstance)
	{

	}

	MonoAssembly::MonoAssembly(const WString& path, const String& name)
		: mName(name), mPath(path), mMonoImage(nullptr), mMonoAssembly(nullptr), mDebugData(nullptr), mIsLoaded(false)
		, mIsDependency(false), mHaveCachedClassList(false)
	{

	}

	MonoAssembly::~MonoAssembly()
	{
		unload();
	}

	void MonoAssembly::load(MonoDomain* domain)
	{
		if (mIsLoaded)
			unload();

		// Load assembly from memory because mono_domain_assembly_open keeps a lock on the file
		SPtr<DataStream> assemblyStream = FileSystem::openFile(mPath, true);
		if (assemblyStream == nullptr)
		{
			LOGERR("Cannot load assembly at path \"" + toString(mPath) + "\" because the file doesn't exist");
			return;
		}

		UINT32 assemblySize = (UINT32)assemblyStream->size();
		char* assemblyData = (char*)bs_stack_alloc(assemblySize);
		assemblyStream->read(assemblyData, assemblySize);

		String imageName = Path(mPath).getFilename();

		MonoImageOpenStatus status = MONO_IMAGE_OK;
		MonoImage* image = mono_image_open_from_data_with_name(assemblyData, assemblySize, true, &status, false, imageName.c_str());
		bs_stack_free(assemblyData);

		if (status != MONO_IMAGE_OK || image == nullptr)
		{
			LOGERR("Failed loading image data for assembly \"" + toString(mPath) + "\"");
			return;
		}

		// Load MDB file
#if BS_DEBUG_MODE
		Path mdbPath = mPath + L".mdb";
		if (FileSystem::exists(mdbPath))
		{
			SPtr<DataStream> mdbStream = FileSystem::openFile(mdbPath, true);

			if (mdbStream != nullptr)
			{
				UINT32 mdbSize = (UINT32)mdbStream->size();
				mDebugData = (UINT8*)bs_alloc(mdbSize);
				mdbStream->read(mDebugData, mdbSize);

				mono_debug_open_image_from_memory(image, mDebugData, mdbSize);
			}
		}
#endif

		mMonoAssembly = mono_assembly_load_from_full(image, imageName.c_str(), &status, false);
		if (status != MONO_IMAGE_OK || mMonoAssembly == nullptr)
		{
			LOGERR("Failed loading assembly \"" + toString(mPath) + "\"");
			return;
		}
		
		mMonoImage = image;
		if(mMonoImage == nullptr)
		{
			BS_EXCEPT(InvalidParametersException, "Cannot get script assembly image.");
		}

		mIsLoaded = true;
		mIsDependency = false;
	}

	void MonoAssembly::loadFromImage(MonoImage* image)
	{
		::MonoAssembly* monoAssembly = mono_image_get_assembly(image);
		if(monoAssembly == nullptr)
		{
			BS_EXCEPT(InvalidParametersException, "Cannot get assembly from image.");
		}

		mMonoAssembly = monoAssembly;
		mMonoImage = image;

		mIsLoaded = true;
		mIsDependency = true;
	}

	void MonoAssembly::unload()
	{
		if(!mIsLoaded)
			return;

		if(mDebugData != nullptr)
		{
			mono_debug_close_image(mMonoImage);

			bs_free(mDebugData);
			mDebugData = nullptr;
		}

		for(auto& entry : mClassesByRaw)
			bs_delete(entry.second);

		mClasses.clear();
		mClassesByRaw.clear();
		mCachedClassList.clear();

		if(mMonoImage != nullptr && !mIsDependency)
		{
			// Note: I used to call mono_image_close and mono_assembly_close here but those 
			// didn't seem to be necessary and were just crashing the program.
			mMonoImage = nullptr;
		}

		mIsLoaded = false;
		mIsDependency = false;
		mMonoAssembly = nullptr;
		mHaveCachedClassList = false;
	}

	void MonoAssembly::invoke(const String& functionName)
	{
		MonoMethodDesc* methodDesc = mono_method_desc_new(functionName.c_str(), false);

		if(methodDesc != nullptr)
		{
			::MonoMethod* entry = mono_method_desc_search_in_image(methodDesc, mMonoImage);

			if(entry != nullptr)
			{
				MonoObject* exception = nullptr;
				mono_runtime_invoke(entry, nullptr, nullptr, &exception);

				MonoUtil::throwIfException(exception);
			}
		}
	}

	MonoClass* MonoAssembly::getClass(const String& namespaceName, const String& name) const
	{
		if(!mIsLoaded)
			BS_EXCEPT(InvalidStateException, "Trying to use an unloaded assembly.");

		MonoAssembly::ClassId classId(namespaceName, name);
		auto iterFind = mClasses.find(classId);

		if(iterFind != mClasses.end())
			return iterFind->second;

		::MonoClass* monoClass = mono_class_from_name(mMonoImage, namespaceName.c_str(), name.c_str());
		if(monoClass == nullptr)
			return nullptr;

		MonoClass* newClass = new (bs_alloc<MonoClass>()) MonoClass(namespaceName, name, monoClass, this);
		mClasses[classId] = newClass;
		mClassesByRaw[monoClass] = newClass;

		return newClass;
	}

	MonoClass* MonoAssembly::getClass(::MonoClass* rawMonoClass) const
	{
		if(!mIsLoaded)
			BS_EXCEPT(InvalidStateException, "Trying to use an unloaded assembly.");

		if(rawMonoClass == nullptr)
			return nullptr;

		auto iterFind = mClassesByRaw.find(rawMonoClass);

		if(iterFind != mClassesByRaw.end())
			return iterFind->second;

		String ns;
		String typeName;
		MonoUtil::getClassName(rawMonoClass, ns, typeName);

		MonoClass* newClass = new (bs_alloc<MonoClass>()) MonoClass(ns, typeName, rawMonoClass, this);
		
		mClassesByRaw[rawMonoClass] = newClass;

		if(!isGenericClass(typeName)) // No point in referencing generic types by name as all instances share it
		{
			MonoAssembly::ClassId classId(ns, typeName);
			mClasses[classId] = newClass;
		}

		return newClass;
	}

	MonoClass* MonoAssembly::getClass(const String& ns, const String& typeName, ::MonoClass* rawMonoClass) const
	{
		if (!mIsLoaded)
			BS_EXCEPT(InvalidStateException, "Trying to use an unloaded assembly.");

		if (rawMonoClass == nullptr)
			return nullptr;

		auto iterFind = mClassesByRaw.find(rawMonoClass);

		if (iterFind != mClassesByRaw.end())
			return iterFind->second;

		MonoClass* newClass = new (bs_alloc<MonoClass>()) MonoClass(ns, typeName, rawMonoClass, this);

		mClassesByRaw[rawMonoClass] = newClass;

		if (!isGenericClass(typeName)) // No point in referencing generic types by name as all instances share it
		{
			MonoAssembly::ClassId classId(ns, typeName);
			mClasses[classId] = newClass;
		}

		return newClass;
	}

	const Vector<MonoClass*>& MonoAssembly::getAllClasses() const
	{
		if(mHaveCachedClassList)
			return mCachedClassList;
		
		mCachedClassList.clear();
		Stack<MonoClass*> todo;

		int numRows = mono_image_get_table_rows (mMonoImage, MONO_TABLE_TYPEDEF);

		for(int i = 1; i < numRows; i++) // Skip Module
		{
			::MonoClass* monoClass = mono_class_get (mMonoImage, (i + 1) | MONO_TOKEN_TYPE_DEF);

			String ns;
			String type;
			MonoUtil::getClassName(monoClass, ns, type);

			MonoClass* curClass = getClass(ns, type);

			if (curClass != nullptr)
			{
				// Get nested types if it has any
				todo.push(curClass);
				while (!todo.empty())
				{
					MonoClass* curNestedClass = todo.top();
					todo.pop();

					void* iter = nullptr;
					do
					{
						::MonoClass* rawNestedClass = rawNestedClass = mono_class_get_nested_types(curNestedClass->_getInternalClass(), &iter);
						if (rawNestedClass == nullptr)
							break;

						String nestedType = curNestedClass->getTypeName() + "+" + mono_class_get_name(rawNestedClass);

						MonoClass* nestedClass = getClass(ns, nestedType, rawNestedClass);
						if (nestedClass != nullptr)
						{
							mCachedClassList.push_back(nestedClass);
							todo.push(nestedClass);
						}

					} while (true);					
				}

				mCachedClassList.push_back(curClass);
			}
		}

		mHaveCachedClassList = true;

		return mCachedClassList;
	}

	bool MonoAssembly::isGenericClass(const String& name) const
	{
		// By CIL convention generic classes have ` separating their name and
		// number of generic parameters
		auto iterFind = std::find(name.rbegin(), name.rend(), '`');

		return iterFind != name.rend();
	}
}