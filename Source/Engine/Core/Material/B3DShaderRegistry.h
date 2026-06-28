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

	/** Resource wrapper that holds a single cooked shader artifact (e.g PrecompiledShaderData, PrecompiledVariationData). */
	class B3D_EXPORT PrebuiltShader final : public Resource
	{
	public:
		/** Returns the wrapped shader object, or null if none is present. */
		TShared<IReflectable> GetObject() const { return mObjects.Empty() ? nullptr : mObjects[0]; }

		/** Creates a wrapper holding the provided shader object. */
		static TShared<PrebuiltShader> Create(const TShared<IReflectable>& object);

	private:
		explicit PrebuiltShader(const TShared<IReflectable>& object)
			: Resource(false), mObjects({ object })
		{ }

		TInlineArray<TShared<IReflectable>, 1> mObjects;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PrebuiltShaderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

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

		/**
		 * @name Cache / prebuilt-store path derivation
		 *
		 * Helpers that compute the virtual paths under which shaders and their variations are keyed in both the
		 * application cache and the prebuilt store. They are shared between the runtime resolver (GetOrCompileShader /
		 * GetOrCompileVariation) and the offline shader cook tool so the two can never drift in how entries are named.
		 * @{
		 */

		/**
		 * Returns the store directory for a shader, of the form "<cachePrefix><shaderName>/". This is the value stored
		 * in ShaderCompilerMetaData::NameInCache and forms the root of every per-language entry for that shader.
		 */
		static String GetShaderCacheName(const String& cachePrefix, const String& shaderName);

		/** Returns the virtual path of a shader's metadata entry: "<shaderCacheName><language>/MetaData". */
		static Path GetShaderMetaDataPath(const String& shaderCacheName, const String& language);

		/**
		 * Returns the virtual path of a shader variation entry: "<NameInCache><language>/<hash>". The hash folds in the
		 * variation name together with the shader's source and include hashes (taken from @p metadata), so the key is
		 * source-sensitive: it changes whenever the shader or any of its includes is edited, and stale entries are then
		 * simply not found. The application cache, the prebuilt store, and the cook all key variations through here so
		 * the three can never drift.
		 */
		static Path GetVariationPath(const ShaderCompilerMetaData& metadata, const String& language, const String& variationName);

		/**
		 * Returns the absolute path of the default prebuilt shader store - the package this module loads at start-up and
		 * the offline shader cook tool writes by default.
		 */
		static Path GetPrebuiltStorePath();

		/** @} */

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
