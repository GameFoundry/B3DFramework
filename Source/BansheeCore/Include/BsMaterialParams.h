//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsStaticAlloc.h"
#include "BsVector2.h"
#include "BsGpuParams.h"

namespace bs
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	struct SHADER_DATA_PARAM_DESC;
	struct SHADER_OBJECT_PARAM_DESC;

	/** Common functionality for MaterialParams and ct::MaterialParams. */
	class BS_CORE_EXPORT MaterialParamsBase
	{
	public:
		/** Type of material parameter. */
		enum class ParamType
		{
			Data, Texture, Sampler, Buffer
		};

		/** Result codes for getParam method. */
		enum class GetParamResult
		{
			Success,
			NotFound,
			InvalidType,
			IndexOutOfBounds
		};

		/** Meta-data about a parameter. */
		struct ParamData
		{
			ParamType type;
			GpuParamDataType dataType;
			UINT32 index;
			UINT32 arraySize;
			mutable UINT64 version;
		};

		/** 
		 * Creates a new material params object and initializes enough room for parameters from the provided parameter data.
		 */
		MaterialParamsBase(
			const Map<String, SHADER_DATA_PARAM_DESC>& dataParams, 
			const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams,
			const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams,
			const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams);

		/** Constructor for serialization use only. */
		MaterialParamsBase() { }

		virtual ~MaterialParamsBase();

		/**
		 * Returns the value of a shader data parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[out]	output		If successful, value of the parameter.
		 *
		 * @tparam		T			Native type of the parameter.
		 */
		template <typename T>
		void getDataParam(const String& name, UINT32 arrayIdx, T& output) const
		{
			GpuParamDataType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = getParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if (result != GetParamResult::Success)
				return;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;
			output = *(T*)(mDataParamsBuffer[param->index + arrayIdx * paramTypeSize]);

			memcpy(output, &mDataParamsBuffer[param->index + arrayIdx * paramTypeSize], sizeof(paramTypeSize));
		}

		/**
		 * Sets the value of a shader data parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[in]	input		New value of the parameter.
		 *
		 * @tparam		T			Native type of the parameter.
		 */
		template <typename T>
		void setDataParam(const String& name, UINT32 arrayIdx, const T& input) const
		{
			GpuParamDataType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = getParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if (result != GetParamResult::Success)
				return;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			memcpy(&mDataParamsBuffer[param->index + arrayIdx * paramTypeSize], input, sizeof(paramTypeSize));
		}

		/** 
		 * Returns an index of the parameter with the specified name. Index can be used in a call to getParamData(UINT32) to
		 * get the actual parameter data.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @return					Index of the parameter, or -1 if not found.
		 */
		UINT32 getParamIndex(const String& name) const;

		/** 
		 * Returns an index of the parameter with the specified name. Index can be used in a call to getParamData(UINT32) to
		 * get the actual parameter data.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	type		Type of the parameter retrieve. Error will be logged if actual type of the parameter
		 *							doesn't match.
		 * @param[in]	dataType	Only relevant if the parameter is a data type. Determines exact data type of the parameter
		 *							to retrieve.
		 * @param[in]	arrayIdx	Array index of the entry to retrieve.
		 * @param[out]	output		Index of the requested parameter, only valid if success is returned.
		 * @return					Success or error state of the request.
		 */
		GetParamResult getParamIndex(const String& name, ParamType type, GpuParamDataType dataType, UINT32 arrayIdx,
			UINT32& output) const;

		/**
		 * Returns data about a parameter and reports an error if there is a type or size mismatch, or if the parameter
		 * does exist.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	type		Type of the parameter retrieve. Error will be logged if actual type of the parameter
		 *							doesn't match.
		 * @param[in]	dataType	Only relevant if the parameter is a data type. Determines exact data type of the parameter
		 *							to retrieve.
		 * @param[in]	arrayIdx	Array index of the entry to retrieve.
		 * @param[out]	output		Object describing the parameter with an index to its data. If the parameter was not found
		 *							this value is undefined. This value will still be valid if parameter was found but
		 *							some other error was reported.
		 * @return					Success or error state of the request.
		 */
		GetParamResult getParamData(const String& name, ParamType type, GpuParamDataType dataType, UINT32 arrayIdx,
			const ParamData** output) const;

		/**
		 * Returns information about a parameter at the specified global index, as retrieved by getParamIndex(). 
		 */
		const ParamData* getParamData(UINT32 index) const { return &mParams[index]; }

		/** Returns the total number of parameters managed by this object. */
		UINT32 getNumParams() const { return (UINT32)mParams.size(); }

		/**
		 * Logs an error that was reported by getParamData().
		 *
		 * @param[in]	errorCode	Information about the error.
		 * @param[in]	name		Name of the shader parameter for which the error occurred.
		 * @param[in]	arrayIdx	Array index for which the error occurred.
		 */
		void reportGetParamError(GetParamResult errorCode, const String& name, UINT32 arrayIdx) const;

		/**
		 * Equivalent to getDataParam(const String&, UINT32, T&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		template <typename T>
		void getDataParam(const ParamData& param, UINT32 arrayIdx, T& output) const
		{
			GpuParamDataType dataType = (GpuParamDataType)TGpuDataParamInfo<T>::TypeId;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			assert(sizeof(output) == paramTypeSize);
			memcpy(&output, &mDataParamsBuffer[param.index + arrayIdx * paramTypeSize], paramTypeSize);
		}

		/**
		 * Equivalent to setDataParam(const String&, UINT32, T&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		template <typename T>
		void setDataParam(const ParamData& param, UINT32 arrayIdx, const T& input) const
		{
			GpuParamDataType dataType = (GpuParamDataType)TGpuDataParamInfo<T>::TypeId;

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			assert(sizeof(input) == paramTypeSize);
			memcpy(&mDataParamsBuffer[param.index + arrayIdx * paramTypeSize], &input, paramTypeSize);

			param.version = ++mParamVersion;
		}

		/** Returns pointer to the internal data buffer for a data parameter at the specified index. */
		UINT8* getData(UINT32 index) const
		{
			return &mDataParamsBuffer[index];
		}

		/** Returns a counter that gets incremented whenever a parameter gets updated. */
		UINT64 getParamVersion() const { return mParamVersion; }

	protected:
		const static UINT32 STATIC_BUFFER_SIZE = 256;

		UnorderedMap<String, UINT32> mParamLookup;
		Vector<ParamData> mParams;

		UINT8* mDataParamsBuffer = nullptr;

		UINT32 mDataSize = 0;
		UINT32 mNumStructParams = 0;
		UINT32 mNumTextureParams = 0;
		UINT32 mNumBufferParams = 0;
		UINT32 mNumSamplerParams = 0;

		mutable UINT64 mParamVersion = 1;
		mutable StaticAlloc<STATIC_BUFFER_SIZE, STATIC_BUFFER_SIZE> mAlloc;
	};

	/** Raw data for a single structure parameter. */
	class BS_CORE_EXPORT MaterialParamStructDataCore
	{
	public:
		UINT8* data;
		UINT32 dataSize;
	};

	/** Raw data for a single structure parameter. */
	class BS_CORE_EXPORT MaterialParamStructData : public IReflectable
	{
	public:
		UINT8* data;
		UINT32 dataSize;

		friend class MaterialParamStructDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Data for a single texture parameter. */
	class BS_CORE_EXPORT MaterialParamTextureDataCore
	{
	public:
		SPtr<ct::Texture> value;
		bool isLoadStore;
		TextureSurface surface;
	};

	/** Data for a single texture parameter. */
	class BS_CORE_EXPORT MaterialParamTextureData : public IReflectable
	{
	public:
		HTexture value;
		bool isLoadStore;
		TextureSurface surface;

		friend class MaterialParamTextureDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Data for a single buffer parameter. */
	class BS_CORE_EXPORT MaterialParamBufferDataCore
	{
	public:
		SPtr<ct::GpuBuffer> value;
	};

	/** Data for a single buffer parameter. */
	class BS_CORE_EXPORT MaterialParamBufferData
	{
	public:
		SPtr<GpuBuffer> value;
	};

	/** Data for a single sampler state parameter. */
	class BS_CORE_EXPORT MaterialParamSamplerStateDataCore
	{
	public:
		SPtr<ct::SamplerState> value;
	};

	/** Data for a single sampler state parameter. */
	class BS_CORE_EXPORT MaterialParamSamplerStateData
	{
	public:
		SPtr<SamplerState> value;
	};

	/** Helper typedefs that reference types used by either core or sim thread implementation of TMaterialParams<Core>. */
	template<bool Core> struct TMaterialParamsTypes { };
	template<> struct TMaterialParamsTypes < false >
	{
		typedef GpuParams GpuParamsType;
		typedef HTexture TextureType;
		typedef SPtr<GpuBuffer> BufferType;
		typedef SPtr<SamplerState> SamplerType;
		typedef SPtr<GpuParamBlockBuffer> ParamsBufferType;
		typedef MaterialParamStructData StructParamDataType;
		typedef MaterialParamTextureData TextureParamDataType;
		typedef MaterialParamBufferData BufferParamDataType;
		typedef MaterialParamSamplerStateData SamplerStateParamDataType;
		typedef HShader ShaderType;
	};

	template<> struct TMaterialParamsTypes < true >
	{
		typedef ct::GpuParams GpuParamsType;
		typedef SPtr<ct::Texture> TextureType;
		typedef SPtr<ct::GpuBuffer> BufferType;
		typedef SPtr<ct::SamplerState> SamplerType;
		typedef SPtr<ct::GpuParamBlockBuffer> ParamsBufferType;
		typedef MaterialParamStructDataCore StructParamDataType;
		typedef MaterialParamTextureDataCore TextureParamDataType;
		typedef MaterialParamBufferDataCore BufferParamDataType;
		typedef MaterialParamSamplerStateDataCore SamplerStateParamDataType;
		typedef SPtr<ct::Shader> ShaderType;
	};

	/** Common code that may be specialized for both MaterialParams and ct::MaterialParams. */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParams : public MaterialParamsBase
	{
	public:
		typedef typename TMaterialParamsTypes<Core>::GpuParamsType GpuParamsType;
		typedef typename TMaterialParamsTypes<Core>::TextureType TextureType;
		typedef typename TMaterialParamsTypes<Core>::BufferType BufferType;
		typedef typename TMaterialParamsTypes<Core>::SamplerType SamplerType;
		typedef typename TMaterialParamsTypes<Core>::ShaderType ShaderType;
		typedef typename TMaterialParamsTypes<Core>::StructParamDataType ParamStructDataType;
		typedef typename TMaterialParamsTypes<Core>::TextureParamDataType ParamTextureDataType;
		typedef typename TMaterialParamsTypes<Core>::BufferParamDataType ParamBufferDataType;
		typedef typename TMaterialParamsTypes<Core>::SamplerStateParamDataType ParamSamplerStateDataType;

		/** Creates a new material params object and initializes enough room for parameters from the provided shader. */
		TMaterialParams(const ShaderType& shader);

		/** Constructor for serialization use only. */
		TMaterialParams() { }

		virtual ~TMaterialParams();

		/**
		 * Returns the value of a shader structure parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Pre-allocated buffer of @p size bytes where the value will be retrieved.
		 * @param[in]	size		Size of the buffer into which to write the value. Must match parameter struct's size.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 */
		void getStructData(const String& name, void* value, UINT32 size, UINT32 arrayIdx) const;

		/**
		 * Sets the value of a shader structure parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		Buffer of @p size bytes containing the new value of the structure.
		 * @param[in]	size		Size of the buffer from which to retrieve the value. Must match parameter struct's size.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 */
		void setStructData(const String& name, const void* value, UINT32 size, UINT32 arrayIdx);

		/**
		 * Returns the value of a shader texture parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 * @param[out]	surface		Surface describing which part of the texture is being accessed.
		 */
		void getTexture(const String& name, TextureType& value, TextureSurface& surface) const;

		/**
		 * Sets the value of a shader texture parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 * @param[in]	surface		Surface describing which part of the texture is being accessed.
		 */
		void setTexture(const String& name, const TextureType& value, 
						const TextureSurface& surface = TextureSurface::COMPLETE);

		/**
		 * Returns the value of a shader load/store texture parameter with the specified name. If the parameter name or
		 * type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 * @param[out]	surface		Surface describing which part of the texture is being accessed.
		 */
		void getLoadStoreTexture(const String& name, TextureType& value, TextureSurface& surface) const;

		/**
		 * Sets the value of a shader load/store texture parameter with the specified name. If the parameter name or
		 * type is not valid a warning will be logged and the value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 * @param[in]	surface		Surface describing which part of the texture is being accessed.
		 */
		void setLoadStoreTexture(const String& name, const TextureType& value, const TextureSurface& surface);

		/**
		 * Returns the value of a shader buffer parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void getBuffer(const String& name, BufferType& value) const;

		/**
		 * Sets the value of a shader buffer parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void setBuffer(const String& name, const BufferType& value);

		/**
		 * Sets the value of a shader sampler state parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void getSamplerState(const String& name, SamplerType& value) const;

		/**
		 * Sets the value of a shader sampler state parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void setSamplerState(const String& name, const SamplerType& value);

		/**
		 * Equivalent to getStructData(const String&, UINT32, void*, UINT32) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void getStructData(const ParamData& param, void* value, UINT32 size, UINT32 arrayIdx) const;

		/**
		 * Equivalent to setStructData(const String&, UINT32, void*, UINT32) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void setStructData(const ParamData& param, const void* value, UINT32 size, UINT32 arrayIdx);

		/**
		 * Returns a size of a struct parameter in bytes, using the internal parameter reference. Caller must guarantee the
		 * parameter reference is valid and belongs to this object.
		 */
		UINT32 getStructSize(const ParamData& param) const;

		/**
		 * Equivalent to getTexture(const String&, HTexture&) except it uses the internal parameter reference directly,
		 * avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this object.
		 */
		void getTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const;

		/**
		 * Equivalent to setTexture(const String&, HTexture&) except it uses the internal parameter reference directly,
		 * avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this object.
		 */
		void setTexture(const ParamData& param, const TextureType& value, 
						const TextureSurface& surface = TextureSurface::COMPLETE);

		/**
		 * Equivalent to getBuffer(const String&, SPtr<GpuBuffer>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void getBuffer(const ParamData& param, BufferType& value) const;

		/**
		 * Equivalent to setBuffer(const String&, SPtr<GpuBuffer>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void setBuffer(const ParamData& param, const BufferType& value);

		/**
		 * Equivalent to getLoadStoreTexture(const String&, HTexture&, TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void getLoadStoreTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const;

		/**
		 * Equivalent to setLoadStoreTexture(const String&, HTexture&, TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void setLoadStoreTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface);

		/**
		 * Checks is a texture with the specified parameter reference a load/store texture or a normal one. Caller must
		 * guarantee the parameter reference is valid and belongs to this object.
		 */
		bool getIsTextureLoadStore(const ParamData& param) const;

		/**
		 * Equivalent to getSamplerState(const String&, SPtr<SamplerState>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void getSamplerState(const ParamData& param, SamplerType& value) const;

		/**
		 * Equivalent to setSamplerState(const String&, SPtr<SamplerState>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void setSamplerState(const ParamData& param, const SamplerType& value);

		/**
		 * Returns the default texture (one assigned when no other is provided), if available for the specified parameter.
		 * Parameter is represented using the internal parameter reference and the caller must guarantee the parameter 
		 * eference is valid and belongs to this object.
		 */
		void getDefaultTexture(const ParamData& param, TextureType& value) const;

		/**
		 * Returns the default sampler state (one assigned when no other is provided), if available for the specified 
		 * parameter. Parameter is represented using the internal parameter reference and the caller must guarantee the
		 * parameter reference is valid and belongs to this object.
		 */
		void getDefaultSamplerState(const ParamData& param, SamplerType& value) const;

	protected:
		ParamStructDataType* mStructParams = nullptr;
		ParamTextureDataType* mTextureParams = nullptr;
		ParamBufferDataType* mBufferParams = nullptr;
		ParamSamplerStateDataType* mSamplerStateParams = nullptr;
		TextureType* mDefaultTextureParams = nullptr;
		SamplerType* mDefaultSamplerStateParams = nullptr;
	};

	/** 
	 * Contains all parameter values set in a Material. This is similar to GpuParams which also stores parameter values,
	 * however GpuParams are built for use on the GPU-side and don't store parameters that don't exist in a compiled GPU
	 * program. This object on the other hand stores all parameters defined in a shader, regardless or not if they actually
	 * exist in the GPU program. Additionally GpuParams are defined per-program (for example vertex, fragment) while this
	 * object exists for the entire material.
	 *
	 * @note
	 * This introduces redundancy as parameters stored by GpuParams and this object are duplicated. If this is an issue the
	 * implementation can be modified to only store parameters not included in GpuParams.
	 * @note
	 * The reason why parameters in this class and GpuParams differ is most often compiler optimizations. If a compiler
	 * optimizes out a variable in a GPU program we should still be able to store it, either for later when the variable
	 * will be introduced, or for other techniques that might have that variable implemented.
	 */
	class BS_CORE_EXPORT MaterialParams : public IReflectable, public TMaterialParams<false>
	{
	public:
		/** @copydoc TMaterialParams::TMaterialParams(const ShaderType&) */
		MaterialParams(const HShader& shader);

		/** 
		 * Populates the provided buffer with parameters that can be used for syncing this object with its core-thread
		 * counterpart. Can be applied by calling ct::MaterialParams::setSyncData.
		 *
		 * @param[in]		buffer		Pre-allocated buffer to store the sync data in. Set to null to calculate the size
		 *								of the required buffer.
		 * @param[in, out]	size		Size of the provided allocated buffer. Or if the buffer is null, this parameter will
		 *								contain the required buffer size when the method executes.
		 */
		void getSyncData(UINT8* buffer, UINT32& size);

	private:
		friend class ct::MaterialParams;

		UINT64 mLastSyncVersion;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		MaterialParams() { } // Only for serialization

		friend class MaterialParamsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
	/** Core thread version of MaterialParams. */
	class BS_CORE_EXPORT MaterialParams : public TMaterialParams<true>
	{
	public:
		/** Initializes the core thread version of MaterialParams from its sim thread counterpart. */
		MaterialParams(const SPtr<Shader>& shader, const SPtr<bs::MaterialParams>& params);
		
		/** @copydoc TMaterialParams::TMaterialParams(const ShaderType&) */
		MaterialParams(const SPtr<Shader>& shader);

		/** 
		 * Updates the stored parameters from the provided buffer, allowing changes to be transfered between the sim and 
		 * core thread material param objects. Buffer must be retrieved from bs::MaterialParams::getSyncData. 
		 *
		 * @param[in]		buffer		Buffer containing the dirty data.
		 * @param[in, out]	size		Size of the provided buffer.
		 */
		void setSyncData(UINT8* buffer, UINT32 size);
	};
	}
	/** @} */
}
