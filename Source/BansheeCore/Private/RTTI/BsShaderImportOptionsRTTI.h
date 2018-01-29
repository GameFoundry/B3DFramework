//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Importer/BsShaderImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ShaderImportOptionsRTTI : public RTTIType<ShaderImportOptions, ImportOptions, ShaderImportOptionsRTTI>
	{
	private:
		std::pair<String, String>& getDefinePair(ShaderImportOptions* obj, UINT32 idx)
		{
			Vector<std::pair<String, String>>& definePairs = 
				any_cast_ref<Vector<std::pair<String, String>>>(obj->mRTTIData);

			return definePairs[idx];
		}


		void setDefinePair(ShaderImportOptions* obj, UINT32 idx, std::pair<String, String>& val)
		{
			obj->getDefines()[val.first] = val.second;
		}

		UINT32 getNumDefines(ShaderImportOptions* obj) { return (UINT32)obj->getDefines().size(); }
		void setNumDefines(ShaderImportOptions* obj, UINT32 val) { /* Do nothing */ }

	public:
		ShaderImportOptionsRTTI()
		{
			addPlainArrayField("mDefines", 0, &ShaderImportOptionsRTTI::getDefinePair, 
				&ShaderImportOptionsRTTI::getNumDefines, &ShaderImportOptionsRTTI::setDefinePair, 
				&ShaderImportOptionsRTTI::setNumDefines);
		}

		/** @copydoc ShaderImportOptionsRTTI::onSerializationStarted */
		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			ShaderImportOptions* importOptions = static_cast<ShaderImportOptions*>(obj);

			Vector<std::pair<String, String>> definePairs;
			UnorderedMap<String, String>& defines = importOptions->getDefines();

			for (auto& entry : defines)
				definePairs.push_back(entry);

			importOptions->mRTTIData = definePairs;
		}

		/** @copydoc RTTIType::onSerializationEnded */
		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			ShaderImportOptions* importOptions = static_cast<ShaderImportOptions*>(obj);
			importOptions->mRTTIData = nullptr;
		}

		/** @copydoc RTTIType::getRTTIName */
		const String& getRTTIName() override
		{
			static String name = "ShaderImportOptions";
			return name;
		}

		/** @copydoc RTTIType::getRTTIId */
		UINT32 getRTTIId() override
		{
			return TID_ShaderImportOptions;
		}

		/** @copydoc RTTIType::newRTTIObject */
		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ShaderImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}