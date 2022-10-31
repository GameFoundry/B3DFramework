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

	class BS_CORE_EXPORT MorphShapeRTTI : public RTTIType<MorphShape, IReflectable, MorphShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mName, 0)
			BS_RTTI_MEMBER_PLAIN(mWeight, 1)
			BS_RTTI_MEMBER_PLAIN_ARRAY(mVertices, 2)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "MorphShape";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_MorphShape;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MorphShape>();
		}
	};

	class BS_CORE_EXPORT MorphChannelRTTI : public RTTIType<MorphChannel, IReflectable, MorphChannelRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mName, 0)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mShapes, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MorphChannel";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_MorphChannel;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return MorphChannel::CreateEmpty();
		}
	};

	class BS_CORE_EXPORT MorphShapesRTTI : public RTTIType<MorphShapes, IReflectable, MorphShapesRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mChannels, 0)
			BS_RTTI_MEMBER_PLAIN(mNumVertices, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MorphShapes";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_MorphShapes;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return MorphShapes::CreateEmpty();
		}
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(MorphVertex);

	/** @} */
	/** @endcond */
} // namespace bs
