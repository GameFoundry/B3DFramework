//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
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

		std::pair<String, String>& GetDefinePair(ShaderImportOptions* obj, UINT32 idx)
		{
			return mDefinePairs[idx];
		}

		void SetDefinePair(ShaderImportOptions* obj, UINT32 idx, std::pair<String, String>& val)
		{
			obj->mDefines[val.first] = val.second;
		}

		UINT32 GetNumDefines(ShaderImportOptions* obj) { return (UINT32)obj->mDefines.size(); }
		void SetNumDefines(ShaderImportOptions* obj, UINT32 val) { /* Do nothing */ }

	public:
		ShaderImportOptionsRTTI()
		{
			AddPlainArrayField("mDefines", 0, &ShaderImportOptionsRTTI::GetDefinePair,
				&ShaderImportOptionsRTTI::GetNumDefines, &ShaderImportOptionsRTTI::SetDefinePair,
				&ShaderImportOptionsRTTI::SetNumDefines);
		}

		/** @copydoc ShaderImportOptionsRTTI::onSerializationStarted */
		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) 
		{
			ShaderImportOptions* importOptions = static_cast<ShaderImportOptions*>(obj);

			UnorderedMap<String, String>& defines = importOptions->mDefines;
			for (auto& entry : defines)
				mDefinePairs.push_back(entry);
		}

		/** @copydoc RTTIType::getRTTIName */
		const String& GetRttiName() 
		{
			static String name = "ShaderImportOptions";
			return name;
		}

		/** @copydoc RTTIType::getRTTIId */
		UINT32 GetRttiId() 
		{
			return TID_ShaderImportOptions;
		}

		/** @copydoc RTTIType::NewRttiObject */
		SPtr<IReflectable> NewRttiObject() 
		{
			return bs_shared_ptr_new<ShaderImportOptions>();
		}

	private:
		Vector<std::pair<String, String>> mDefinePairs;
	};

	/** @} */
	/** @endcond */
}
