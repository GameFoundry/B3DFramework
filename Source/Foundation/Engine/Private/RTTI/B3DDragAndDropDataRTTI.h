//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "GUI/B3DDragAndDrop.h"
#include "RTTI/B3DPathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT DragAndDropDataRTTI : public TRTTIType<DragAndDropData, IReflectable, DragAndDropDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "DragAndDropData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_DragAndDropData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<DragAndDropData>();
		}
	};

	class B3D_EXPORT SceneObjectDragAndDropDataRTTI : public TRTTIType<SceneObjectDragAndDropData, DragAndDropData, SceneObjectDragAndDropDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(SceneObjects, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "SceneObjectDragAndDropData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SceneObjectDragAndDropData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SceneObjectDragAndDropData>();
		}
	};

	class B3D_EXPORT ResourceDragAndDropDataRTTI : public TRTTIType<ResourceDragAndDropData, DragAndDropData, ResourceDragAndDropDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(RelativeResourcePaths, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ResourceDragAndDropData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ResourceDragAndDropData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ResourceDragAndDropData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
