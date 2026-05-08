//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShader.h"
#include "GpuBackend/B3DGpuProgram.h"

namespace b3d
{
	struct BSLParsedShaderMetaData;
	struct BSLParsedShaderData;

	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Supported types of low-level shading languages. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), ExportName(ShadingLanguageFlags), API(Framework), API(Editor)) ShadingLanguageFlag
	{
		Unknown = 0,
		/** High level shading language used by DirectX backend. */
		HLSL = 1 << 0,
		/** OpenGL shading language. */
		GLSL = 1 << 1,
		/** Variant of GLSL used for Vulkan. */
		VKSL = 1 << 2,
		/** Metal shading language. */
		MSL = 1 << 3,
		/** Null shading language for the null render backend. Produces no actual shader code. */
		NullSL = 1 << 4,
		Count = 5,
		/** Helper entry that includes all languages. */
		All = HLSL | GLSL | VKSL | MSL
	};

	using ShadingLanguageFlags = Flags<ShadingLanguageFlag>;
	B3D_FLAGS_OPERATORS(ShadingLanguageFlag)

	/**	Contains the results of shader parsing or compilation. */
	struct ShaderCompilerResult
	{
		String ErrorMessage; /**< Error message if parsing/compilation failed. */
		int ErrorLine = 0; /**< Line of the error if one occurred. */
		int ErrorColumn = 0; /**< Column of the error if one occurred. */
		String ErrorFile; /**< File in which the error occurred. Empty if root file. */
	};

	/** Meta-data for a shader. Can be used for compiling specific variations of the shader. */
	struct B3D_EXPORT ShaderCompilerMetaData : public IReflectable
	{
		String Source; /**< High level source code of the shader. */
		String NameInCache; /**< Unique name of this shader in the shader cache. */
		Array<u64, 2> ShaderHash; /**< Hash of the shader's source code (not including the include file source). */
		TInlineArray<GpuProgramType, 2> GPUProgramTypes; /**< Types of GPU programs used by the shader. */
		Vector<ShaderVariationParameters> Variations; /**< Sets of defines controlling which variations of the shader are present. */
		UnorderedMap<String, String> Defines; /**< Optional list of defines to provide when compiling the shader variations. This is added along with the shader variation defines. */

		// Note: Important this is ordered, as we create another set of hashes from this
		Map<String, Array<u64, 2>> IncludeHashes; /**< Hash value for each referenced include file. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ShaderCompilerMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Interface used for compilers that transform a source file written in a higher level shading language into a Shader and shader variations usable by the engine. */
	class B3D_EXPORT IShaderCompiler
	{
	public:
		virtual ~IShaderCompiler() = default;

		/**
		 * Compiles the shader from BSL and outputs a Shader object. Depending on the @p compileVariations parameter the shader variations will be compiled as well, or the shader will be empty and requires
		 * variations to be compiled on demand.
		 *
		 * @param		name				Name used to identify the shader.
		 * @param		source				BSL source to compile.
		 * @param		defines				An optional set of defines to set during compilation.
		 * @param		languages			Low-level languages to compile individual variations for. Each language will result in another set of variations.
		 * @param		compileVariations	If true all shader variations will be compiled. If false, you must compile the variations on demand before use.
		 * @param		outShader			Shader if the compilation is successful, null otherwise.
		 * @return							A result object containing an error message if not successful.
		 */
		virtual ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, TShared<Shader>& outShader) = 0;

		/** @copydoc Compile(const String&, const String&, const UnorderedMap<String, String>&, ShadingLanguageFlags, bool, TShared<Shader>&) */
		virtual ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, TShared<render::Shader>& outShader) = 0;

		/**
		 * Compiles a particular shader variation.
		 *
		 * @param		shader					Shader for which to compile the variation.
		 * @param		variationParameters		Specific variation to compile.
		 * @param		language				Language to compile the variation for. Must be a single language, rather than a mask of multiple languages.
		 * @param		inOutVariation			Variation on which to set the compiled data if successful.
		 * @return								A result object containing an error message if not successful.
		 */
		virtual ShaderCompilerResult CompileVariation(const Shader& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, Variation& inOutVariation) = 0;

		/** @copydoc CompileVariation(const Shader&, const ShaderVariationParameters&, ShadingLanguageFlag, ShaderCompilerMetaData, Variation&) */
		virtual ShaderCompilerResult CompileVariation(const render::Shader& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, render::Variation& inOutVariation) = 0;
	};

	/** Keeps track of all available shader compilers. */
	class B3D_EXPORT ShaderCompilers : public Module<ShaderCompilers>
	{
	public:
		/** Registers a new shader compiler for the provided language. */
		void RegisterCompiler(const String& language, const TShared<IShaderCompiler>& compiler) { mCompilers[language] = compiler; }

		/** Unregisters a shader compiler. */
		void UnregisterCompiler(const String& language) { mCompilers.erase(language); }

		/** Registers a path that will be used for looking for shader files. Thread safe. */
		void RegisterSearchPath(const Path& folder);

		/** Returns the compiler for the specified language. */
		TShared<IShaderCompiler> GetCompiler(const String& language);

		/**
		 * Attempts to retrieve a Shader object from cache or builds the Shader and adds it to the cache.
		 *
		 * @param	shaderPath		Relative or absolute path to the shader source file. If relative, search paths provided through
		 *							RegisterSearchPath() will be searched for the file.
		 * @param	cachePrefix		Folder within the cache to perform the lookup in.
		 * @param	defines			Optional set of defines to use when compiling the shader.
		 * @return					Shader object on success, or null on failure.
		 */
		template <bool IsRenderProxy>
		TShared<CoreVariantType<Shader, IsRenderProxy>> GetOrCompileShader(const Path& shaderPath, const String& cachePrefix, const ShaderDefines& defines);

		/** Detects shading language supported by the current render backend. */
		static ShadingLanguageFlag DetectActiveShadingLanguage();

		/** Converts a shading language name to a corresponding enum entry. */
		static ShadingLanguageFlag ParseShadingLanguage(const String& name);

		/** Returns the name of the provided shading language. */
		static const char* GetShadingLanguageName(ShadingLanguageFlag language);

	private:
		UnorderedMap<String, TShared<IShaderCompiler>> mCompilers;
		Vector<Path> mSearchPaths;
		Mutex mSearchPathMutex;
	};


	/** @} */
} // namespace b3d
