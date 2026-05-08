//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Resources/B3DScriptCode.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Contains import options you may use to control how is a file containing script source code importer. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), API(Framework), API(Editor)) ScriptCodeImportOptions : public ImportOptions
	{
	public:
		ScriptCodeImportOptions() = default;

		/**	Determines whether the script is editor-only or a normal game script. */
		B3D_SCRIPT_EXPORT()
		bool EditorScript = false;

		/** Creates a new import options object that allows you to customize how is script code imported. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static TShared<ScriptCodeImportOptions> Create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ScriptCodeImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
