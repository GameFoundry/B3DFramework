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

	class BS_CORE_EXPORT SkeletonRTTI : public RTTIType<Skeleton, IReflectable, SkeletonRTTI>
	{
	private:
		Matrix4& GetBindPose(Skeleton* obj, u32 idx) { return obj->mInvBindPoses[idx]; }

		void SetBindPose(Skeleton* obj, u32 idx, Matrix4& value) { obj->mInvBindPoses[idx] = value; }

		void SetNumBindPoses(Skeleton* obj, u32 size)
		{
			obj->mNumBones = size;

			assert(obj->mInvBindPoses == nullptr);
			obj->mInvBindPoses = bs_newN<Matrix4>(size);
		}

		SkeletonBoneInfo& GetBoneInfo(Skeleton* obj, u32 idx) { return obj->mBoneInfo[idx]; }

		void SetBoneInfo(Skeleton* obj, u32 idx, SkeletonBoneInfo& value) { obj->mBoneInfo[idx] = value; }

		void SetNumBoneInfos(Skeleton* obj, u32 size)
		{
			obj->mNumBones = size;

			assert(obj->mBoneInfo == nullptr);
			obj->mBoneInfo = bs_newN<SkeletonBoneInfo>(size);
		}

		Transform& GetBoneTransform(Skeleton* obj, u32 idx) { return obj->mBoneTransforms[idx]; }

		void SetBoneTransform(Skeleton* obj, u32 idx, Transform& value) { obj->mBoneTransforms[idx] = value; }

		void SetNumBoneTransforms(Skeleton* obj, u32 size)
		{
			obj->mNumBones = size;

			assert(obj->mBoneTransforms == nullptr);
			obj->mBoneTransforms = bs_newN<Transform>(size);
		}

		u32 GetNumBones(Skeleton* obj) { return obj->mNumBones; }

	public:
		SkeletonRTTI()
		{
			AddPlainArrayField("bindPoses", 0, &SkeletonRTTI::GetBindPose, &SkeletonRTTI::GetNumBones, &SkeletonRTTI::SetBindPose, &SkeletonRTTI::SetNumBindPoses);
			AddPlainArrayField("boneInfo", 1, &SkeletonRTTI::GetBoneInfo, &SkeletonRTTI::GetNumBones, &SkeletonRTTI::SetBoneInfo, &SkeletonRTTI::SetNumBoneInfos);
			AddReflectableArrayField("boneTransforms", 3, &SkeletonRTTI::GetBoneTransform, &SkeletonRTTI::GetNumBones, &SkeletonRTTI::SetBoneTransform, &SkeletonRTTI::SetNumBoneTransforms);
		}

		const String& GetRttiName()
		{
			static String name = "Skeleton";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_Skeleton;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Skeleton::CreateEmpty();
		}
	};

	template <>
	struct RTTIPlainType<SkeletonBoneInfo>
	{
		enum
		{
			id = TID_SkeletonBoneInfo
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Parent, stream);

				return size; });
		}

		static BitLength FromMemory(SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.Name, stream);
			rtti_read(data.Parent, stream);

			return size;
		}

		static BitLength GetSize(const SkeletonBoneInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += rtti_size(data.Name);
			dataSize += rtti_size(data.Parent);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
