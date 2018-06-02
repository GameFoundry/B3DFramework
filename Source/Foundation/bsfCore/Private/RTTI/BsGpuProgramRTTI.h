//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsGpuProgramManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT GpuProgramBytecodeRTTI : public RTTIType<GpuProgramBytecode, IReflectable, GpuProgramBytecodeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(instructions, 0)
			BS_RTTI_MEMBER_REFLPTR(paramDesc, 1)
			BS_RTTI_MEMBER_PLAIN(vertexInput, 2)
			BS_RTTI_MEMBER_PLAIN(messages, 3)
			BS_RTTI_MEMBER_PLAIN(compilerId, 4)
			BS_RTTI_MEMBER_PLAIN(compilerVersion, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "GpuProgramBytecode";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GpuProgramBytecode;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<GpuProgramBytecode>();
		}
	};

	class BS_CORE_EXPORT GpuParamDescRTTI : public RTTIType<GpuParamDesc, IReflectable, GpuParamDescRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(paramBlocks, 0)
			BS_RTTI_MEMBER_PLAIN(params, 1)
			BS_RTTI_MEMBER_PLAIN(samplers, 2)
			BS_RTTI_MEMBER_PLAIN(textures, 3)
			BS_RTTI_MEMBER_PLAIN(loadStoreTextures, 4)
			BS_RTTI_MEMBER_PLAIN(buffers, 5)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "GpuParamDesc";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GpuParamDesc;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<GpuParamDesc>();
		}
	};

	class BS_CORE_EXPORT GpuProgramRTTI : public RTTIType<GpuProgram, IReflectable, GpuProgramRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mType, 2)
			BS_RTTI_MEMBER_PLAIN(mNeedsAdjacencyInfo, 3)
			BS_RTTI_MEMBER_PLAIN(mEntryPoint, 4)
			BS_RTTI_MEMBER_PLAIN(mSource, 6)
			BS_RTTI_MEMBER_PLAIN(mLanguage, 7)
		BS_END_RTTI_MEMBERS

	public:
		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// Need to ensure the core thread object is initialized
			GpuProgram* gpuProgram = static_cast<GpuProgram*>(obj);
			gpuProgram->blockUntilCoreInitialized();
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			GpuProgram* gpuProgram = static_cast<GpuProgram*>(obj);
			gpuProgram->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "GpuProgram";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GpuProgram;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GpuProgramManager::instance().createEmpty("", GPT_VERTEX_PROGRAM); // Params don't matter, they'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
}
