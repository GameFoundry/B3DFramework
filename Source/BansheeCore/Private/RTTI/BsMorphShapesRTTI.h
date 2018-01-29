//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Animation/BsMorphShapes.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT MorphShapeRTTI : public RTTIType <MorphShape, IReflectable, MorphShapeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mName, 0)
			BS_RTTI_MEMBER_PLAIN(mWeight, 1)
			BS_RTTI_MEMBER_PLAIN_ARRAY(mVertices, 2)
		BS_END_RTTI_MEMBERS
		
	public:
		MorphShapeRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "MorphShape";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MorphShape;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<MorphShape>();
		}
	};

	class BS_CORE_EXPORT MorphChannelRTTI : public RTTIType <MorphChannel, IReflectable, MorphChannelRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mName, 0)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mShapes, 1)
		BS_END_RTTI_MEMBERS

	public:
		MorphChannelRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "MorphChannel";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MorphChannel;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return MorphChannel::createEmpty();
		}
	};

	class BS_CORE_EXPORT MorphShapesRTTI : public RTTIType <MorphShapes, IReflectable, MorphShapesRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mChannels, 0)
			BS_RTTI_MEMBER_PLAIN(mNumVertices, 1)
		BS_END_RTTI_MEMBERS

	public:
		MorphShapesRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "MorphShapes";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MorphShapes;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return MorphShapes::createEmpty();
		}
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(MorphVertex);

	/** @} */
	/** @endcond */
}