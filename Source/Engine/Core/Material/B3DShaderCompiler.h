//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Material/B3DShader.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuBackend.h"

namespace b3d
{
	struct BSLParsedShaderMetaData;
	struct BSLParsedShaderData;

	/** @addtogroup Material-Internal
	 *  @{
	 */

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
		 * @param		languages			Low-level shading language identifiers (for example "hlsl", "vksl") to compile individual variations for. Each language will result in another set of variations.
		 * @param		compileVariations	If true all shader variations will be compiled. If false, you must compile the variations on demand before use.
		 * @param		outShader			Shader if the compilation is successful, null otherwise.
		 * @return							A result object containing an error message if not successful.
		 */
		virtual ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, const Vector<String>& languages, bool compileVariations, TShared<Shader>& outShader) = 0;

		/** @copydoc Compile(const String&, const String&, const UnorderedMap<String, String>&, const Vector<String>&, bool, TShared<Shader>&) */
		virtual ShaderCompilerResult Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, const Vector<String>& languages, bool compileVariations, TShared<render::Shader>& outShader) = 0;

		/**
		 * Compiles a particular shader variation.
		 *
		 * @param		shader					Shader for which to compile the variation.
		 * @param		variationParameters		Specific variation to compile.
		 * @param		language				Identifier of the single language to compile the variation for (for example "hlsl", "vksl").
		 * @param		inOutVariation			Variation on which to set the compiled data if successful.
		 * @return								A result object containing an error message if not successful.
		 */
		virtual ShaderCompilerResult CompileVariation(const Shader& shader, const ShaderVariationParameters& variationParameters, const String& language, Variation& inOutVariation) = 0;

		/** @copydoc CompileVariation(const Shader&, const ShaderVariationParameters&, const String&, Variation&) */
		virtual ShaderCompilerResult CompileVariation(const render::Shader& shader, const ShaderVariationParameters& variationParameters, const String& language, render::Variation& inOutVariation) = 0;
	};

	/**
	 * Interface used for compilers that turn a GPU program written in a backend-native high-level shading language (for example "vksl")
	 * into the intermediate bytecode consumed by the GPU driver (for example SPIR-V).
	 */
	class B3D_EXPORT IGpuBytecodeCompiler
	{
	public:
		virtual ~IGpuBytecodeCompiler() = default;

		/**
		 * Compiles a single GPU program described by @p createInformation into an intermediate bytecode format.
		 *
		 * @param	createInformation	GPU program source and metadata to compile.
		 * @return						Compiled bytecode, or null on failure.
		 */
		virtual TShared<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& createInformation) = 0;

		/**
		 * Returns true if @p bytecode was produced by a version of this compiler compatible with the current one.
		 *
		 * @param	bytecode	Previously produced bytecode to test for staleness.
		 * @return				True if the bytecode is current, false if it should be recompiled.
		 */
		virtual bool IsUpToDate(const GpuProgramBytecode& bytecode) const = 0;
	};

	/** Keeps track of all available shader compilers. */
	class B3D_EXPORT ShaderCompilers : public Module<ShaderCompilers>
	{
	public:
		ShaderCompilers();

		/** Registers a new shader compiler for the provided language. Thread safe. */
		void RegisterCompiler(const String& language, const TShared<IShaderCompiler>& compiler)
		{
			Lock lock(mCompilerMutex);
			mCompilers[language] = compiler;
		}

		/** Unregisters a shader compiler. Thread safe. */
		void UnregisterCompiler(const String& language)
		{
			Lock lock(mCompilerMutex);
			mCompilers.erase(language);
		}

		/**
		 * Registers a GPU bytecode compiler for the provided shading language (for example "vksl").
		 * Replaces any compiler previously registered for the same language. Thread safe.
		 */
		void RegisterBytecodeCompiler(const String& language, const TShared<IGpuBytecodeCompiler>& compiler)
		{
			Lock lock(mCompilerMutex);
			mBytecodeCompilers[language] = compiler;
		}

		/** Unregisters a GPU bytecode compiler. Thread safe. */
		void UnregisterBytecodeCompiler(const String& language)
		{
			Lock lock(mCompilerMutex);
			mBytecodeCompilers.erase(language);
		}

		/** Registers a low-level shading language that we can cross-compile to (for example "vksl"). Thread safe. */
		void RegisterShadingLanguage(const String& language);

		/** Returns the compiler for the specified language. Thread safe. */
		TShared<IShaderCompiler> GetCompiler(const String& language);

		/** Returns the GPU bytecode compiler for the specified low-level target shading language, or null if none is registered. Thread safe. */
		TShared<IGpuBytecodeCompiler> GetBytecodeCompiler(const String& language);

		/**
		 * Compiles a Shader object from high-level (BSL) source. Note this only compiles the shader meta-data, you must
		 * also call CompileVariation to compile the actual source code for a shader variation.
		 *
		 * @param	name		Name used to identify the shader.
		 * @param	source		High-level (BSL) source to compile.
		 * @param	defines		Optional set of defines to use when compiling the shader.
		 * @param	languages	Low-level shading language identifiers (for example "vksl") to compile variations for.
		 * @return				Shader object on success, or null on failure (an error is logged in that case).
		 */
		template <bool IsRenderProxy>
		TShared<CoreVariantType<Shader, IsRenderProxy>> CompileShader(const String& name, const String& source, const ShaderDefines& defines, const Vector<String>& languages);

		/**
		 * Detects the shading language identifier supported by the current render backend. Returns an empty string
		 * if no registered language is supported (or no device is available). Thread safe.
		 */
		String DetectActiveShadingLanguage() const;

	private:
		UnorderedMap<String, TShared<IShaderCompiler>> mCompilers;
		UnorderedMap<String, TShared<IGpuBytecodeCompiler>> mBytecodeCompilers;
		mutable Mutex mCompilerMutex;

		Vector<String> mShadingLanguages;
		mutable Mutex mShadingLanguageMutex;
	};


	/** @} */
} // namespace b3d
