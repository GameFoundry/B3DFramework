//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DTransformRTTI.h"
#include "Animation/B3DSkeleton.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SkeletonRTTI : public TRTTIType<Skeleton, IReflectable, SkeletonRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mNumBones, 0)
			B3D_RTTI_MEMBER_CONTAINER(mInvBindPoses, 1)
			B3D_RTTI_MEMBER_CONTAINER(mBoneTransforms, 2)
			B3D_RTTI_MEMBER_CONTAINER(mBoneInfo, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "Skeleton";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Skeleton;
		}

		TShared<IReflectable> NewRttiObject()
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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Parent, stream);

				return size; });
		}

		static BitLength FromMemory(SkeletonBoneInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Parent, stream);

			return size;
		}

		static BitLength GetSize(const SkeletonBoneInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize;
			dataSize += B3DRTTISize(data.Name);
			dataSize += B3DRTTISize(data.Parent);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
