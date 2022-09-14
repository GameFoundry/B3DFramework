//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Resources-Engine
	 *  @{
	 */

	/**	Resource containing script source code. */
	class BS_EXPORT ScriptCode : public Resource
	{
	public:
		/**	Gets the source code contained in the resource. */
		const WString& getString() const { return mString; }

		/**	Sets the source code contained in the resource. */
		void setString(const WString& data) { mString = data; }

		/**	Gets a value that determines should the script code be compiled with editor assemblies. */
		bool getIsEditorScript() const { return mEditorScript; }

		/**	Sets a value that determines should the script code be compiled with editor assemblies. */
		void setIsEditorScript(bool editorScript) { mEditorScript = editorScript; }

		/**	Creates a new script code resource with the specified source code. */
		static HScriptCode create(const WString& data, bool editorScript = false);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new scriptcode resource with the specified source string.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<ScriptCode> CreatePtrInternal(const WString& data, bool editorScript = false);

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
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
