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
	class B3D_EXPORT EngineShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		HShaderInclude FindInclude(const String& name) const override;
		Optional<String> FindIncludeSource(const String& name) const override;
		void AddSearchPath(const Path& path) override { mSearchPaths.push_back(path); }

		/** Converts a shader include name to a full path to the include file. */
		Path DetermineFullPath(const String& name) const;

	private:
		Vector<Path> mSearchPaths;
	};

	/** @} */
} // namespace bs
