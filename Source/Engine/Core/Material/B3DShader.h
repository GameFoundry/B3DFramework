//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"
#include "String/B3DStringID.h"
#include "Resources/B3DResourceMetaData.h"
#include "Material/B3DVariation.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DSamplerState.h"
#include "Material/B3DShaderParameterDescription.h"

namespace b3d
{
	/** @addtogroup Material
	 *  @{
	 */

	class Shader;
	struct ShaderCompilerMetaData;

	namespace render
	{
		class Shader;
	}

	/** Shared shader interface and render settings. Finish construction before publishing to a Shader. */
	struct B3D_EXPORT ShaderDescription : IReflectable
	{
		ShaderDescription() = default;

		/** Material-facing parameters shared with shader reflection. */
		TShared<ShaderParameterDescription> Parameters = B3DMakeShared<ShaderParameterDescription>();

		/**
		 * Sorting order within a render queue priority group. Defaults to insertion order. Front-to-back sorting reduces
		 * overdraw for opaque objects; transparent objects generally require back-to-front sorting.
		 */
		QueueSortType QueueSortType = QueueSortType::None;

		/**
		 * Priority that allows you to control in what order are your shaders rendered. See QueuePriority for a list of
		 * initial values. Shaders with higher priority will be rendered before shaders with lower priority, and
		 * additionally render queue will only sort elements within the same priority group.
		 *
		 * @note
		 * This is useful when you want all your opaque objects to be rendered before you start drawing your transparent
		 * ones. Or to render your overlays after everything else. Values provided in QueuePriority are just for general
		 * guidance and feel free to increase them or decrease them for finer tuning. (for example QueuePriority::Opaque +
		 * 1).
		 */
		i32 QueuePriority = 0;

		/**
		 * Enables or disables separable passes. When separable passes are disabled all shader passes will be executed in a
		 * sequence one after another. If it is enabled the renderer is free to mix and match passes from different
		 * objects to achieve best performance. (They will still be executed in sequence, but some other object may be
		 * rendered in-between passes)
		 *
		 * @note	Shaders with transparency generally can't be separable, while opaque can.
		 */
		bool SeparablePasses = false;

		/** Flags that let the renderer know how should it interpret the shader. */
		ShaderFlags Flags = {};

		/**
		 * Information about all variation parameters and their possible values. Each permutation of variation parameters
		 * represents a separate shader variation.
		 */
		Vector<ShaderVariationParameterInformation> VariationParameters;

