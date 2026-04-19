//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShaderManager.h"

namespace b3d
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Shader include handler for the engine. It loads includes relative to the application working directory and supports
	 * special $ENGINE$ folder for built-in includes.
	 */
	class B3D_EXPORT EngineShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		HShaderInclude FindInclude(const String& name) const override;
		TOptional<String> FindIncludeSource(const String& name) const override;
		void AddSearchPath(const Path& path) override { mSearchPaths.push_back(path); }

		/** Converts a shader include name to a full path to the include file. */
		Path DetermineFullPath(const String& name) const;

	private:
		Vector<Path> mSearchPaths;
	};

	/** @} */
} // namespace b3d
