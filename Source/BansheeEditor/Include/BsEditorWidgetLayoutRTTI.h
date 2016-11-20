//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorWidgetLayout.h"
#include "BsDockManagerLayout.h"
#include "BsRTTIType.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class EditorWidgetLayoutRTTI : public RTTIType<EditorWidgetLayout, IReflectable, EditorWidgetLayoutRTTI>
	{
	private:
		EditorWidgetLayout::Entry& getEntry(EditorWidgetLayout* obj, UINT32 idx) { return obj->mEntries[idx]; }
		void setEntry(EditorWidgetLayout* obj, UINT32 idx, EditorWidgetLayout::Entry& val) { obj->mEntries[idx] = val; } 
		UINT32 getEntriesArraySize(EditorWidgetLayout* obj) { return (UINT32)obj->mEntries.size(); }
		void setEntriesArraySize(EditorWidgetLayout* obj, UINT32 size) { obj->mEntries.resize(size); }

		SPtr<DockManagerLayout> getDockLayout(EditorWidgetLayout* obj) { return obj->mDockLayout; }
		void setDockLayout(EditorWidgetLayout* obj, SPtr<DockManagerLayout> val) { obj->mDockLayout = val; }

		bool& getIsMainWindowMaximized(EditorWidgetLayout* obj) { return obj->mMaximized; }
		void setIsMainWindowMaximized(EditorWidgetLayout* obj, bool& val) { obj->mMaximized = val; }

	public:
		EditorWidgetLayoutRTTI()
		{
			addPlainArrayField("mRootEntry", 0, &EditorWidgetLayoutRTTI::getEntry, &EditorWidgetLayoutRTTI::getEntriesArraySize, 
				&EditorWidgetLayoutRTTI::setEntry, &EditorWidgetLayoutRTTI::setEntriesArraySize);

			addReflectablePtrField("mDockLayout", 1, &EditorWidgetLayoutRTTI::getDockLayout, &EditorWidgetLayoutRTTI::setDockLayout);
			addPlainField("mMaximized", 2, &EditorWidgetLayoutRTTI::getIsMainWindowMaximized, &EditorWidgetLayoutRTTI::setIsMainWindowMaximized);
		}

		const String& getRTTIName() override
		{
			static String name = "EditorWidgetLayout";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_EditorWidgetLayout;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<EditorWidgetLayout>(EditorWidgetLayout::PrivatelyConstruct());
		}
	};

	template<> struct RTTIPlainType<bs::EditorWidgetLayout::Entry>
	{	
		enum { id = bs::TID_EditorWidgetLayoutEntry }; enum { hasDynamicSize = 1 };

		static void toMemory(const bs::EditorWidgetLayout::Entry& data, char* memory)
		{ 
			UINT32 size = 0;
			char* memoryStart = memory;
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			memory = rttiWriteElem(data.widgetNames, memory, size);
			memory = rttiWriteElem(data.isDocked, memory, size);
			memory = rttiWriteElem(data.x, memory, size);
			memory = rttiWriteElem(data.y, memory, size);
			memory = rttiWriteElem(data.width, memory, size);
			memory = rttiWriteElem(data.height, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		static UINT32 fromMemory(bs::EditorWidgetLayout::Entry& data, char* memory)
		{ 
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.widgetNames, memory);
			memory = rttiReadElem(data.isDocked, memory);
			memory = rttiReadElem(data.x, memory);
			memory = rttiReadElem(data.y, memory);
			memory = rttiReadElem(data.width, memory);
			memory = rttiReadElem(data.height, memory);

			return size;
		}

		static UINT32 getDynamicSize(const bs::EditorWidgetLayout::Entry& data)	
		{ 
			UINT64 dataSize = sizeof(UINT32) + rttiGetElemSize(data.widgetNames) + rttiGetElemSize(data.isDocked) + 
				rttiGetElemSize(data.x) + rttiGetElemSize(data.y) + rttiGetElemSize(data.width) + rttiGetElemSize(data.height);

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