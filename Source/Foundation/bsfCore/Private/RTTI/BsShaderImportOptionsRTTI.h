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

	class B3D_CORE_EXPORT ShaderImportOptionsRTTI : public RTTIType<ShaderImportOptions, ImportOptions, ShaderImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Languages, 1)
		B3D_RTTI_END_MEMBERS

		std::pair<String, String>& GetDefinePair(ShaderImportOptions* obj, u32 idx)
		{
			return mDefinePairs[idx];
		}

		void SetDefinePair(ShaderImportOptions* obj, u32 idx, std::pair<String, String>& val)
		{
			obj->mDefines[val.first] = val.second;
		}

		u32 GetNumDefines(ShaderImportOptions* obj) { return (u32)obj->mDefines.size(); }

		void SetNumDefines(ShaderImportOptions* obj, u32 val)
		{ /* Do nothing */
		}

	public:
		ShaderImportOptionsRTTI()
		{
			AddPlainArrayField("mDefines", 0, &ShaderImportOptionsRTTI::GetDefinePair, &ShaderImportOptionsRTTI::GetNumDefines, &ShaderImportOptionsRTTI::SetDefinePair, &ShaderImportOptionsRTTI::SetNumDefines);
		}

		void OnOperationStarted(ShaderImportOptions& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				UnorderedMap<String, String>& defines = object.mDefines;
				for(auto& entry : defines)
					mDefinePairs.push_back(entry);
			}
		}

		const String& GetRttiName() override
		{
			static String name = "ShaderImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderImportOptions;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderImportOptions>();
		}

	private:
		Vector<std::pair<String, String>> mDefinePairs;
	};

	/** @} */
	/** @endcond */
} // namespace bs
