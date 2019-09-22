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
#include "CoreThread/BsCoreThread.h"
#include "Private/RTTI/BsTextureRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(LightProbeSHCoefficients)

	/** Serializable information about a single light probe. */
	struct SavedLightProbeInfo
	{
		Vector<Vector3> positions;
		Vector<LightProbeSHCoefficients> coefficients;
	};

	template<> struct RTTIPlainType<SavedLightProbeInfo>
	{
		enum { id = TID_SavedLightProbeInfo }; enum { hasDynamicSize = 1 };
		static constexpr UINT32 VERSION = 0;

		static BitLength toMemory(const SavedLightProbeInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				uint32_t version;
				size += rtti_write(version, stream);
				size += rtti_write(data.positions, stream);
				size += rtti_write(data.coefficients, stream);

				return size;
			});
		}

		static BitLength fromMemory(SavedLightProbeInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				rtti_read(data.positions, stream);
				rtti_read(data.coefficients, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version of SavedLightProbeInfo data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength getSize(const SavedLightProbeInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.positions) + rtti_size(data.coefficients) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	class BS_CORE_EXPORT LightProbeVolumeRTTI : public RTTIType <LightProbeVolume, IReflectable, LightProbeVolumeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mTransform, 0)
			BS_RTTI_MEMBER_PLAIN(mActive, 1)
			BS_RTTI_MEMBER_PLAIN(mMobility, 2)
			BS_RTTI_MEMBER_PLAIN(mVolume, 3)
			BS_RTTI_MEMBER_PLAIN(mCellCount, 4)
		BS_END_RTTI_MEMBERS

		SavedLightProbeInfo& getProbeInfo(LightProbeVolume* obj)
		{
			obj->updateCoefficients();

			UINT32 numProbes = (UINT32)obj->mProbes.size();
			mSavedLightProbeInfo.coefficients.resize(numProbes);
			mSavedLightProbeInfo.positions.resize(numProbes);

			UINT32 idx = 0;
			for(auto& entry : obj->mProbes)
			{
				mSavedLightProbeInfo.positions[idx] = entry.second.position;
				mSavedLightProbeInfo.coefficients[idx] = entry.second.coefficients;

				idx++;
			}

			return mSavedLightProbeInfo;
		}

		void setProbeInfo(LightProbeVolume* obj, SavedLightProbeInfo& data)
		{
			obj->mProbes.clear();

			UINT32 numProbes = (UINT32)data.positions.size();
			for(UINT32 i = 0; i < numProbes; ++i)
			{
				UINT32 handle = obj->mNextProbeId++;

				LightProbeVolume::ProbeInfo probeInfo;
				probeInfo.flags = LightProbeFlags::Clean;
				probeInfo.position = data.positions[i];
				probeInfo.coefficients = data.coefficients[i];

				obj->mProbes[handle] = probeInfo;
			}
		}
	public:
		LightProbeVolumeRTTI()
		{
			addPlainField("mProbeInfo", 5, &LightProbeVolumeRTTI::getProbeInfo, &LightProbeVolumeRTTI::setProbeInfo,
				RTTIFieldInfo(RTTIFieldFlag::SkipInReferenceSearch));
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "LightProbeVolume";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_LightProbeVolume;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return LightProbeVolume::createEmpty();
		}

	private:
		SavedLightProbeInfo mSavedLightProbeInfo;
	};

	/** @} */
	/** @endcond */
}
