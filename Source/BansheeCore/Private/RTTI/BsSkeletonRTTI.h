//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Animation/BsSkeleton.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT SkeletonRTTI : public RTTIType <Skeleton, IReflectable, SkeletonRTTI>
	{
	private:
		Matrix4& getBindPose(Skeleton* obj, UINT32 idx) { return obj->mInvBindPoses[idx]; }
		void setBindPose(Skeleton* obj, UINT32 idx, Matrix4& value) { obj->mInvBindPoses[idx] = value; }

		void setNumBindPoses(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;
			
			assert(obj->mInvBindPoses == nullptr);
			obj->mInvBindPoses = bs_newN<Matrix4>(size);
		}

		SkeletonBoneInfo& getBoneInfo(Skeleton* obj, UINT32 idx) { return obj->mBoneInfo[idx]; }
		void setBoneInfo(Skeleton* obj, UINT32 idx, SkeletonBoneInfo& value) { obj->mBoneInfo[idx] = value; }

		void setNumBoneInfos(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;

			assert(obj->mBoneInfo == nullptr);
			obj->mBoneInfo = bs_newN<SkeletonBoneInfo>(size);
		}

		Matrix4& getBoneTransform(Skeleton* obj, UINT32 idx) { return obj->mBoneTransforms[idx]; }
		void setBoneTransform(Skeleton* obj, UINT32 idx, Matrix4& value) { obj->mBoneTransforms[idx] = value; }

		void setNumBoneTransforms(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;
			
			assert(obj->mBoneTransforms == nullptr);
			obj->mBoneTransforms = bs_newN<Matrix4>(size);
		}

		UINT32 getNumBones(Skeleton* obj) { return obj->mNumBones; }
	public:
		SkeletonRTTI()
		{
			addPlainArrayField("bindPoses", 0, &SkeletonRTTI::getBindPose, &SkeletonRTTI::getNumBones,
				&SkeletonRTTI::setBindPose, &SkeletonRTTI::setNumBindPoses);
			addPlainArrayField("boneInfo", 1, &SkeletonRTTI::getBoneInfo, &SkeletonRTTI::getNumBones,
				&SkeletonRTTI::setBoneInfo, &SkeletonRTTI::setNumBoneInfos);
			addPlainArrayField("boneTransforms", 2, &SkeletonRTTI::getBoneTransform, &SkeletonRTTI::getNumBones,
				&SkeletonRTTI::setBoneTransform, &SkeletonRTTI::setNumBoneTransforms);
		}

		const String& getRTTIName() override
		{
			static String name = "Skeleton";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Skeleton;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Skeleton::createEmpty();
		}
	};

	template<> struct RTTIPlainType<SkeletonBoneInfo>
	{
		enum { id = TID_SkeletonBoneInfo }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const SkeletonBoneInfo& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.name, memory, size);
			memory = rttiWriteElem(data.parent, memory, size);

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(SkeletonBoneInfo& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.parent, memory);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const SkeletonBoneInfo& data)
		{
			UINT64 dataSize = sizeof(UINT32);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.parent);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}