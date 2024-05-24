//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "Animation/BsMorphShapes.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT MorphShapeRTTI : public RTTIType<MorphShape, IReflectable, MorphShapeRTTI>
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

	class B3D_CORE_EXPORT MorphChannelRTTI : public RTTIType<MorphChannel, IReflectable, MorphChannelRTTI>
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

	class B3D_CORE_EXPORT MorphShapesRTTI : public RTTIType<MorphShapes, IReflectable, MorphShapesRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mChannels, 0)
			B3D_RTTI_MEMBER(mNumVertices, 1)
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

	B3D_ALLOW_MEMCPY_SERIALIZATION(MorphVertex);

	/** @} */
	/** @endcond */
} // namespace bs
