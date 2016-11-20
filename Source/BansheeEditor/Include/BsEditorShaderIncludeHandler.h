//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsShaderManager.h"

namespace bs
{
	/** @addtogroup Library-Internal
	 *  @{
	 */

	/**
	 * Shader include handler for the editor. It uses project library to find the includes, and supports special keywords 
	 * for built-in includes that reside outside of the project library.
	 */
	class BS_ED_EXPORT EditorShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		/** @copydoc IShaderIncludeHandler::findInclude */
		virtual HShaderInclude findInclude(const String& name) const override;

		/** Converts a shader include name or path to a path of the resource containing include data. */
		static Path toResourcePath(const String& name);
	};

	/** @} */
}