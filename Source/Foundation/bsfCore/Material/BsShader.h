//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "String/BsStringID.h"
#include "Resources/BsResourceMetaData.h"
#include "Material/BsTechnique.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	class Shader;

	namespace ct
	{
		class Shader;
	}

	/** Templated version of SubShader that can be used for both core and sim threads. */
	template<bool Core>
	struct TSubShader
	{
		using TechniqueType = CoreVariantType<Technique, Core>;
		using ShaderType = SPtr<CoreVariantType<Shader, Core>>;
		
		String name;
		ShaderType shader;
	};

	/** Shared memebers between SHADER_DATA_PARAM_DESC and SHADER_OBJECT_PARAM_DESC */
	struct SHADER_PARAM_COMMON
	{
		SHADER_PARAM_COMMON() = default;
		SHADER_PARAM_COMMON(String name, String gpuVariableName, StringID rendererSemantic = StringID::NONE)
			: name(std::move(name)), gpuVariableName(gpuVariableName), rendererSemantic(rendererSemantic)
		{ }

		/** The name of the parameter. Name must be unique between all data and object parameters in a shader. */
		String name;

		/** Name of the GPU variable in the GpuProgram that the parameter corresponds with. */
		String gpuVariableName;

		/** 
		 * Optional semantic that allows you to specify the use of this parameter in the renderer. The actual value of the
		 * semantic depends on the current Renderer and its supported list of semantics. Elements with renderer semantics 
		 * should not be updated by the user, and will be updated by the renderer. These semantics will also be used to 
		 * determine if a shader is compatible with a specific renderer or not. Value of 0 signifies the parameter is not
		 * used by the renderer.
		 */
		StringID rendererSemantic;

		/** Index of the default value inside the Shader. Should not be set externally by the user. */
		UINT32 defaultValueIdx = (UINT32)-1;

		/** Index to a set of optional attributes attached to the parameter. Should not be set externally by the user. */
		UINT32 attribIdx = (UINT32)-1;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**
	 * Describes a single data (int, Vector2, etc.) shader parameter.
	 *
	 * @see	Shader::addParameter().
	 */
	struct SHADER_DATA_PARAM_DESC : SHADER_PARAM_COMMON
	{
		SHADER_DATA_PARAM_DESC() = default;
		SHADER_DATA_PARAM_DESC(String name, String gpuVariableName, GpuParamDataType type, 
			StringID rendererSemantic = StringID::NONE, UINT32 arraySize = 1, UINT32 elementSize = 0)
			:SHADER_PARAM_COMMON(std::move(name), std::move(gpuVariableName), rendererSemantic)
			, type(type), arraySize(arraySize), elementSize(elementSize)
		{ }

		/** The type of the parameter, must be the same as the type in GpuProgram. */
		GpuParamDataType type = GPDT_FLOAT1;

		/** If the parameter is an array, the number of elements in the array. Size of 1 means its not an array. */
		UINT32 arraySize = 1;

		/** 
		 * Size of an individual element in the array, in bytes. You only need to set this if you are setting variable
		 * length parameters, like structs. Otherwise the size is determined from the type. 
		 */
		UINT32 elementSize = 0;
	};

	/**
	 * Describes a single object (texture, sampler state, etc.) shader parameter.
	 *
	 * @see	Shader::addParameter().
	 */
	struct SHADER_OBJECT_PARAM_DESC : SHADER_PARAM_COMMON
	{
		SHADER_OBJECT_PARAM_DESC() = default;
		SHADER_OBJECT_PARAM_DESC(String name, String gpuVariableName, GpuParamObjectType type, 
			StringID rendererSemantic = StringID::NONE)
			:SHADER_PARAM_COMMON(std::move(name), gpuVariableName, rendererSemantic), type(type)
		{
			gpuVariableNames.emplace_back(gpuVariableName);
		}

		/** The type of the parameter, must be the same as the type in GpuProgram. */
		GpuParamObjectType type = GPOT_TEXTURE2D;

		/** Names of all GPU variables this shader parameter maps to. */
		Vector<String> gpuVariableNames;
	};

	/** Describes a shader parameter block. */
	struct SHADER_PARAM_BLOCK_DESC
	{
		String name;
		bool shared;
		StringID rendererSemantic;
		GpuBufferUsage usage;
	};

	/** Available attribute types that can be assigned to Shader parameters. */
	enum class ShaderParamAttributeType
	{
		/** 
		 * Selects a 4D vector to use for storing UV offset and size when rendering a subset of a larger texture (e.g.
		 * when attaching a SpriteTexture to the material parameter). The attribute value is a string naming the texture
		 * parameter that contains the texture whose subset the UV represents.
		 */
		SpriteUV
	};

	/** Optional attribute that can be applied to a shader parameter. */
	struct SHADER_PARAM_ATTRIBUTE
	{
		/** Type of the attribute. */
		ShaderParamAttributeType type = (ShaderParamAttributeType)0;

		/** Value of the parameter encoded as a string. */
		String value;

		/** Index of the next attribute in the linked list for this parameter. Should not be set externally by the user. */
		UINT32 nextParamIdx = (UINT32)-1;
	};

	/** 
	 * Sub-shader represents a set of techniques not used by the main shader, but rather a specialized set of techniques
	 * used by the renderer for a specific purpose. The renderer identifies these techniques by a unique name, and utilizes
	 * them when present, or uses the default built-in techniques otherwise. Note that sub-shader techniques need to follow
	 * a specific interface that can be utilized by the renderer, usually similar/identical to the default built-in
	 * technique. 
	 */
	struct BS_CORE_EXPORT SubShader : TSubShader<false>, IReflectable
	{
		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class SubShaderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	template<bool Core> struct TSubShaderType {};
	template<> struct TSubShaderType < false > { typedef SubShader Type; };
	template<> struct TSubShaderType < true > { typedef TSubShader<true> Type; };

	/** Structure used for initializing a shader. */
	template<bool Core>
	struct BS_CORE_EXPORT TSHADER_DESC
	{
		using TextureType = CoreVariantHandleType<Texture, Core>;
		using SamplerStateType = SPtr<CoreVariantType<SamplerState, Core>>;
		using TechniqueType = CoreVariantType<Technique, Core>;
		using SubShaderType = typename TSubShaderType<Core>::Type ;

		TSHADER_DESC();

		/**
		 * Registers a new data (int, Vector2, etc.) parameter you that you may then use via Material by providing the 
		 * parameter name. All parameters internally map to variables defined in GPU programs.
		 *
		 * @param[in]	paramDesc			Structure describing the parameter to add. 
		 * @param[in]	defaultValue		(optional) Pointer to the buffer containing the default value for this parameter
		 *									(initial value that will be set when a material is initialized with this shader). 
		 *									The provided buffer must be of the correct size (depending on the element type 
		 *									and array size).
		 *
		 * @note	If multiple parameters are given with the same name but different types behavior is undefined.
		 */
		void addParameter(SHADER_DATA_PARAM_DESC paramDesc, UINT8* defaultValue = nullptr);

		/**
		 * Registers a new object (texture, sampler state, etc.) parameter you that you may then use via Material by 
		 * providing the parameter name. All parameters internally map to variables defined in GPU programs. Multiple GPU 
		 * variables may be mapped to a single parameter in which case the first variable actually found in the program will
		 * be used while others will be ignored.
		 *
		 * @param[in]	paramDesc			Structure describing the parameter to add. 
		 *
		 * @note	
		 * If multiple parameters are given with the same name but different types behavior is undefined. You are allowed 
		 * to call this method multiple times in order to map multiple GPU variable names to a single parameter, but the 
		 * default value (if any) will only be recognized on the first call. Mapping multiple GPU variables to a single 
		 * parameter is useful when you are defining a shader that supports techniques across different render systems 
		 * where GPU variable names for the same parameters might differ.
		 */
		void addParameter(SHADER_OBJECT_PARAM_DESC paramDesc);

		/**
		 * @see	SHADER_DESC::addParameter(SHADER_OBJECT_PARAM_DESC)
		 *
		 * @note	
		 * Specialized version of addParameter that accepts a default sampler value that will be used for initializing the 
		 * object parameter upon Material creation. Default sampler value is only valid if the object type is one of the 
		 * sampler types.
		 */
		void addParameter(SHADER_OBJECT_PARAM_DESC paramDesc, const SamplerStateType& defaultValue);

		/**
		 * @see	SHADER_DESC::addParameter(SHADER_OBJECT_PARAM_DESC)
		 *
		 * @note	
		 * Specialized version of addParameter that accepts a default texture value that will be used for initializing the 
		 * object parameter upon Material creation. Default texture value is only valid if the object type is one of the 
		 * texture types.
		 */
		void addParameter(SHADER_OBJECT_PARAM_DESC paramDesc, const TextureType& defaultValue);

		/** 
		 * Applies an attribute to the parameter with the specified name. 
		 *
		 * @param[in]	name	Name of an object or data parameter to apply the attribute to.
		 * @param[in]	attrib	Structure describing the attribute to apply.
		 */
		void setParameterAttribute(const String& name, const SHADER_PARAM_ATTRIBUTE& attrib);

		/**
		 * Changes parameters of a parameter block with the specified name.
		 *
		 * @param[in]	name				Name of the parameter block. This should correspond with the name specified in 
		 *									the GPU program code.
		 * @param[in]	shared				If parameter block is marked as shared it will not be automatically created by 
		 *									the Material. You will need to create it elsewhere and then assign it manually.
		 * @param[in]	usage				Specified how often do we plan on modifying the buffer, which determines how is 
		 *									the buffer internally stored for best performance.
		 * @param[in]	rendererSemantic	(optional) Semantic that allows you to specify the use of this parameter block 
		 *									in the renderer. The actual value of the semantic depends on the current 
		 *									Renderer and its supported list of semantics. Elements with a renderer semantic
		 *									will not have their parameter block automatically created (similar to "shared" 
		 *									argument), but instead a Renderer will create an assign it instead. Be aware 
		 *									that renderers have strict policies on what and how are parameters stored in the 
		 *									buffer and you will need to respect them. If you don't respect them your shader 
		 *									will be deemed incompatible and won't be used. Value of 0 signifies the parameter
		 *									block is not used by the renderer.
		 */
		void setParamBlockAttribs(const String& name, bool shared, GpuBufferUsage usage, 
			StringID rendererSemantic = StringID::NONE);

		/**
		 * Sorting type to use when performing sort in the render queue. Default value is sort front to back which causes
		 * least overdraw and is preferable. Transparent objects need to be sorted back to front. You may also specify no 
		 * sorting and the elements will be rendered in the order they were added to the render queue.
		 */
		QueueSortType queueSortType;

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
		INT32 queuePriority;

		/**
		 * Enables or disables separable passes. When separable passes are disabled all shader passes will be executed in a
		 * sequence one after another. If it is disabled the renderer is free to mix and match passes from different 
		 * objects to achieve best performance. (They will still be executed in sequence, but some other object may be 
		 * rendered in-between passes)
		 *
		 * @note	Shaders with transparency generally can't be separable, while opaque can.
		 */
		bool separablePasses;

		/** Flags that let the renderer know how should it interpret the shader. */
		ShaderFlags flags;

		/** Techniques to initialize the shader with. */
		Vector<SPtr<TechniqueType>> techniques;

		/** Optional set of sub-shaders to initialize the shader with. */
		Vector<SubShaderType> subShaders;

		Map<String, SHADER_DATA_PARAM_DESC> dataParams;
		Map<String, SHADER_OBJECT_PARAM_DESC> textureParams;
		Map<String, SHADER_OBJECT_PARAM_DESC> bufferParams;
		Map<String, SHADER_OBJECT_PARAM_DESC> samplerParams;
		Map<String, SHADER_PARAM_BLOCK_DESC> paramBlocks;

		Vector<UINT8> dataDefaultValues;
		Vector<SamplerStateType> samplerDefaultValues;
		Vector<TextureType> textureDefaultValues;
		Vector<SHADER_PARAM_ATTRIBUTE> paramAttributes;

	private:
		/**
		 * @copydoc	addParameter(SHADER_OBJECT_PARAM_DESC)
		 *
		 * @note	Common method shared by different addParameter overloads.
		 */
		void addParameterInternal(SHADER_OBJECT_PARAM_DESC paramDesc, UINT32 defaultValueIdx);
	};

	/**	Templated version of Shader used for implementing both sim and core thread variants. */
	template<bool Core>
	class BS_CORE_EXPORT TShader
	{
	public:
		using TechniqueType = CoreVariantType<Technique, Core>;
		using TextureType = typename TSHADER_DESC<Core>::TextureType;
		using SamplerStateType = typename TSHADER_DESC<Core>::SamplerStateType;
		using SubShaderType = typename TSubShaderType<Core>::Type;

		TShader() = default;
		TShader(const String& name, const TSHADER_DESC<Core>& desc, UINT32 id);
		virtual ~TShader();
	
		/** Returns the total number of techniques in this shader. */
		UINT32 getNumTechniques() const { return (UINT32)mDesc.techniques.size(); }

		/** Returns the list of all supported techniques based on current render API and renderer. */
		Vector<SPtr<TechniqueType>> getCompatibleTechniques() const;

		/** 
		 * Returns the list of all supported techniques based on current render API and renderer, and limits the techniques
		 * to only those implementing the specified variation.
		 * 
		 * @param[in]		variation	Object containing variation parameters to compare to technique variation.
		 * @param[in]		exact		When true the technique variation needs to have the exact number of parameters with
		 *								identical contents to the provided variation. When false, only the provided subset 
		 *								of parameters is used for comparison, while any extra parameters present in
		 *								the technique are not compared.
		 */
		Vector<SPtr<TechniqueType>> getCompatibleTechniques(const ShaderVariation& variation, bool exact) const;

		/** Returns a list of all techniques in this shader. */
		const Vector<SPtr<TechniqueType>>& getTechniques() const { return mDesc.techniques; }

		/** Returns a list of all sub-shaders in this shader. */
		const Vector<SubShaderType>& getSubShaders() const { return mDesc.subShaders; }

		/**
		 * Returns currently active queue sort type.
		 *
		 * @see		SHADER_DESC::queueSortType
		 */
		QueueSortType getQueueSortType() const { return mDesc.queueSortType; }

		/**
		 * Returns currently active queue priority.
		 *
		 * @see		SHADER_DESC::queuePriority
		 */
		INT32 getQueuePriority() const { return mDesc.queuePriority; }

		/**
		 * Returns if separable passes are allowed.
		 *
		 * @see		SHADER_DESC::separablePasses
		 */
		bool getAllowSeparablePasses() const { return mDesc.separablePasses; }

		/**
		 * Returns flags that control how the renderer interprets the shader. Actual interpretation of the flags depends on 
		 * the active renderer.
		 */
		ShaderFlags getFlags() const { return mDesc.flags; }

		/** Returns type of the parameter with the specified name. Throws exception if the parameter doesn't exist. */
		GpuParamType getParamType(const String& name) const;

		/**
		 * Returns description for a data parameter with the specified name. Throws exception if the parameter doesn't exist.
		 */
		const SHADER_DATA_PARAM_DESC& getDataParamDesc(const String& name) const;

		/**
		 * Returns description for a texture parameter with the specified name. Throws exception if the parameter doesn't 
		 * exist.
		 */
		const SHADER_OBJECT_PARAM_DESC& getTextureParamDesc(const String& name) const;

		/**
		 * Returns description for a sampler parameter with the specified name. Throws exception if the parameter doesn't 
		 * exist.
		 */
		const SHADER_OBJECT_PARAM_DESC& getSamplerParamDesc(const String& name) const;

		/**
		 * Returns description for a buffer parameter with the specified name. Throws exception if the parameter doesn't 
		 * exist.
		 */
		const SHADER_OBJECT_PARAM_DESC& getBufferParamDesc(const String& name) const;

		/** Checks if the parameter with the specified name exists, and is a data parameter. */
		bool hasDataParam(const String& name) const;

		/**	Checks if the parameter with the specified name exists, and is a texture parameter. */
		bool hasTextureParam(const String& name) const;

		/** Checks if the parameter with the specified name exists, and is a sampler parameter. */
		bool hasSamplerParam(const String& name) const;

		/** Checks if the parameter with the specified name exists, and is a buffer parameter. */
		bool hasBufferParam(const String& name) const;

		/** Checks if the parameter block with the specified name exists. */
		bool hasParamBlock(const String& name) const;

		/**	Returns a map of all data parameters in the shader. */
		const Map<String, SHADER_DATA_PARAM_DESC>& getDataParams() const { return mDesc.dataParams; }

		/**	Returns a map of all texture parameters in the shader. */
		const Map<String, SHADER_OBJECT_PARAM_DESC>& getTextureParams() const { return mDesc.textureParams; }

		/**	Returns a map of all buffer parameters in the shader. */
		const Map<String, SHADER_OBJECT_PARAM_DESC>& getBufferParams() const { return mDesc.bufferParams; }

		/** Returns a map of all sampler parameters in the shader. */
		const Map<String, SHADER_OBJECT_PARAM_DESC>& getSamplerParams() const { return mDesc.samplerParams; }

		/** Returns a map of all parameter blocks. */
		const Map<String, SHADER_PARAM_BLOCK_DESC>& getParamBlocks() const { return mDesc.paramBlocks; }

		/** Returns a list of all parameter attributes, as referenced by individual parameters. */
		const Vector<SHADER_PARAM_ATTRIBUTE>& getParamAttributes() const { return mDesc.paramAttributes; }

		/**
		 * Returns a default texture for a parameter that has the specified default value index (retrieved from the 
		 * parameters descriptor).
		 */
		TextureType getDefaultTexture(UINT32 index) const;

		/**
		 * Returns a default sampler state for a parameter that has the specified default value index (retrieved from the 
		 * parameters descriptor).
		 */
		SamplerStateType getDefaultSampler(UINT32 index) const;

		/**
		 * Returns a pointer to the internal buffer containing the default value for a data parameter that has the
		 * specified default value index (retrieved from the parameters descriptor).
		 */
		UINT8* getDefaultValue(UINT32 index) const;

		/** Returns the unique shader ID. */
		UINT32 getId() const { return mId; }

	protected:
		String mName;
		TSHADER_DESC<Core> mDesc;
		UINT32 mId;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	typedef TSHADER_DESC<true> SHADER_DESC;

	/** Core thread version of bs::SubShader. */
	typedef TSubShader<true> SubShader;

	/** @} */
	}

	/** @addtogroup Material
	 *  @{
	 */

	typedef TSHADER_DESC<false> SHADER_DESC;

	/** 
	 * @native
	 * Shader represents a collection of techniques that control object rendering. They are used in Material%s, which can be
	 * considered as instances of a Shader. Multiple materials may share the same shader but provide different parameters to
	 * it.
	 *
	 * Shader will always choose the first supported technique based on the current render system, render manager and other
	 * properties. So make sure to add most important techniques first so you make sure they are used if they are supported.
	 * @endnative
	 * 
	 * @script
	 * Contains definitions of GPU programs used for rendering, as well as a set of global parameters to control those 
	 * programs.
	 * @endscript
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) Shader : public Resource, public TShader<false>
	{
	public:
		/** Retrieves an implementation of a shader usable only from the core thread. */
		SPtr<ct::Shader> getCore() const;

		/**
		 * Sets a list include file paths that are referenced by this shader.
		 *
		 * @note	
		 * This is not used directly by the shader as includes are expected to be processed during GPU program and state 
		 * creation, but it may be referenced by higher layers for various purposes.
		 */
		void setIncludeFiles(const Vector<String>& includes);

		/**	Checks is the provided object type a sampler. */
		static bool isSampler(GpuParamObjectType type);

		/**	Checks is the provided object type a texture. */
		static bool isTexture(GpuParamObjectType type);

		/**	Checks is the provided object type a load/store (unordered read/write) texture. */
		static bool isLoadStoreTexture(GpuParamObjectType type);

		/** Checks is the provided object type a buffer. */
		static bool isBuffer(GpuParamObjectType type);

		/**
		 * Returns the size in bytes for a specific data type.
		 *
		 * @note	Returns 0 for variable size types like structures.
		 */
		static UINT32 getDataParamSize(GpuParamDataType type);

		/**	Creates a new shader resource using the provided descriptor and techniques. */
		static HShader create(const String& name, const SHADER_DESC& desc);

		/**	Returns a shader object but doesn't initialize it. */
		static SPtr<Shader> createEmpty();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Creates a new shader object using the provided descriptor and techniques.
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Shader> _createPtr(const String& name, const SHADER_DESC& desc);

		/** @} */

	private:
		Shader(const String& name, const SHADER_DESC& desc, UINT32 id);

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** Converts a sim thread version of the shader descriptor to a core thread version. */
		ct::SHADER_DESC convertDesc(const SHADER_DESC& desc) const;

	private:
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		Shader() = default;

	public:
		friend class ShaderRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
	/** @addtogroup Material
	 *  @{
	 */

	/** Shader specific resource meta-data containing information about referenced include files. */
	class BS_CORE_EXPORT ShaderMetaData : public ResourceMetaData
	{
	public:
		Vector<String> includes;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ShaderMetaDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** Core thread version of Shader. */
	class BS_CORE_EXPORT Shader : public CoreObject, public TShader<true>
	{
	public:
		/** @copydoc bs::Shader::create */
		static SPtr<Shader> create(const String& name, const SHADER_DESC& desc);

	protected:
		friend class bs::Shader;

		Shader(const String& name, const SHADER_DESC& desc, UINT32 id);

		static std::atomic<UINT32> mNextShaderId;
	};

	/** @} */
	}
}