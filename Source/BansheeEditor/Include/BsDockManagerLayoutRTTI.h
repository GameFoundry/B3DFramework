//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsDockManagerLayout.h"
#include "BsRTTIType.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class DockManagerLayoutRTTI : public RTTIType<DockManagerLayout, IReflectable, DockManagerLayoutRTTI>
	{
	private:
		DockManagerLayout::Entry& getRootEntry(DockManagerLayout* obj) { return obj->mRootEntry; }
		void setRootEntry(DockManagerLayout* obj, DockManagerLayout::Entry& val) { obj->mRootEntry = val; } 

		bool& getIsMaximized(DockManagerLayout* obj) { return obj->mIsMaximized; }
		void setIsMaximized(DockManagerLayout* obj, bool& val) { obj->mIsMaximized = val; }

		Vector<String>& getMaximizedWidgetNames(DockManagerLayout* obj) { return obj->mMaximizedWidgetNames; }
		void setMaximizedWidgetNames(DockManagerLayout* obj, Vector<String>& val) { obj->mMaximizedWidgetNames = val; }

	public:
		DockManagerLayoutRTTI()
		{
			addPlainField("mRootEntry", 0, &DockManagerLayoutRTTI::getRootEntry, &DockManagerLayoutRTTI::setRootEntry);
			addPlainField("mIsMaximized", 1, &DockManagerLayoutRTTI::getIsMaximized, &DockManagerLayoutRTTI::setIsMaximized);
			addPlainField("mMaximizedWidgetNames", 2, &DockManagerLayoutRTTI::getMaximizedWidgetNames, &DockManagerLayoutRTTI::setMaximizedWidgetNames);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			DockManagerLayout* layout = static_cast<DockManagerLayout*>(obj);

			Stack<DockManagerLayout::Entry*> todo;
			todo.push(&layout->mRootEntry);

			while (!todo.empty())
			{
				DockManagerLayout::Entry* current = todo.top();
				todo.pop();

				if (!current->isLeaf)
				{
					current->children[0]->parent = current;
					current->children[1]->parent = current;

					todo.push(current->children[0]);
					todo.push(current->children[1]);
				}
			}
		}

		const String& getRTTIName() override
		{
			static String name = "DockManagerLayout";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_DockManagerLayout;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<DockManagerLayout>();
		}
	};

	template<> struct RTTIPlainType<BansheeEngine::DockManagerLayout::Entry>
	{	
		enum { id = BansheeEngine::TID_DockManagerLayoutEntry }; enum { hasDynamicSize = 1 };

		static void toMemory(const BansheeEngine::DockManagerLayout::Entry& data, char* memory)
		{ 
			UINT32 size = 0;
			char* memoryStart = memory;
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			memory = rttiWriteElem(data.isLeaf, memory, size);
			memory = rttiWriteElem(data.horizontalSplit, memory, size);
			memory = rttiWriteElem(data.splitPosition, memory, size);
			memory = rttiWriteElem(data.widgetNames, memory, size);

			if(!data.isLeaf)
			{
				memory = rttiWriteElem(*data.children[0], memory, size);
				memory = rttiWriteElem(*data.children[1], memory, size);
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		static UINT32 fromMemory(BansheeEngine::DockManagerLayout::Entry& data, char* memory)
		{ 
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.isLeaf, memory);
			memory = rttiReadElem(data.horizontalSplit, memory);
			memory = rttiReadElem(data.splitPosition, memory);
			memory = rttiReadElem(data.widgetNames, memory);

			if(!data.isLeaf)
			{
				data.children[0] = bs_new<BansheeEngine::DockManagerLayout::Entry>();
				data.children[1] = bs_new<BansheeEngine::DockManagerLayout::Entry>();

				memory = rttiReadElem(*data.children[0], memory);
				memory = rttiReadElem(*data.children[1], memory);
				
				data.children[0]->parent = &data;
				data.children[1]->parent = &data;
			}

			return size;
		}

		static UINT32 getDynamicSize(const BansheeEngine::DockManagerLayout::Entry& data)	
		{ 
			UINT64 dataSize = sizeof(UINT32) + rttiGetElemSize(data.isLeaf) + rttiGetElemSize(data.horizontalSplit) + 
				rttiGetElemSize(data.splitPosition) + rttiGetElemSize(data.widgetNames);

			if(!data.isLeaf)
			{
				dataSize += rttiGetElemSize(*data.children[0]);
				dataSize += rttiGetElemSize(*data.children[1]);
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