//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Utility/B3DModule.h"
#include "Material/B3DShader.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuBackend.h"

namespace b3d
{
	class Package;

	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**
	 * Resolves Shader objects and owns the read-only store of prebuilt (cooked) shaders. A shader is resolved in the 
	 * following order: the prebuilt store, then the application cache, then on-demand compilation via ShaderCompilers. 
	 * The prebuilt store is produced ahead of time by the offline shader cook tool, the system falls back on other
	 * methods if the shader is not available in the prebuilt store or prebuilt store is out of date.
	 *
	 * @note	Non-development builds will return prebuilt shaders as-is, without checking if they are out of date. 
	 */
	class B3D_EXPORT ShaderRegistry : public Module<ShaderRegistry>
	{
	public:
		ShaderRegistry();
		~ShaderRegistry() override;

		/** Registers a path that will be used for looking for shader source files. Thread safe. */
		void RegisterSearchPath(const Path& folder);

		/**
		 * Resolves a Shader object, retrieving it from the prebuilt shader store or the application cache if available, or
		 * compiling it from source and adding it to the cache otherwise. The lookup order is: prebuilt
		 * store, then the application cache, then on-demand compilation.
		 *
		 * @param	shaderPath		Relative or absolute path to the shader source file. If relative, search paths provided
		 *							through RegisterSearchPath() will be searched for the file. 
		 * @param	cachePrefix		Folder within the cache (and the prebuilt store) to perform the lookup in.
		 * @param	defines			Optional set of defines to use when compiling the shader.
		 * @return					Shader object on success, or null on failure.
		 */
		template <bool IsRenderProxy>
		TShared<CoreVariantType<Shader, IsRenderProxy>> GetOrCompileShader(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);

		/**
		 * Resolves compiled pass data for a shader variation, retrieving it from the application cache if available, or
		 * compiling it and adding it to the cache otherwise.
		 *
		 * @param	shader		Shader that owns the variation.
		 * @param	variation	Variation to initialize with compiled pass data.
		 * @param	language	Shading language identifier for the variation.
		 * @return				True if the variation pass data was resolved successfully, or false on failure.
		 */
		template <bool IsRenderProxy>
		bool GetOrCompileVariation(const TShared<CoreVariantType<Shader, IsRenderProxy>>& shader, const TShared<CoreVariantType<Variation, IsRenderProxy>>& variation, const String& language);

	protected:
		void OnStartUp() override;
		void OnShutDown() override;

	private:
		TShared<Package> mPrebuiltStore; /**< Read-only store of prebuilt shaders located next to the executable. Null when no store was found. */

		Vector<Path> mSearchPaths; /**< Folders searched for shader source files when a shader has to be compiled. */
		Mutex mSearchPathMutex;
	};

	/** @} */
} // namespace b3d
