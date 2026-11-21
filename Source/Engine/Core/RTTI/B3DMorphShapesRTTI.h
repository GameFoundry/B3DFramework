//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "Animation/B3DMorphShapes.h"
#include "RTTI/B3DStdRTTI.h"

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
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MorphShape";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MorphShape;
		}

		TShared<IReflectable> NewRttiObject() override
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

		TShared<IReflectable> NewRttiObject() override
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

		TShared<IReflectable> NewRttiObject() override
		{
			return MorphShapes::CreateEmpty();
		}
	};

	class B3D_EXPORT MorphShapeDeltasRTTI : public TRTTIType<MorphShapeDeltas, IReflectable, MorphShapeDeltasRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mVertexRanges, 0)
			B3D_RTTI_MEMBER(mDeltas, 1)
			B3D_RTTI_MEMBER(mShapeCount, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MorphShapeDeltas";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MorphShapeDeltas;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeSharedFromExisting(new(B3DAllocate<MorphShapeDeltas>()) MorphShapeDeltas());
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