		friend class ShaderDescriptionRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	namespace render
	{
	/** Shader initialization data. The description is shared and must not be modified after creation. */
	struct B3D_EXPORT ShaderCreateInformation : IReflectable
	{
		ShaderCreateInformation() = default;
		explicit ShaderCreateInformation(TShared<ShaderDescription> description) : Description(std::move(description)) { }

		/** Describes shader parameters and other settings. */
		TShared<ShaderDescription> Description = B3DMakeShared<ShaderDescription>();

		/** Source and dependency metadata for compiling variations on demand. */
		TShared<ShaderCompilerMetaData> CompilerMetaData;

		/** Variations to initialize the shader with. */
		Vector<TShared<Variation>> Variations;

		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	}

	/** Shader initialization data. The description is shared and must not be modified after creation. */
	struct B3D_EXPORT ShaderCreateInformation : IReflectable
	{
		ShaderCreateInformation() = default;
		explicit ShaderCreateInformation(TShared<ShaderDescription> description) : Description(std::move(description)) { }

		/** Describes shader parameters and other settings. */
		TShared<ShaderDescription> Description = B3DMakeShared<ShaderDescription>();

		/** Source and dependency metadata for compiling variations on demand. */
		TShared<ShaderCompilerMetaData> CompilerMetaData;

		/** Variations to initialize the shader with. */
		Vector<TShared<Variation>> Variations;

		/** Shares the description and converts variation references to their render-thread counterparts. */
		static render::ShaderCreateInformation ConvertToRenderProxy(const ShaderCreateInformation& other);

		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Serializable shader description. Variation artifacts are cached separately. */
	struct B3D_EXPORT PrecompiledShaderData : IReflectable
	{
		PrecompiledShaderData() = default;

		/** Name of the shader. */
		String Name;

		/** Thread-independent parameters and render settings. */
		TShared<ShaderDescription> Description;

		/** Source and dependency metadata for compiling variations on demand. */
		TShared<ShaderCompilerMetaData> CompilerMetaData;

		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/**	Templated version of Shader used for implementing both main and render thread counterparts. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TShader
	{
	public:
		using VariationType = CoreVariantType<Variation, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using ShaderCreateInformationType = CoreVariantType<ShaderCreateInformation, IsRenderProxy>;

		TShader(u32 id);
		TShader(const ShaderCreateInformationType& createInformation, u32 id);
		virtual ~TShader();

		/** Returns the total number of variations in this shader. */
		u32 GetVariationCount() const { return (u32)mInformation.Variations.size(); }

		/** Returns the list of all supported variations based on current render API and renderer. */
		Vector<TShared<VariationType>> GetCompatibleVariations() const;

		/**
		 * Returns the list of all supported variations based on current render API and renderer, and limits the variations
		 * to only those implementing the specified variation parameters.
		 *
		 * @param		variationParameters		Object containing variation parameters to compare to check.
		 * @param		exact					When true the variation needs to have the exact number of parameters with
		 *										identical contents to the provided parameters. When false, only the provided subset
		 *										of parameters is used for comparison, while any extra parameters present in
		 *										the variation are not compared.
		 */
		Vector<TShared<VariationType>> GetCompatibleVariations(const ShaderVariationParameters& variationParameters, bool exact) const;

		/** Returns a list of all variations in this shader. */
		const Vector<TShared<VariationType>>& GetVariations() const { return mInformation.Variations; }

		/**
		 * Returns the list of all variation parameters supported by this shader, possible values of each parameter and
		 * other meta-data.
		 */
		B3D_SCRIPT_EXPORT(ExportName(VariationParams), Property(Getter))
		const Vector<ShaderVariationParameterInformation>& GetVariationParameters() const { return mInformation.Description->VariationParameters; }

		/**
		 * Returns currently active queue sort type.
		 *
		 * @see		ShaderDescription::QueueSortType
		 */
		QueueSortType GetQueueSortType() const { return mInformation.Description->QueueSortType; }

		/**
		 * Returns currently active queue priority.
		 *
		 * @see		ShaderDescription::QueuePriority
		 */
		i32 GetQueuePriority() const { return mInformation.Description->QueuePriority; }

		/**
		 * Returns if separable passes are allowed.
		 *
		 * @see		ShaderDescription::SeparablePasses
		 */
		bool GetAllowSeparablePasses() const { return mInformation.Description->SeparablePasses; }

		/**
		 * Returns flags that control how the renderer interprets the shader. Actual interpretation of the flags depends on
		 * the active renderer.
		 */
		ShaderFlags GetFlags() const { return mInformation.Description->Flags; }

		/** Returns description for a data parameter with the specified name. Returns null if it doesn't exist. */
		const ShaderDataParameterInformation* GetDataParameterDescription(const String& name) const;

		/** Returns description for a texture parameter with the specified name. Returns null if it doesn't exist. */
		const ShaderObjectParameterInformation* GetTextureParameterDescription(const String& name) const;

		/** Returns description for a sampler parameter with the specified name. Returns null if it doesn't exist. */
		const ShaderObjectParameterInformation* GetSamplerParameterDescription(const String& name) const;

		/** Returns description for a buffer parameter with the specified name. Returns null if it doesn't exist. */
		const ShaderObjectParameterInformation* GetBufferParameterInformation(const String& name) const;

		/** Checks if the parameter with the specified name exists, and is a data parameter. */
		bool HasDataParameter(const String& name) const;

		/**	Checks if the parameter with the specified name exists, and is a texture parameter. */
		bool HasTextureParameter(const String& name) const;

		/** Checks if the parameter with the specified name exists, and is a sampler parameter. */
		bool HasSamplerParameter(const String& name) const;

		/** Checks if the parameter with the specified name exists, and is a buffer parameter. */
		bool HasBufferParameter(const String& name) const;

		/** Checks if the uniform buffer with the specified name exists. */
		bool HasUniformBuffer(const String& name) const;

		/** Shared material interface, independent of the GPU layout of individual variations. */
		const TShared<ShaderParameterDescription>& GetParameterDescription() const { return mInformation.Description->Parameters; }

		/**	Returns a map of all data parameters in the shader. */
		const Map<String, ShaderDataParameterInformation>& GetDataParameters() const { return mInformation.Description->Parameters->GetDataParameters(); }

		/**	Returns a map of all texture parameters in the shader. */
		const Map<String, ShaderObjectParameterInformation>& GetTextureParameters() const { return mInformation.Description->Parameters->GetTextureParameters(); }

		/**	Returns a map of all buffer parameters in the shader. */
		const Map<String, ShaderObjectParameterInformation>& GetBufferParameters() const { return mInformation.Description->Parameters->GetBufferParameters(); }

		/** Returns a map of all sampler parameters in the shader. */
		const Map<String, ShaderObjectParameterInformation>& GetSamplerParameters() const { return mInformation.Description->Parameters->GetSamplerParameters(); }

		/** Returns a map of all uniform buffers. */
		const Map<String, ShaderUniformBufferInformation>& GetUniformBuffers() const { return mInformation.Description->Parameters->GetUniformBuffers(); }

		/** Returns a list of all parameter attributes, as referenced by individual parameters. */
		const Vector<ShaderParameterAttribute>& GetParameterAttributes() const { return mInformation.Description->Parameters->GetParameterAttributes(); }

		/**
		 * Returns a default 2D texture for a parameter that has the specified default value index (retrieved from the
		 * parameters descriptor).
		 */
		TextureType GetDefault2DTexture(u32 index) const;

		/**
		 * Returns a default 3D texture for a parameter that has the specified default value index (retrieved from the
		 * parameters descriptor).
		 */
		TextureType GetDefault3DTexture(u32 index) const;

		/**
		 * Returns a default sampler state for a parameter that has the specified default value index (retrieved from the
		 * parameters descriptor).
		 */
		TShared<SamplerState> GetDefaultSampler(u32 index) const;

		/**
		 * Returns a pointer to the internal buffer containing the default value for a data parameter that has the
		 * specified default value index (retrieved from the parameters descriptor).
		 */
		const u8* GetDefaultValue(u32 index) const;

		/** Returns the unique shader ID. */
		u32 GetShaderId() const { return mShaderId; }

		/**
		 * @name Internal
		 * @{
		 */

		/** Returns the meta-data required by the shader compiler to compile individual shader variations. */
		const TShared<ShaderCompilerMetaData>& GetCompilerMetaData() const { return mInformation.CompilerMetaData; }

		/** @} */

	protected:
		friend class ShaderRenderProxyRTTI;

		/** Shares the thread-independent shader data without copying parameter collections. */
		void FillPrecompiledData(PrecompiledShaderData& outData) const
		{
			outData.Description = mInformation.Description;
			outData.CompilerMetaData = mInformation.CompilerMetaData;
		}

		ShaderCreateInformationType mInformation;
		u32 mShaderId;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**
	 * @native
	 * Shader represents a collection of variations that control object rendering. They are used in Material%s, which can be
	 * considered as instances of a Shader. Multiple materials may share the same shader but provide different parameters to
	 * it.
	 *
	 * Shader will always choose the first supported variation based on the current render backend, render manager and other
	 * properties. So make sure to add most important variations first so you make sure they are used if they are supported.
	 * @endnative
	 *
	 * @script
	 * Contains definitions of GPU programs used for rendering, as well as a set of global parameters to control those
	 * programs.
	 * @endscript
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Shader : public Resource, public TShader<false>
	{
	public:
		/** Returns the name of the shader. */
		String GetShaderName() const { return mName; }

		/**
		 * Sets a list include file paths that are referenced by this shader.
		 *
		 * @note
		 * This is not used directly by the shader as includes are expected to be processed during GPU program and state
		 * creation, but it may be referenced by higher layers for various purposes.
		 */
		void SetIncludeFiles(const Vector<String>& includes);

		/**	Checks is the provided object type a sampler. */
		static bool IsSampler(GpuParameterObjectType type);

		/**	Checks is the provided object type a texture. */
		static bool IsTexture(GpuParameterObjectType type);

		/**	Checks is the provided object type a load/store (unordered read/write) texture. */
		static bool IsLoadStoreTexture(GpuParameterObjectType type);

		/** Checks is the provided object type a buffer. */
		static bool IsBuffer(GpuParameterObjectType type);

		/**
		 * Returns the size in bytes for a specific data type.
		 *
		 * @note	Returns 0 for variable size types like structures.
		 */
		static u32 GetDataParameterSize(GpuDataParameterType type);

		/**	Creates a new shader resource using the provided descriptor and variations. */
		static HShader Create(const String& name, const ShaderCreateInformation& createInformation);

		/**	Returns a shader object but doesn't initialize it. */
		static TShared<Shader> CreateEmpty();

		/** Computes a hash from shader source code. */
		static Array<u64, 2> ComputeHash(const String& string);

		/**
		 * Computes a hash for a shader include at the provided path.
		 *
		 * @param	path			Relative path to the include, as provided in the shader source.
		 * @return					Computed hash value.
		 */
		static Array<u64, 2> ComputeIncludeHash(const String& path);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new shader object using the provided descriptor and variations.
		 *
		 * @note	Internal method. Use Create() for normal use.
		 */
		static TShared<Shader> CreateShared(const String& name, const ShaderCreateInformation& createInformation);

		/**
		 * Reconstructs a shader from a PrecompiledShaderData snapshot, recreating the variation shells for the
		 * provided languages. The variations are left uncompiled (pass data is filled on demand).
		 *
		 * @param	data		Cached shader description created by GetPrecompiledData() in a previous session.
		 * @param	languages	Shading language identifiers to create variation shells for.
		 */
		static TShared<Shader> Create(const PrecompiledShaderData& data, const Vector<String>& languages);

		/** Extracts a snapshot of this shader. This object can be cached and used for creating the same shader. */
		TShared<PrecompiledShaderData> GetPrecompiledData() const;

		/** @} */

	private:
		Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id);

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;
		TShared<render::RenderProxy> CreateRenderProxy() const override;

	private:
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		Shader(u32 id);

	public:
		friend class ShaderRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Shader specific resource meta-data containing information about referenced include files. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() ShaderMetaData : public ResourceMetaData
	{
	public:
		B3D_SCRIPT_EXPORT()
		Vector<String> Includes;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	class ShaderRenderProxyRTTI;

	namespace render
	{
		/** @addtogroup Material-Internal
		 *  @{
		 */

		/** Render thread version of Shader. */
		class B3D_EXPORT Shader : public IReflectable, public RenderProxy, public TShader<true>
		{
		public:
			/** @copydoc b3d::Shader::Create */
			static TShared<Shader> Create(const String& name, const ShaderCreateInformation& createInformation);

			/** @copydoc b3d::Shader::Create(const PrecompiledShaderData&, const Vector<String>&) */
			static TShared<Shader> Create(const PrecompiledShaderData& data, const Vector<String>& languages);

			/** Creates an empty shader. */
			static TShared<Shader> CreateEmpty();

			/** @copydoc b3d::Shader::GetPrecompiledData */
			TShared<PrecompiledShaderData> GetPrecompiledData() const;

			/** Returns the name of the shader. */
			String GetShaderName() const { return mName; }

		protected:
			friend class b3d::Shader;

			Shader(const String& name, const ShaderCreateInformation& createInformation, u32 id);

			String mName;
			static std::atomic<u32> mNextShaderId;

		private:
			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
			Shader(u32 id);

		public:
			friend class b3d::ShaderRenderProxyRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
