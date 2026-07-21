//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Material/B3DShaderCompiler.h"

namespace b3d
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how is a shader imported. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), Assembly(Engine), Assembly(Editor), API(FrameworkOnly)) ShaderImportOptions : public ImportOptions
	{
	public:
		/**
		 * Sets a define and its value. Replaces an existing define if one already exists with the provided name.
		 *
		 * @param	define		Name of the define.
		 * @param	value		Value to assign to the define.
		 */
		B3D_SCRIPT_EXPORT()
		void SetDefine(const String& define, const String& value)
		{
			mDefines[define] = value;
		}

		/**
		 * Checks if the define exists and returns its value if it does.
		 *
		 * @param	define		Name of the define to get the value for.
		 * @param	outValue	Value of the define. Only defined if the method returns true.
		 * @return				True if the define was found, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		bool GetDefine(const String& define, String& outValue) const
		{
			auto found = mDefines.find(define);
			if(found != mDefines.end())
			{
				outValue = found->second;
				return true;
			}

			return false;
		}

		/**
		 * Checks if the provided define exists.
		 *
		 * @param	define		Name of the define to check.
		 * @return				True if the define was found, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		bool HasDefine(const String& define) const
		{
			auto found = mDefines.find(define);
			return found != mDefines.end();
		}

		/**
		 * Unregisters a previously set define.
		 *
		 * @param	define		Name of the define to unregister.
		 */
		B3D_SCRIPT_EXPORT()
		void RemoveDefine(const String& define)
		{
			mDefines.erase(define);
		}

		/** Returns all the set defines and their values. */
		const UnorderedMap<String, String>& GetDefines() const { return mDefines; }

		/**
		 * Low-level shading language identifiers (for example "hlsl", "vksl") the BSL shader should be converted into.
		 * This ultimately controls on which render backends it will be able to run on.
		 */
		B3D_SCRIPT_EXPORT()
		Vector<String> Languages = { kGpuProgramLanguageHlsl, kGpuProgramLanguageVksl, kGpuProgramLanguageMvksl, kGpuProgramLanguageMsl };

		/** Creates a new import options object that allows you to customize how are meshes imported. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))

		static TShared<ShaderImportOptions> Create() { return B3DMakeShared<ShaderImportOptions>(); }

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	private:
		UnorderedMap<String, String> mDefines;
	};

	/** @} */
} // namespace b3d
