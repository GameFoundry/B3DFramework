//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
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

		static void toMemory(const SavedLightProbeInfo& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			UINT32 version = 0;

			memory = rttiWriteElem(version, memory);
			memory = rttiWriteElem(data.positions, memory);
			rttiWriteElem(data.coefficients, memory);
		}

		static UINT32 fromMemory(SavedLightProbeInfo& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 version;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
				memory = rttiReadElem(data.positions, memory);
				memory = rttiReadElem(data.coefficients, memory);
				break;
			default:
				LOGERR("Unknown version of SavedLightProbeInfo data. Unable to deserialize.");
				break;
			}

			return size;
		}

		static UINT32 getDynamicSize(const SavedLightProbeInfo& data)
		{
			UINT64 dataSize = rttiGetElemSize(data.positions) + rttiGetElemSize(data.coefficients) + sizeof(UINT32) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
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
			SavedLightProbeInfo savedLightProbeInfo;
			savedLightProbeInfo.coefficients.resize(numProbes);
			savedLightProbeInfo.positions.resize(numProbes);

			UINT32 idx = 0;
			for(auto& entry : obj->mProbes)
			{
				savedLightProbeInfo.positions[idx] = entry.second.position;
				savedLightProbeInfo.coefficients[idx] = entry.second.coefficients;

				idx++;
			}

			obj->mRTTIData = savedLightProbeInfo;
			return any_cast_ref<SavedLightProbeInfo>(obj->mRTTIData);
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
				RTTI_Flag_SkipInReferenceSearch);
		}

		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// Clear temporary data
			LightProbeVolume* volume = static_cast<LightProbeVolume*>(obj);
			volume->mRTTIData = nullptr;
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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
	};

	/** @} */
	/** @endcond */
}
