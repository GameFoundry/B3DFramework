//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "FileSystem/B3DPath.h"
#include "Material/B3DShaderCookerSource.h"

namespace b3d
{
	/**
	 * Cook source for the engine's builtin shaders. Globs the builtin shader folder for *.bsl files and classifies each
	 * one against the registered renderer materials: shaders backing a renderer material are emitted under the
	 * "RendererMaterialShaders/" cache prefix with that material's defines, all others under "BuiltinShaders/" with no
	 * defines - matching how BuiltinResources and RendererMaterial resolve them at runtime.
	 *
	 * @note	Requires the renderer materials to have registered with RendererMaterialManager (means you need the non-null renderer loaded when running this).
	 */
	class BuiltinShaderCookerSource final : public IShaderCookerSource
	{
	public:
		/**
		 * @param	shaderFolder	Absolute path to the folder containing the builtin *.bsl shaders (typically
		 *							BuiltinResources::GetShaderFolder()). Only top-level files are cooked; the Includes
		 *							sub-folder is ignored.
		 */
		explicit BuiltinShaderCookerSource(Path shaderFolder);

		void GetItems(Vector<ShaderCookItem>& outItems) override;

	private:
		Path mShaderFolder;
	};
} // namespace b3d
