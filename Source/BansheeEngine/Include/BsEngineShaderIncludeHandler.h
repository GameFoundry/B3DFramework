//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsShaderManager.h"

namespace bs
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	/**
	 * Shader include handler for the engine. It loads includes relative to the application working directory and supports
	 * special $ENGINE$ folder for built-in includes.
	 */
	class BS_EXPORT EngineShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		/** @copydoc IShaderIncludeHandler::findInclude */
		virtual HShaderInclude findInclude(const String& name) const override;

		/** Converts a shader include name or path to a path of the resource containing include data. */
		static Path toResourcePath(const String& name);
	};

	/** @} */
}