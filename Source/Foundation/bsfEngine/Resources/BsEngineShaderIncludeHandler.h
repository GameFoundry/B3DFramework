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
		HShaderInclude FindInclude(const String& name) const;

		/** @copydoc IShaderIncludeHandler::addSearchPath */
		void AddSearchPath(const Path& path) { mSearchPaths.push_back(path); }

		/** Converts a shader include name or path to a path of the resource containing include data. */
		static Path ToResourcePath(const String& name);

	private:
		Vector<Path> mSearchPaths;
	};

	/** @} */
} // namespace bs
