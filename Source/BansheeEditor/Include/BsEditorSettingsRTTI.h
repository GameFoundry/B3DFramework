//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorSettings.h"
#include "BsRTTIType.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Editor
	 *  @{
	 */

	class EditorSettingsRTTI : public RTTIType <EditorSettings, Settings, EditorSettingsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mMoveSnapActive, 0)
			BS_RTTI_MEMBER_PLAIN(mRotateSnapActive, 1)

			BS_RTTI_MEMBER_PLAIN(mMoveSnap, 2)
			BS_RTTI_MEMBER_PLAIN(mRotationSnap, 3)

			BS_RTTI_MEMBER_PLAIN(mGridSize, 4)
			BS_RTTI_MEMBER_PLAIN(mGridAxisSpacing, 5)

			BS_RTTI_MEMBER_PLAIN(mActiveSceneTool, 6)
			BS_RTTI_MEMBER_PLAIN(mActiveCoordinateMode, 7)
			BS_RTTI_MEMBER_PLAIN(mActivePivotMode, 8)

			BS_RTTI_MEMBER_PLAIN(mHandleSize, 9)

			BS_RTTI_MEMBER_PLAIN(mLastOpenProject, 10)
			BS_RTTI_MEMBER_PLAIN(mAutoLoadLastProject, 11)
			BS_RTTI_MEMBER_PLAIN(mRecentProjects, 12)

			BS_RTTI_MEMBER_PLAIN(mFPSLimit, 13)
			BS_RTTI_MEMBER_PLAIN(mMouseSensitivity, 14)
		BS_END_RTTI_MEMBERS
	public:
		EditorSettingsRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "EditorSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_EditorSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<EditorSettings>();
		}
	};

	template<> struct RTTIPlainType <RecentProject>
	{
		enum { id = TID_RecentProject }; enum { hasDynamicSize = 1 };

		static void toMemory(const RecentProject& data, char* memory)
		{
			UINT32 size = 0;
			char* memoryStart = memory;
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			memory = rttiWriteElem(data.path, memory, size);
			memory = rttiWriteElem(data.accessTimestamp, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		static UINT32 fromMemory(RecentProject& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.path, memory);
			memory = rttiReadElem(data.accessTimestamp, memory);

			return size;
		}

		static UINT32 getDynamicSize(const RecentProject& data)
		{
			UINT64 dataSize = sizeof(UINT32) + rttiGetElemSize(data.path) + rttiGetElemSize(data.accessTimestamp);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<UINT32>::max())
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