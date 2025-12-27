//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DLightProbeVolume.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DStdRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(LightProbeSHCoefficients, TID_LightProbeSHCoefficient)

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


	class B3D_EXPORT LightProbeVolumeRTTI : public TRTTIType<LightProbeVolume, Component, LightProbeVolumeRTTI>
	{
	private:
		SavedLightProbeInfo mSavedLightProbeInfo;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mVolume, 0)
			B3D_RTTI_MEMBER(mCellCount, 1)
			B3D_RTTI_GENERATED_MEMBER(mSavedLightProbeInfo, 2)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(LightProbeVolume& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				object.UpdateCoefficients();

				const u32 probeCount = (u32)object.mProbes.size();
				mSavedLightProbeInfo.Coefficients.resize(probeCount);
				mSavedLightProbeInfo.Positions.resize(probeCount);

				u32 probeIndex = 0;
				for(auto& entry : object.mProbes)
				{
					mSavedLightProbeInfo.Positions[probeIndex] = entry.second.Position;
					mSavedLightProbeInfo.Coefficients[probeIndex] = entry.second.Coefficients;

					probeIndex++;
				}
			}
		}

		void OnOperationEnded(LightProbeVolume& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.mProbes.clear();

				u32 probeCount = (u32)mSavedLightProbeInfo.Positions.size();
				for(u32 probeIndex = 0; probeIndex < probeCount; ++probeIndex)
				{
					const u32 probeHandle = object.mNextProbeId++;

					LightProbeVolume::ProbeInfo probeInfo;
					probeInfo.Flags = LightProbeFlags::Clean;
					probeInfo.Position = mSavedLightProbeInfo.Positions[probeIndex];
					probeInfo.Coefficients = mSavedLightProbeInfo.Coefficients[probeIndex];

					object.mProbes[probeHandle] = probeInfo;
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

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<LightProbeVolume>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
