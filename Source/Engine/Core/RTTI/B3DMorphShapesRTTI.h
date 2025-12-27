//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "Animation/B3DMorphShapes.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT MorphShapeRTTI : public TRTTIType<MorphShape, IReflectable, MorphShapeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mName, 0)
			B3D_RTTI_MEMBER(mWeight, 1)
			B3D_RTTI_MEMBER_CONTAINER(mVertices, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "MorphShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MorphShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MorphShape>();
		}
	};

	class B3D_EXPORT MorphChannelRTTI : public TRTTIType<MorphChannel, IReflectable, MorphChannelRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mName, 0)
			B3D_RTTI_MEMBER_CONTAINER(mShapes, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MorphChannel";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MorphChannel;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return MorphChannel::CreateEmpty();
		}
	};

	class B3D_EXPORT MorphShapesRTTI : public TRTTIType<MorphShapes, IReflectable, MorphShapesRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mChannels, 0)
			B3D_RTTI_MEMBER(mVertexCount, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MorphShapes";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MorphShapes;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return MorphShapes::CreateEmpty();
		}
	};

	B3D_ALLOW_MEMCPY_SERIALIZATION(MorphVertex, TID_MorphVertex);

	/** @} */
	/** @endcond */
} // namespace b3d
