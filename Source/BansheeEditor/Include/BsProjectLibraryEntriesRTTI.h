//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsRTTIType.h"
#include "BsProjectLibraryEntries.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class ProjectLibraryEntriesRTTI : public RTTIType<ProjectLibraryEntries, IReflectable, ProjectLibraryEntriesRTTI>
	{
	private:
		ProjectLibrary::DirectoryEntry& getRootElement(ProjectLibraryEntries* obj) { return obj->mRootEntry; }
		void setRootElement(ProjectLibraryEntries* obj, ProjectLibrary::DirectoryEntry& val) { obj->mRootEntry = val; } 

	public:
		ProjectLibraryEntriesRTTI()
		{
			addPlainField("mRootElement", 0, &ProjectLibraryEntriesRTTI::getRootElement, &ProjectLibraryEntriesRTTI::setRootElement);
		}

		virtual const String& getRTTIName()
		{
			static String name = "ProjectLibraryEntries";
			return name;
		}

		virtual UINT32 getRTTIId()
		{
			return TID_ProjectLibraryEntries;
		}

		virtual SPtr<IReflectable> newRTTIObject()
		{
			return ProjectLibraryEntries::createEmpty();
		}
	};

	template<> struct RTTIPlainType<bs::ProjectLibrary::FileEntry>
	{	
		enum { id = bs::TID_ProjectLibraryResEntry }; enum { hasDynamicSize = 1 };

		static void toMemory(const bs::ProjectLibrary::FileEntry& data, char* memory)
		{ 
			UINT32 size = 0;
			char* memoryStart = memory;
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			UINT32 type = (UINT32)data.type;
			memory = rttiWriteElem(type, memory, size);
			memory = rttiWriteElem(data.path, memory, size);
			memory = rttiWriteElem(data.elementName, memory, size);
			memory = rttiWriteElem(data.lastUpdateTime, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		static UINT32 fromMemory(bs::ProjectLibrary::FileEntry& data, char* memory)
		{ 
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 type;
			memory = rttiReadElem(type, memory);
			data.type = (bs::ProjectLibrary::LibraryEntryType)type;

			memory = rttiReadElem(data.path, memory);
			memory = rttiReadElem(data.elementName, memory);
			memory = rttiReadElem(data.lastUpdateTime, memory);

			return size;
		}

		static UINT32 getDynamicSize(const bs::ProjectLibrary::FileEntry& data)	
		{ 
			UINT64 dataSize = sizeof(UINT32) + rttiGetElemSize(data.type) + rttiGetElemSize(data.path) + rttiGetElemSize(data.elementName) +
				rttiGetElemSize(data.lastUpdateTime);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	template<> struct RTTIPlainType<bs::ProjectLibrary::DirectoryEntry>
	{	
		enum { id = bs::TID_ProjectLibraryDirEntry }; enum { hasDynamicSize = 1 };

		static void toMemory(const bs::ProjectLibrary::DirectoryEntry& data, char* memory)
		{ 
			UINT32 size = 0;
			char* memoryStart = memory;
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			memory = rttiWriteElem(data.type, memory, size);
			memory = rttiWriteElem(data.path, memory, size);
			memory = rttiWriteElem(data.elementName, memory, size);

			UINT32 numChildren = (UINT32)data.mChildren.size();
			memory = rttiWriteElem(numChildren, memory, size);

			for(auto& child : data.mChildren)
			{
				if(child->type == bs::ProjectLibrary::LibraryEntryType::File)
				{
					bs::ProjectLibrary::FileEntry* childResEntry = static_cast<bs::ProjectLibrary::FileEntry*>(child);
					memory = rttiWriteElem(*childResEntry, memory, size);
				}
				else if(child->type == bs::ProjectLibrary::LibraryEntryType::Directory)
				{
					bs::ProjectLibrary::DirectoryEntry* childDirEntry = static_cast<bs::ProjectLibrary::DirectoryEntry*>(child);
					memory = rttiWriteElem(*childDirEntry, memory, size);
				}
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		static UINT32 fromMemory(bs::ProjectLibrary::DirectoryEntry& data, char* memory)
		{ 
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.type, memory);
			memory = rttiReadElem(data.path, memory);
			memory = rttiReadElem(data.elementName, memory);

			UINT32 numChildren = 0;
			memory = rttiReadElem(numChildren, memory);

			for(UINT32 i = 0; i < numChildren; i++)
			{
				bs::ProjectLibrary::LibraryEntryType childType = bs::ProjectLibrary::LibraryEntryType::File;
				rttiReadElem(childType, memory + sizeof(UINT32)); // Skip ahead to get type

				if(childType == bs::ProjectLibrary::LibraryEntryType::File)
				{
					bs::ProjectLibrary::FileEntry* childResEntry = bs_new<bs::ProjectLibrary::FileEntry>(); // Note: Assumes that ProjectLibrary takes care of the cleanup
					memory = rttiReadElem(*childResEntry, memory);

					childResEntry->parent = &data;
					data.mChildren.push_back(childResEntry);
				}
				else if(childType == bs::ProjectLibrary::LibraryEntryType::Directory)
				{
					bs::ProjectLibrary::DirectoryEntry* childDirEntry = bs_new<bs::ProjectLibrary::DirectoryEntry>(); // Note: Assumes that ProjectLibrary takes care of the cleanup
					memory = rttiReadElem(*childDirEntry, memory);

					childDirEntry->parent = &data;
					data.mChildren.push_back(childDirEntry);
				}
			}

			return size;
		}

		static UINT32 getDynamicSize(const bs::ProjectLibrary::DirectoryEntry& data)	
		{ 
			UINT64 dataSize = sizeof(UINT32) + rttiGetElemSize(data.type) + rttiGetElemSize(data.path) + rttiGetElemSize(data.elementName);

			dataSize += sizeof(UINT32);

			for(auto& child : data.mChildren)
			{
				if(child->type == bs::ProjectLibrary::LibraryEntryType::File)
				{
					bs::ProjectLibrary::FileEntry* childResEntry = static_cast<bs::ProjectLibrary::FileEntry*>(child);
					dataSize += rttiGetElemSize(*childResEntry);
				}
				else if(child->type == bs::ProjectLibrary::LibraryEntryType::Directory)
				{
					bs::ProjectLibrary::DirectoryEntry* childDirEntry = static_cast<bs::ProjectLibrary::DirectoryEntry*>(child);
					dataSize += rttiGetElemSize(*childDirEntry);
				}
			}

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	/** @} */
	/** @endcond */
}