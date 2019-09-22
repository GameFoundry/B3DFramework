//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
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

		Transform& getBoneTransform(Skeleton* obj, UINT32 idx) { return obj->mBoneTransforms[idx]; }
		void setBoneTransform(Skeleton* obj, UINT32 idx, Transform& value) { obj->mBoneTransforms[idx] = value; }

		void setNumBoneTransforms(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;

			assert(obj->mBoneTransforms == nullptr);
			obj->mBoneTransforms = bs_newN<Transform>(size);
		}

		UINT32 getNumBones(Skeleton* obj) { return obj->mNumBones; }
	public:
		SkeletonRTTI()
		{
			addPlainArrayField("bindPoses", 0, &SkeletonRTTI::getBindPose, &SkeletonRTTI::getNumBones,
				&SkeletonRTTI::setBindPose, &SkeletonRTTI::setNumBindPoses);
			addPlainArrayField("boneInfo", 1, &SkeletonRTTI::getBoneInfo, &SkeletonRTTI::getNumBones,
				&SkeletonRTTI::setBoneInfo, &SkeletonRTTI::setNumBoneInfos);
			addReflectableArrayField("boneTransforms", 3, &SkeletonRTTI::getBoneTransform, &SkeletonRTTI::getNumBones,
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
		static BitLength toMemory(const SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(data.name, stream);
				size += rtti_write(data.parent, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.name, stream);
			rtti_read(data.parent, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const SkeletonBoneInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += rtti_size(data.name);
			dataSize += rtti_size(data.parent);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
