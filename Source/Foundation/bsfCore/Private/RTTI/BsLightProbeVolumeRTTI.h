//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Renderer/BsLightProbeVolume.h"
#include "Renderer/BsRenderer.h"
#include "CoreObject/BsRenderThread.h"
#include "Private/RTTI/BsTextureRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(LightProbeSHCoefficients)

	/** Serializable information about a single light probe. */
	struct SavedLightProbeInfo
	{
		Vector<Vector3> Positions;
		Vector<LightProbeSHCoefficients> Coefficients;
	};

	template <>
	struct RTTIPlainType<SavedLightProbeInfo>
	{
		enum
		{
			id = TID_SavedLightProbeInfo
		};

		enum
		{
			hasDynamicSize = 1
		};

		static constexpr u32 kVersion = 0;

		static BitLength ToMemory(const SavedLightProbeInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				uint32_t version;
				size += B3DRTTIWrite(version, stream);
				size += B3DRTTIWrite(data.Positions, stream);
				size += B3DRTTIWrite(data.Coefficients, stream);

				return size; });
		}

		static BitLength FromMemory(SavedLightProbeInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				B3DRTTIRead(data.Positions, stream);
				B3DRTTIRead(data.Coefficients, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version of SavedLightProbeInfo data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const SavedLightProbeInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.Positions) + B3DRTTISize(data.Coefficients) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	class B3D_CORE_EXPORT LightProbeVolumeRTTI : public RTTIType<LightProbeVolume, IReflectable, LightProbeVolumeRTTI>
	{
		SavedLightProbeInfo mSavedLightProbeInfo;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mTransform, 0)
			B3D_RTTI_MEMBER(mActive, 1)
			B3D_RTTI_MEMBER(mMobility, 2)
			B3D_RTTI_MEMBER(mVolume, 3)
			B3D_RTTI_MEMBER(mCellCount, 4)
			B3D_RTTI_GENERATED_MEMBER(mSavedLightProbeInfo, 5)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(LightProbeVolume& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				object.UpdateCoefficients();

				u32 numProbes = (u32)object.mProbes.size();
				mSavedLightProbeInfo.Coefficients.resize(numProbes);
				mSavedLightProbeInfo.Positions.resize(numProbes);

				u32 idx = 0;
				for(auto& entry : object.mProbes)
				{
					mSavedLightProbeInfo.Positions[idx] = entry.second.Position;
					mSavedLightProbeInfo.Coefficients[idx] = entry.second.Coefficients;

					idx++;
				}
			}
		}

		void OnOperationEnded(LightProbeVolume& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.mProbes.clear();

				u32 numProbes = (u32)mSavedLightProbeInfo.Positions.size();
				for(u32 i = 0; i < numProbes; ++i)
				{
					u32 handle = object.mNextProbeId++;

					LightProbeVolume::ProbeInfo probeInfo;
					probeInfo.Flags = LightProbeFlags::Clean;
					probeInfo.Position = mSavedLightProbeInfo.Positions[i];
					probeInfo.Coefficients = mSavedLightProbeInfo.Coefficients[i];

					object.mProbes[handle] = probeInfo;
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "LightProbeVolume";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_LightProbeVolume;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return LightProbeVolume::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
