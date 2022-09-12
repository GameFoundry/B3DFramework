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
		Matrix4& GetBindPose(Skeleton* obj, UINT32 idx) { return obj->mInvBindPoses[idx]; }
		void SetBindPose(Skeleton* obj, UINT32 idx, Matrix4& value) { obj->mInvBindPoses[idx] = value; }

		void SetNumBindPoses(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;

			assert(obj->mInvBindPoses == nullptr);
			obj->mInvBindPoses = bs_newN<Matrix4>(size);
		}

		SkeletonBoneInfo& GetBoneInfo(Skeleton* obj, UINT32 idx) { return obj->mBoneInfo[idx]; }
		void SetBoneInfo(Skeleton* obj, UINT32 idx, SkeletonBoneInfo& value) { obj->mBoneInfo[idx] = value; }

		void SetNumBoneInfos(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;

			assert(obj->mBoneInfo == nullptr);
			obj->mBoneInfo = bs_newN<SkeletonBoneInfo>(size);
		}

		Transform& GetBoneTransform(Skeleton* obj, UINT32 idx) { return obj->mBoneTransforms[idx]; }
		void SetBoneTransform(Skeleton* obj, UINT32 idx, Transform& value) { obj->mBoneTransforms[idx] = value; }

		void SetNumBoneTransforms(Skeleton* obj, UINT32 size)
		{
			obj->mNumBones = size;

			assert(obj->mBoneTransforms == nullptr);
			obj->mBoneTransforms = bs_newN<Transform>(size);
		}

		UINT32 GetNumBones(Skeleton* obj) { return obj->mNumBones; }
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

		const String& GetRTTIName() override
		{
			static String name = "Skeleton";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_Skeleton;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return Skeleton::CreateEmpty();
		}
	};

	template<> struct RTTIPlainType<SkeletonBoneInfo>
	{
		enum { id = TID_SkeletonBoneInfo }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static BitLength ToMemory(const SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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
		static BitLength FromMemory(SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.name, stream);
			rtti_read(data.parent, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getSize */
		static BitLength GetSize(const SkeletonBoneInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
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
