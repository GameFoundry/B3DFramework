//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "FileSystem/B3DPath.h"
#include "Material/B3DShaderVariation.h"

namespace b3d
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**
	 * A single shader the offline shader cook tool should cook, together with everything required to key the cooked
	 * artifacts identically to the runtime shader resolver (ShaderRegistry). Name + CachePrefix derive the store path,
	 * while Defines control compilation.
	 */
	struct ShaderCookItem
	{
		/** Shader name (source filename without extension). Combined with CachePrefix to form the cache key. */
		String Name;

		/** High-level (BSL) source of the shader. */
		String Source;

		/** Absolute path to the source file. Used for diagnostics and up-to-date checks. */
		Path SourcePath;

		/** Cache folder the shader is keyed under (for example "BuiltinShaders/" or "RendererMaterialShaders/"). */
		String CachePrefix;

		/** Defines to compile the shader with. Empty for surface shaders; per-material defines for renderer materials. */
		ShaderDefines Defines;
	};

	/**
	 * Provides the set of shaders to cook to the offline shader cook tool. Implementations enumerate shaders from a
	 * particular origin (the engine's builtin shader folder, the editor project library, ...) and classify each one
	 * into the cache prefix and defines the runtime resolver will look it up with.
	 */
	class IShaderCookerSource
	{
	public:
		virtual ~IShaderCookerSource() = default;

		/** Appends the shaders to be cooked to @p outItems. */
		virtual void GetItems(Vector<ShaderCookItem>& outItems) = 0;
	};

	/** @} */
} // namespace b3d
