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

	/** Information about a shader parameter. */
	struct ShaderParameterInformation
	{
		ShaderParameterInformation() = default;

		ShaderParameterInformation(String name, String gpuVariableName, StringID rendererSemantic = StringID::kNone)
			: Name(std::move(name)), GpuVariableName(std::move(gpuVariableName)), RendererSemantic(rendererSemantic)
		{}

		/** The name of the parameter. Name must be unique between all data and object parameters in a shader. */
		String Name;

		/** Name of the GPU variable in the GpuProgram that the parameter corresponds with. */
		String GpuVariableName;

		/**
		 * Optional semantic that allows you to specify the use of this parameter in the renderer. The actual value of the
		 * semantic depends on the current Renderer and its supported list of semantics. Elements with renderer semantics
		 * should not be updated by the user, and will be updated by the renderer. These semantics will also be used to
		 * determine if a shader is compatible with a specific renderer or not. Value of 0 signifies the parameter is not
		 * used by the renderer.
		 */
		StringID RendererSemantic;

		/** Index of the default value inside the Shader. Should not be set externally by the user. */
		u32 DefaultValueIndex = ~0u;

		/** Index to a set of optional attributes attached to the parameter. Should not be set externally by the user. */
		u32 AttributeIndex = ~0u;
	};

	/**
	 * Describes a single data (int, Vector2, etc.) shader parameter.
	 *
	 * @see	Shader::AddParameter().
	 */
	struct ShaderDataParameterInformation : ShaderParameterInformation
	{
		ShaderDataParameterInformation() = default;

		ShaderDataParameterInformation(String name, String gpuVariableName, GpuDataParameterType type, StringID rendererSemantic = StringID::kNone, u32 arraySize = 1, u32 elementSize = 0)
			: ShaderParameterInformation(std::move(name), std::move(gpuVariableName), rendererSemantic)
			, Type(type) , ArraySize(arraySize) , ElementSize(elementSize)
		{}

		/** The type of the parameter, must be the same as the type in GpuProgram. */
		GpuDataParameterType Type = GPDT_FLOAT1;

		/** If the parameter is an array, the number of elements in the array. Size of 1 means its not an array. */
		u32 ArraySize = 1;

		/**
		 * Size of an individual element in the array, in bytes. You only need to set this if you are setting variable
		 * length parameters, like structs. Otherwise the size is determined from the type.
		 */
		u32 ElementSize = 0;
	};

	/**
	 * Describes a single object (texture, sampler state, etc.) shader parameter.
	 *
	 * @see	Shader::AddParameter().
	 */
	struct ShaderObjectParameterInformation : ShaderParameterInformation
	{
		ShaderObjectParameterInformation() = default;

		ShaderObjectParameterInformation(String name, String gpuVariableName, GpuParameterObjectType type, StringID rendererSemantic = StringID::kNone, u32 arraySize = 1)
			: ShaderParameterInformation(std::move(name), gpuVariableName, rendererSemantic), Type(type), ArraySize(arraySize)
		{
			GpuVariableNames.emplace_back(gpuVariableName);
		}

		GpuParameterObjectType Type = GPOT_TEXTURE2D; /**< The type of the parameter, must be the same as the type in GpuProgram. */
		u32 ArraySize = 1; /**< Number of elements in the array, if the parameter is an array. */
		Vector<String> GpuVariableNames; /**< Names of all GPU variables this shader parameter maps to. */
	};

	/** Describes a shader uniform buffer. */
	struct ShaderUniformBufferInformation
	{
		String Name;
		bool Shared = false;
		StringID RendererSemantic;
		GpuBufferFlags Flags = GpuBufferFlag::StoreOnGPU;
	};

	/** Available attribute types that can be assigned to Shader parameters. */
	enum class ShaderParamAttributeType
	{
		/**
		 * Selects a 4D vector to use for storing UV offset and size when rendering a subset of a larger texture (e.g.
		 * when attaching a SpriteTexture to the material parameter). The attribute value is a string naming the texture
		 * parameter that contains the texture whose subset the UV represents.
		 */
		SpriteUV,

		/** Specifies a human readable name of the shader parameter. */
		Name,

		/** Hides the parameter from the display in editor inspector. */
		HideInInspector,

		/** Notifies the system the parameter is a HDR color. */
		HDR
	};

	/** Optional attribute that can be applied to a shader parameter. */
	struct ShaderParameterAttribute
	{
		/** Type of the attribute. */
		ShaderParamAttributeType Type = (ShaderParamAttributeType)0;

		/** Value of the parameter encoded as a string. */
		String Value;

		/** Index of the next attribute in the linked list for this parameter. Should not be set externally by the user. */
		u32 NextParameterIndex = ~0u;
	};

	/** Represents a single potential value of a shader variation parameter and optionally its name. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Renderer), ExportAsStruct(true)) ShaderVariationParameterValue
	{
		/** Optional human-readable name describing what this particular value represents. */
		String Name;

		/** Integer value of the parameter. */
		i32 Value = 0;
	};

	/** Represents a single shader variation parameter and a set of all possible values. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Renderer), ExportAsStruct(true)) ShaderVariationParameterInformation
	{
		/** Optional human-readable name describing the variation parameter. */
		String Name;

		/** BSL identifier for the parameter. */
		String Identifier;

		/** True if the parameter is for internal use by the renderer, and false if its intended to be set by the user. */
		bool IsInternal = true;

		/** A list of potential values this parameter can take on. */
		TInlineArray<ShaderVariationParameterValue, 4> Values;
	};

	/** @} */

	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Built-in texture types that can be assigned as default values for shader texture inputs. */
	enum class ShaderDefaultTextureType
	{
		None,
		White,
		Black,
		Normal
	};

	/** Structure used for initializing a shader. */
	struct B3D_EXPORT ShaderInformationBase : public IReflectable
	{
		ShaderInformationBase();

		/**
		 * Registers a new data (int, Vector2, etc.) parameter you that you may then use via Material by providing the
		 * parameter name. All parameters internally map to variables defined in GPU programs.
		 *
		 * @param	parameterInformation	Structure describing the parameter to add.
		 * @param	defaultValue			(optional) Pointer to the buffer containing the default value for this parameter
		 *									(initial value that will be set when a material is initialized with this shader).
		 *									The provided buffer must be of the correct size (depending on the element type
		 *									and array size).
		 *
		 * @note	If multiple parameters are given with the same name but different types behavior is undefined.
		 */
		void AddParameter(ShaderDataParameterInformation parameterInformation, u8* defaultValue = nullptr);

		/**
		 * Registers a new object (texture, sampler state, etc.) parameter you that you may then use via Material by
		 * providing the parameter name. All parameters internally map to variables defined in GPU programs. Multiple GPU
		 * variables may be mapped to a single parameter in which case the first variable actually found in the program will
		 * be used while others will be ignored.
		 *
		 * @param	parameterInformation	Structure describing the parameter to add.
		 *
		 * @note
		 * If multiple parameters are given with the same name but different types behavior is undefined. You are allowed
		 * to call this method multiple times in order to map multiple GPU variable names to a single parameter, but the
		 * default value (if any) will only be recognized on the first call. Mapping multiple GPU variables to a single
		 * parameter is useful when you are defining a shader that supports variations across different render systems
		 * where GPU variable names for the same parameters might differ.
		 */
		void AddParameter(ShaderObjectParameterInformation parameterInformation);

		/**
		 * @see	ShaderInformationBase::AddParameter(ShaderObjectParameterInformation)
		 *
		 * @note
		 * Specialized version of AddParameter that accepts a default sampler value that will be used for initializing the
		 * object parameter upon Material creation. Default sampler value is only valid if the object type is one of the
		 * sampler types.
		 */
		void AddParameter(ShaderObjectParameterInformation parameterInformation, const SamplerStateCreateInformation& defaultValue);

		/**
		 * @see	ShaderInformationBase::AddParameter(ShaderObjectParameterInformation)
		 *
		 * @note
		 * Specialized version of AddParameter that accepts a default texture value that will be used for initializing the
		 * object parameter upon Material creation. Default texture value is only valid if the object type is one of the
		 * texture types.
		 */
		void AddParameter(ShaderObjectParameterInformation parameterInformation, ShaderDefaultTextureType defaultValue);

		/**
		 * Applies an attribute to the parameter with the specified name.
		 *
		 * @param	name		Name of an object or data parameter to apply the attribute to.
		 * @param	attribute	Structure describing the attribute to apply.
		 */
		void SetParameterAttribute(const String& name, const ShaderParameterAttribute& attribute);

		/**
		 * Changes parameters of a uniform buffer with the specified name.
		 *
		 * @param	name				Name of the uniform buffer. This should correspond with the name specified in
		 *								the GPU program code.
		 * @param	shared				If uniform buffer is marked as shared it will not be automatically created by
		 *								the Material. You will need to create it elsewhere and then assign it manually.
		 * @param	flags				Flags that control the behaviour of the uniform buffer.
		 * @param	rendererSemantic	(optional) Semantic that allows you to specify the use of this uniform buffer
		 *								in the renderer. The actual value of the semantic depends on the current
		 *								Renderer and its supported list of semantics. Elements with a renderer semantic
		 *								will not have their uniform buffer automatically created (similar to "shared"
		 *								argument), but instead a Renderer will create an assign it instead. Be aware
		 *								that renderers have strict policies on what and how are parameters stored in the
		 *								buffer and you will need to respect them. If you don't respect them your shader
		 *								will be deemed incompatible and won't be used. Value of 0 signifies the uniform
		 *								buffer is not used by the renderer.
		 */
		void SetUniformBufferAttributes(const String& name, bool shared, GpuBufferFlags flags, StringID rendererSemantic = StringID::kNone);

		/**
		 * Sorting type to use when performing sort in the render queue. Default value is sort front to back which causes
		 * least overdraw and is preferable. Transparent objects need to be sorted back to front. You may also specify no
		 * sorting and the elements will be rendered in the order they were added to the render queue.
		 */
		QueueSortType QueueSortType;

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
		i32 QueuePriority;

		/**
		 * Enables or disables separable passes. When separable passes are disabled all shader passes will be executed in a
		 * sequence one after another. If it is disabled the renderer is free to mix and match passes from different
		 * objects to achieve best performance. (They will still be executed in sequence, but some other object may be
		 * rendered in-between passes)
		 *
		 * @note	Shaders with transparency generally can't be separable, while opaque can.
		 */
		bool SeparablePasses;

		/** Flags that let the renderer know how should it interpret the shader. */
		ShaderFlags Flags;

		/**
		 * Information about all variation parameters and their possible values. Each permutation of variation parameters
		 * represents a separate shader variation.
		 */
		Vector<ShaderVariationParameterInformation> VariationParameters;

		/** Meta-data required by the shader compiler when compiling shader variations on demand. Can be null if the shader is being initialized with precompiled variations. */
		TShared<ShaderCompilerMetaData> CompilerMetaData;

		Map<String, ShaderDataParameterInformation> DataParameters;
		Map<String, ShaderObjectParameterInformation> TextureParameters;
		Map<String, ShaderObjectParameterInformation> BufferParameters;
		Map<String, ShaderObjectParameterInformation> SamplerParameters;
		Map<String, ShaderUniformBufferInformation> UniformBuffers;

		Vector<u8> DataDefaultValues;
		Vector<SamplerStateInformation> SamplerDefaultValues;
		Vector<ShaderDefaultTextureType> TextureDefaultValues;
		Vector<ShaderParameterAttribute> ParameterAttributes;

	private:
		/**
		 * @copydoc	AddParameter(ShaderObjectParameterInformation)
		 *
		 * @note	Common method shared by different addParameter overloads.
		 */
		void AddParameterInternal(ShaderObjectParameterInformation parameterInformation, u32 defaultValueIndex);

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderInformationBaseRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	class ShaderInformationRenderProxyRTTI;

	namespace render
	{
	struct B3D_EXPORT ShaderInformation : ShaderInformationBase
	{
		ShaderInformation() = default;

		/** Variations to initialize the shader with. */
		Vector<TShared<Variation>> Variations;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class b3d::ShaderInformationRenderProxyRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Descriptor structure used for initialization of a Shader. */
	struct ShaderCreateInformation : ShaderInformation
	{
		ShaderCreateInformation() = default;
		ShaderCreateInformation(const ShaderInformation& other)
			:ShaderInformation(other)
		{ }
	};
	} // namespace render

	struct B3D_EXPORT ShaderInformation : ShaderInformationBase
	{
		ShaderInformation() = default;

		/** Converts object to the render thread variant. */
		static render::ShaderInformation ConvertToRenderProxy(const ShaderInformation& other);

		/** Variations to initialize the shader with. */
		Vector<TShared<Variation>> Variations;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderInformationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	class PrebuiltShaderDataRTTI;

	/**
	 * Thread-agnostic, serializable snapshot of a compiled shader, excluding the per-variation objects (which are cached
	 * separately). Used to cache a shader once and reconstruct either the main or render-thread Shader from it, so the same
	 * cache entry serves both threads.
	 */
	struct B3D_EXPORT PrebuiltShaderData : ShaderInformationBase
	{
		PrebuiltShaderData() = default;

		/** Name of the shader. */
		String Name;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PrebuiltShaderDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/** @name Material
	 *  @{
	 */

	/** Descriptor structure used for initialization of a Shader. */
	struct ShaderCreateInformation : ShaderInformation
	{
		ShaderCreateInformation() = default;
		ShaderCreateInformation(const ShaderInformation& other)
			:ShaderInformation(other)
		{ }
	};

	/** @} */

	/**	Templated version of Shader used for implementing both main and render thread counterparts. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TShader
	{
	public:
		using VariationType = CoreVariantType<Variation, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using ShaderInformationType = CoreVariantType<ShaderInformation, IsRenderProxy>;
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
		const Vector<ShaderVariationParameterInformation>& GetVariationParameters() const { return mInformation.VariationParameters; }

		/**
		 * Returns currently active queue sort type.
		 *
		 * @see		ShaderCreateInformation::QueueSortType
		 */
		QueueSortType GetQueueSortType() const { return mInformation.QueueSortType; }

		/**
		 * Returns currently active queue priority.
		 *
		 * @see		ShaderCreateInformation::QueuePriority
		 */
		i32 GetQueuePriority() const { return mInformation.QueuePriority; }

		/**
		 * Returns if separable passes are allowed.
		 *
		 * @see		ShaderCreateInformation::SeparablePasses
		 */
		bool GetAllowSeparablePasses() const { return mInformation.SeparablePasses; }

		/**
		 * Returns flags that control how the renderer interprets the shader. Actual interpretation of the flags depends on
		 * the active renderer.
		 */
		ShaderFlags GetFlags() const { return mInformation.Flags; }

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

		/**	Returns a map of all data parameters in the shader. */
		const Map<String, ShaderDataParameterInformation>& GetDataParameters() const { return mInformation.DataParameters; }

		/**	Returns a map of all texture parameters in the shader. */
		const Map<String, ShaderObjectParameterInformation>& GetTextureParameters() const { return mInformation.TextureParameters; }

		/**	Returns a map of all buffer parameters in the shader. */
		const Map<String, ShaderObjectParameterInformation>& GetBufferParameters() const { return mInformation.BufferParameters; }

		/** Returns a map of all sampler parameters in the shader. */
		const Map<String, ShaderObjectParameterInformation>& GetSamplerParameters() const { return mInformation.SamplerParameters; }

		/** Returns a map of all uniform buffers. */
		const Map<String, ShaderUniformBufferInformation>& GetUniformBuffers() const { return mInformation.UniformBuffers; }

		/** Returns a list of all parameter attributes, as referenced by individual parameters. */
		const Vector<ShaderParameterAttribute>& GetParameterAttributes() const { return mInformation.ParameterAttributes; }

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
		u8* GetDefaultValue(u32 index) const;

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

		/**
		 * Copies the thread-agnostic shader description into @p out, slicing away the per-variation objects (which are
		 * cached separately). Shared by both the main and render-thread GetPrebuiltData() implementations.
		 */
		void FillPrebuiltData(ShaderInformationBase& out) const { out = mInformation; }

		ShaderInformationType mInformation;
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
		 * Reconstructs a shader from a PrebuiltShaderData snapshot, recreating the variation shells for the
		 * provided languages. The variations are left uncompiled (pass data is filled on demand).
		 *
		 * @param	data		Cached shader description created by GetPrebuiltData() in a previous session.
		 * @param	languages	Shading language identifiers to create variation shells for.
		 */
		static TShared<Shader> Create(const PrebuiltShaderData& data, const Vector<String>& languages);

		/** Extracts a snapshot of this shader. This object can be cached and used for creating the same shader. */
		TShared<PrebuiltShaderData> GetPrebuiltData() const;

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

			/** @copydoc b3d::Shader::Create(const PrebuiltShaderData&, const Vector<String>&) */
			static TShared<Shader> Create(const PrebuiltShaderData& data, const Vector<String>& languages);

			/** Creates an empty shader. */
			static TShared<Shader> CreateEmpty();

			/** @copydoc b3d::Shader::GetPrebuiltData */
			TShared<PrebuiltShaderData> GetPrebuiltData() const;

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
