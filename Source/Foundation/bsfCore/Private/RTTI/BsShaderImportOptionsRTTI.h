//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(languages, 1)
		BS_END_RTTI_MEMBERS

		std::pair<String, String>& getDefinePair(ShaderImportOptions* obj, UINT32 idx)
		{
			return mDefinePairs[idx];
		}

		void setDefinePair(ShaderImportOptions* obj, UINT32 idx, std::pair<String, String>& val)
		{
			obj->mDefines[val.first] = val.second;
		}

		UINT32 getNumDefines(ShaderImportOptions* obj) { return (UINT32)obj->mDefines.size(); }
		void setNumDefines(ShaderImportOptions* obj, UINT32 val) { /* Do nothing */ }

	public:
		ShaderImportOptionsRTTI()
		{
			addPlainArrayField("mDefines", 0, &ShaderImportOptionsRTTI::getDefinePair, 
				&ShaderImportOptionsRTTI::getNumDefines, &ShaderImportOptionsRTTI::setDefinePair, 
				&ShaderImportOptionsRTTI::setNumDefines);
		}

		/** @copydoc ShaderImportOptionsRTTI::onSerializationStarted */
		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ShaderImportOptions* importOptions = static_cast<ShaderImportOptions*>(obj);

			UnorderedMap<String, String>& defines = importOptions->mDefines;
			for (auto& entry : defines)
				mDefinePairs.push_back(entry);
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

	private:
		Vector<std::pair<String, String>> mDefinePairs;
	};

	/** @} */
	/** @endcond */
}