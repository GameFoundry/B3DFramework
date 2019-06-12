//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Material/BsShaderManager.h"

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
		HShaderInclude findInclude(const String& name) const override;

		/** @copydoc IShaderIncludeHandler::addSearchPath */
		void addSearchPath(const Path& path) override { mSearchPaths.push_back(path); }

		/** Converts a shader include name or path to a path of the resource containing include data. */
		static Path toResourcePath(const String& name);

	private:
		Vector<Path> mSearchPaths;
	};

	/** @} */
}