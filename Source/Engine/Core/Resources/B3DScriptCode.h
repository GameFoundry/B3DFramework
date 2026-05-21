//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Resource containing script source code. */
	class B3D_EXPORT ScriptCode : public Resource
	{
	public:
		/**	Gets the source code contained in the resource. */
		const WString& GetString() const { return mString; }

		/**	Sets the source code contained in the resource. */
		void SetString(const WString& data) { mString = data; }

		/**	Gets a value that determines should the script code be compiled with editor assemblies. */
		bool GetIsEditorScript() const { return mEditorScript; }

		/**	Sets a value that determines should the script code be compiled with editor assemblies. */
		void SetIsEditorScript(bool editorScript) { mEditorScript = editorScript; }

		/**	Creates a new script code resource with the specified source code. */
		static HScriptCode Create(const WString& data, bool editorScript = false);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new scriptcode resource with the specified source string.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static TShared<ScriptCode> CreateShared(const WString& data, bool editorScript = false);

		/** @} */
	private:
		ScriptCode(const WString& data, bool editorScript);

		WString mString;
		bool mEditorScript;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ScriptCodeRTTI;
		static RTTIType* GetRttiStatic();
		virtual RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
