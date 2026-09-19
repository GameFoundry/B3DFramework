//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "String/B3DStringID.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DSamplerState.h"
#include "Utility/B3DResult.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	/** @addtogroup Material
	 *  @{
	 */

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

		bool IsInternal = false; /**< Excludes this declaration from the material interface. */

		u32 Set = ~0u; /**< Generated descriptor set or register space; ~0u when unavailable. */
		u32 Slot = ~0u; /**< Generated resource binding, or owning uniform-buffer binding for data; ~0u when unavailable. */

		/**
		 * Optional semantic that allows you to specify the use of this parameter in the renderer. The actual value of the
		 * semantic depends on the current Renderer and its supported list of semantics. Elements with renderer semantics
		 * should not be updated by the user, and will be updated by the renderer. These semantics will also be used to
		 * determine if a shader is compatible with a specific renderer or not. Value of 0 signifies the parameter is not
		 * used by the renderer.
		 */
		StringID RendererSemantic;

		/** Index of the default value inside the ShaderParameterDescription. Should not be set externally by the user. */
		u32 DefaultValueIndex = ~0u;

		/** Index to a set of optional attributes attached to the parameter. Should not be set externally by the user. */
		u32 AttributeIndex = ~0u;
	};

	/**
	 * Describes a single data (int, Vector2, etc.) shader parameter.
	 *
	 * @see	ShaderParameterDescription::AddParameter().
	 */
	struct ShaderDataParameterInformation : ShaderParameterInformation
	{
		ShaderDataParameterInformation() = default;

		ShaderDataParameterInformation(String name, String gpuVariableName, GpuDataParameterType type, StringID rendererSemantic = StringID::kNone, u32 arraySize = 1, u32 elementSize = 0)
			: ShaderParameterInformation(std::move(name), std::move(gpuVariableName), rendererSemantic), Type(type), ArraySize(arraySize), ElementSize(elementSize)
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
	 * @see	ShaderParameterDescription::AddParameter().
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
		ShaderUniformBufferInformation() = default;

		String Name;
		bool IsInternal = false; /**< Excludes this buffer and its members from the material interface. */
		u32 Set = ~0u; /**< Generated descriptor set or register space; ~0u when unavailable. */
		u32 Slot = ~0u; /**< Generated binding or register number; ~0u when unavailable. */
		bool UsesDynamicOffset = false; /**< Enables per-draw offset changes for this binding. */
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

	/** Source parameters, generated bindings, defaults and attributes. Native bytecode reflection owns member offsets and optimized resource layouts. */
	class B3D_EXPORT ShaderParameterDescription : public IReflectable
	{
	public:
		ShaderParameterDescription() = default;

		/**
		 * Registers a new data (int, Vector2, etc.) parameter that you may then use via Material by providing the
		 * parameter name. All parameters internally map to variables defined in GPU programs.
		 *
		 * @param	parameterInformation	Structure describing the parameter to add.
		 * @param	defaultValue			(optional) Read-only bytes containing the default value for this parameter
		 *									(initial value that will be set when a material is initialized with this shader).
		 *									The provided buffer must be of the correct size (depending on the element type
		 *									and array size).
		 *
		 * @note	If multiple parameters are given with the same name but different types behavior is undefined.
		 */
		void AddParameter(ShaderDataParameterInformation parameterInformation, TArrayView<const u8> defaultValue = {});

		/**
		 * Registers a new object (texture, sampler state, etc.) parameter that you may then use via Material by
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
		 * @see	ShaderParameterDescription::AddParameter(ShaderObjectParameterInformation)
		 *
		 * @note
		 * Specialized version of AddParameter that accepts a default sampler value that will be used for initializing the
		 * object parameter upon Material creation. Default sampler value is only valid if the object type is one of the
		 * sampler types.
		 */
		void AddParameter(ShaderObjectParameterInformation parameterInformation, const SamplerStateCreateInformation& defaultValue);

		/**
		 * @see	ShaderParameterDescription::AddParameter(ShaderObjectParameterInformation)
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

		/** Assigns generated binding coordinates to an existing parameter or uniform buffer. */
		void SetParameterBinding(const String& name, u32 set, u32 slot);

		/** Sets whether a declaration is excluded from the material interface. */
		void SetParameterInternal(const String& name, bool isInternal);

		/** Registers a uniform buffer, including its generated binding and source attributes. */
		void AddUniformBuffer(ShaderUniformBufferInformation information);

		/** Combines compatible declarations and aliases. Conflicting declarations leave the description unchanged. */
		Result TryCombine(const ShaderParameterDescription& other);

		/**
		 * Checks that the material-visible declarations across @p passParameters match this material interface.
		 * Ignores internal declarations, binding coordinates and native resource usage. Does not modify either description.
		 */
		Result ValidateMaterialInterface(TArrayView<const ShaderParameterDescription*> passParameters) const;

		/** Returns the data parameters owned by this description. */
		const Map<String, ShaderDataParameterInformation>& GetDataParameters() const { return mDataParameters; }

		/** Returns the texture parameters owned by this description. */
		const Map<String, ShaderObjectParameterInformation>& GetTextureParameters() const { return mTextureParameters; }

		/** Returns the buffer parameters owned by this description. */
		const Map<String, ShaderObjectParameterInformation>& GetBufferParameters() const { return mBufferParameters; }

		/** Returns the sampler parameters owned by this description. */
		const Map<String, ShaderObjectParameterInformation>& GetSamplerParameters() const { return mSamplerParameters; }

		/** Returns the uniform buffers owned by this description. */
		const Map<String, ShaderUniformBufferInformation>& GetUniformBuffers() const { return mUniformBuffers; }

		/** Returns the data default values owned by this description. */
		const Vector<u8>& GetDataDefaultValues() const { return mDataDefaultValues; }

		/** Returns the sampler default values owned by this description. */
		const Vector<SamplerStateInformation>& GetSamplerDefaultValues() const { return mSamplerDefaultValues; }

		/** Returns the texture default values owned by this description. */
		const Vector<ShaderDefaultTextureType>& GetTextureDefaultValues() const { return mTextureDefaultValues; }

		/** Returns the parameter attributes owned by this description. */
		const Vector<ShaderParameterAttribute>& GetParameterAttributes() const { return mParameterAttributes; }

		friend class ShaderParameterDescriptionRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	private:
		/** Registers an object parameter and preserves its existing defaults when adding an alias. */
		void AddParameterInternal(ShaderObjectParameterInformation parameterInformation, u32 defaultValueIndex);

	private:
		Map<String, ShaderDataParameterInformation> mDataParameters;
		Map<String, ShaderObjectParameterInformation> mTextureParameters;
		Map<String, ShaderObjectParameterInformation> mBufferParameters;
		Map<String, ShaderObjectParameterInformation> mSamplerParameters;
		Map<String, ShaderUniformBufferInformation> mUniformBuffers;
		Vector<u8> mDataDefaultValues;
		Vector<SamplerStateInformation> mSamplerDefaultValues;
		Vector<ShaderDefaultTextureType> mTextureDefaultValues;
		Vector<ShaderParameterAttribute> mParameterAttributes;
	};

	/** @} */
}
