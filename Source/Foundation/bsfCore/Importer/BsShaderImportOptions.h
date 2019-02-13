//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsImportOptions.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Supported types of low-level shading languages. */
	enum class BS_SCRIPT_EXPORT(m:Importer,n:ShadingLanguageFlags,api:bsf) ShadingLanguageFlag
	{
		/** High level shading language used by DirectX backend. */
		HLSL = 1 << 0, 
		/** OpenGL shading language. */
		GLSL = 1 << 1, 
		/** Variant of GLSL used for Vulkan. */
		VKSL = 1 << 2,
		/** Helper entry that includes all languages. */
		All = HLSL | GLSL | VKSL
	};

	using ShadingLanguageFlags = Flags<ShadingLanguageFlag>;
	BS_FLAGS_OPERATORS(ShadingLanguageFlag)

	/** Contains import options you may use to control how is a shader imported. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf) ShaderImportOptions : public ImportOptions
	{
	public:
		/** 
		 * Sets a define and its value. Replaces an existing define if one already exists with the provided name. 
		 * 
		 * @param[in]	define		Name of the define.
		 * @param[in]	value		Value to assign to the define.
		 */
		BS_SCRIPT_EXPORT()
		void setDefine(const String& define, const String& value)
		{
			mDefines[define] = value;
		}

		/** 
		 * Checks if the define exists and returns its value if it does. 
		 * 
		 * @param[in]	define		Name of the define to get the value for.
		 * @param[out]	value		value of the define. Only defined if the method returns true.
		 * @returns					True if the define was found, false otherwise.
		 */
		BS_SCRIPT_EXPORT()
		bool getDefine(const String& define, String& value) const
		{
			auto iterFind = mDefines.find(define);
			if(iterFind != mDefines.end())
			{
				value = iterFind->second;
				return true;
			}

			return false;
		}

		/** 
		 * Checks if the provided define exists. 
		 *
		 * @param[in]	define		Name of the define to check.
		 * @returns					True if the define was found, false otherwise.
		 */
		BS_SCRIPT_EXPORT()
		bool hasDefine(const String& define) const
		{
			auto iterFind = mDefines.find(define);
			return iterFind != mDefines.end();
		}

		/**
		 * Unregisters a previously set define.
		 * 
		 * @param[in]	define		Name of the define to unregister. 
		 */
		BS_SCRIPT_EXPORT()
		void removeDefine(const String& define)
		{
			mDefines.erase(define);
		}

		/** Returns all the set defines and their values. */
		const UnorderedMap<String, String>& getDefines() const { return mDefines; }

		/** 
		 * Flags that control which shading languages should the BSL shader be converted into. This ultimately controls on 
		 * which render backends it will be able to run on.
		 */
		BS_SCRIPT_EXPORT()
		ShadingLanguageFlags languages = ShadingLanguageFlag::All;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	private:
		UnorderedMap<String, String> mDefines;
	};

	/** @} */
}