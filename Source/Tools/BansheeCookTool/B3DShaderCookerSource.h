//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "FileSystem/B3DPath.h"
#include "Material/B3DIShaderCookerSource.h"

namespace b3d
{
	/**
	 * Enumerates all the shaders in a single folder. Globs the folder for *.bsl files and classifies each one
	 * as either a renderer material shader or a regular shader.
	 */
	class ShaderCookerSource final : public IShaderCookerSource
	{
	public:
		/**
		 * @param	shaderFolder	Absolute path to the folder containing the *.bsl shaders to cook. Only top-level
		 *							files are cooked; sub-folders may hold shared include code and are ignored.
		 */
		explicit ShaderCookerSource(Path shaderFolder);

		void GetItems(Vector<ShaderCookItem>& outItems) override;

	private:
		Path mShaderFolder;
	};
} // namespace b3d
