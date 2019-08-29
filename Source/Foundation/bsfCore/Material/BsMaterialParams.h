//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "Allocators/BsStaticAlloc.h"
#include "Math/BsVector2.h"
#include "RenderAPI/BsGpuParams.h"
#include "Allocators/BsPoolAlloc.h"

namespace bs
{
	template<class T>
	class TAnimationCurve;
	class ColorGradientHDR;

	/** @addtogroup Material-Internal
	 *  @{
	 */

	struct SHADER_DATA_PARAM_DESC;
	struct SHADER_OBJECT_PARAM_DESC;
	struct SHADER_PARAM_ATTRIBUTE;

	/** Types of textures that can be assigned to a material texture parameter. */
	enum class MateralParamTextureType
	{
		/** Normal texture (static image, entire UV range). */
		Normal,
		/** Texture that is writeable by the material using unordered writes. */
		LoadStore,
		/** Sprite texture (either a subset of a larger texture, or an animated texture). */
		Sprite
	};

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

		/** Information about a single data parameter in a material. */
		struct DataParamInfo
		{
			UINT32 offset;

			TAnimationCurve<float>* floatCurve;
			ColorGradientHDR* colorGradient;
			UINT32 spriteTextureIdx;
		};

		/**
		 * Creates a new material params object and initializes enough room for parameters from the provided parameter data.
		 */
		MaterialParamsBase(
			const Map<String, SHADER_DATA_PARAM_DESC>& dataParams,
			const Map<String, SHADER_OBJECT_PARAM_DESC>& textureParams,
			const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferParams,
			const Map<String, SHADER_OBJECT_PARAM_DESC>& samplerParams,
			UINT64 initialParamVersion
		);

		/** Constructor for serialization use only. */
		MaterialParamsBase() = default;
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

			const DataParamInfo& paramInfo = mDataParams[param->index + arrayIdx];

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			memcpy(output, &mDataParamsBuffer[paramInfo.offset], sizeof(paramTypeSize));
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

			const DataParamInfo& paramInfo = mDataParams[param->index + arrayIdx];

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			memcpy(&mDataParamsBuffer[paramInfo.offset], input, sizeof(paramTypeSize));
		}

		/**
		 * Returns the animation curve assigned to a shader data parameter with the specified name at the specified array
		 * index. If the parameter name, index or type is not valid a warning will be logged and output value will not be
		 * retrieved. If no curve has been assigned to this parameter then an empty curve is returned.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @return					Animation curve assigned to the parameter.
		 *
		 * @tparam		T			Native type of the parameter.
		 */
		template <typename T>
		const TAnimationCurve<T>& getCurveParam(const String& name, UINT32 arrayIdx) const
		{
			static TAnimationCurve<T> EMPTY_CURVE;
			GpuParamDataType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = getParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if (result != GetParamResult::Success)
				return EMPTY_CURVE;

			return getCurveParam<T>(*param, arrayIdx);
		}

		/**
		 * Sets an animation curve to a shader data parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[in]	input		New value of the parameter.
		 *
		 * @tparam		T			Native type of the parameter.
		 */
		template <typename T>
		void setCurveParam(const String& name, UINT32 arrayIdx, TAnimationCurve<T> input) const
		{
			GpuParamDataType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = getParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if (result != GetParamResult::Success)
				return;

			setCurveParam(*param, arrayIdx, std::move(input));
		}

		/**
		 * Returns the color gradient assigned to a shader color parameter with the specified name at the specified array
		 * index. If the parameter name, index or type is not valid a warning will be logged and output value will not be
		 * retrieved. If no gradient has been assigned to this parameter then an empty gradient is returned.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @return					Color gradient assigned to the parameter.
		 */
		const ColorGradientHDR& getColorGradientParam(const String& name, UINT32 arrayIdx) const;

		/**
		 * Sets a color gradient to a shader color parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[in]	input		New value of the parameter.
		 */
		void setColorGradientParam(const String& name, UINT32 arrayIdx, const ColorGradientHDR& input) const;

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

			const DataParamInfo& paramInfo = mDataParams[param.index + arrayIdx];

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			assert(sizeof(output) == paramTypeSize);
			memcpy(&output, &mDataParamsBuffer[paramInfo.offset], paramTypeSize);
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

			DataParamInfo& paramInfo = mDataParams[param.index + arrayIdx];
			if (paramInfo.floatCurve)
			{
				bs_pool_free(paramInfo.floatCurve);
				paramInfo.floatCurve = nullptr;
			}

			if (paramInfo.colorGradient)
			{
				bs_pool_free(paramInfo.colorGradient);
				paramInfo.colorGradient = nullptr;
			}

			const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[dataType];
			UINT32 paramTypeSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;

			assert(sizeof(input) == paramTypeSize);
			memcpy(&mDataParamsBuffer[paramInfo.offset], &input, paramTypeSize);

			param.version = ++mParamVersion;
		}

		/**
		 * Equivalent to getCurveParam(const String&, UINT32) except it uses the internal parameter reference directly,
		 * avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		template <typename T>
		const TAnimationCurve<T>& getCurveParam(const ParamData& param, UINT32 arrayIdx) const
		{
			GpuParamDataType dataType = (GpuParamDataType)TGpuDataParamInfo<T>::TypeId;

			// Only supported for float types
			if(dataType == GPDT_FLOAT1)
			{
				const DataParamInfo& paramInfo = mDataParams[param.index + arrayIdx];
				if (paramInfo.floatCurve)
					return *paramInfo.floatCurve;
			}

			static TAnimationCurve<T> EMPTY_CURVE;
			return EMPTY_CURVE;
		}

		/**
		 * Equivalent to setCurveParam(const String&, UINT32, const TAnimationCurve<T>&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		template <typename T>
		void setCurveParam(const ParamData& param, UINT32 arrayIdx, TAnimationCurve<T> input) const
		{
			GpuParamDataType dataType = (GpuParamDataType)TGpuDataParamInfo<T>::TypeId;

			// Only supported for float types
			if(dataType == GPDT_FLOAT1)
			{
				DataParamInfo& paramInfo = mDataParams[param.index + arrayIdx];
				if(paramInfo.floatCurve)
					bs_pool_free(paramInfo.floatCurve);

				paramInfo.floatCurve = bs_pool_new<TAnimationCurve<T>>(std::move(input));

				param.version = ++mParamVersion;
			}
		}

		/**
		 * Equivalent to getColorGradientParam(const String&, UINT32) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		const ColorGradientHDR& getColorGradientParam(const ParamData& param, UINT32 arrayIdx) const;

		/**
		 * Equivalent to setColorGradientParam(const String&, UINT32, const ColorGradient&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void setColorGradientParam(const ParamData& param, UINT32 arrayIdx, const ColorGradientHDR& input) const;

		/** Returns pointer to the internal data buffer for a data parameter at the specified index. */
		UINT8* getData(UINT32 index) const
		{
			return &mDataParamsBuffer[mDataParams[index].offset];
		}

		/** Returns a counter that gets incremented whenever a parameter gets updated. */
		UINT64 getParamVersion() const { return mParamVersion; }

	protected:
		const static UINT32 STATIC_BUFFER_SIZE = 256;

		UnorderedMap<String, UINT32> mParamLookup;
		Vector<ParamData> mParams;

		DataParamInfo* mDataParams = nullptr;
		UINT8* mDataParamsBuffer = nullptr;

		UINT32 mDataSize = 0;
		UINT32 mNumDataParams = 0;
		UINT32 mNumStructParams = 0;
		UINT32 mNumTextureParams = 0;
		UINT32 mNumBufferParams = 0;
		UINT32 mNumSamplerParams = 0;

		mutable UINT64 mParamVersion = 1;
		mutable StaticAlloc<STATIC_BUFFER_SIZE> mAlloc;
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
		SPtr<ct::Texture> texture;
		SPtr<ct::SpriteTexture> spriteTexture;
		bool isLoadStore;
		TextureSurface surface;
	};

	/** Data for a single texture parameter. */
	class BS_CORE_EXPORT MaterialParamTextureData : public IReflectable
	{
	public:
		HTexture texture;
		HSpriteTexture spriteTexture;
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
		typedef MaterialParamStructData StructParamDataType;
		typedef MaterialParamTextureData TextureParamDataType;
		typedef MaterialParamBufferData BufferParamDataType;
		typedef MaterialParamSamplerStateData SamplerStateParamDataType;
	};

	template<> struct TMaterialParamsTypes < true >
	{
		typedef MaterialParamStructDataCore StructParamDataType;
		typedef MaterialParamTextureDataCore TextureParamDataType;
		typedef MaterialParamBufferDataCore BufferParamDataType;
		typedef MaterialParamSamplerStateDataCore SamplerStateParamDataType;
	};

	/** Common code that may be specialized for both MaterialParams and ct::MaterialParams. */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParams : public MaterialParamsBase
	{
	public:
		using GpuParamsType = CoreVariantType<GpuParams, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;
		using ShaderType = CoreVariantHandleType<Shader, Core>;
		using SpriteTextureType = CoreVariantHandleType<SpriteTexture, Core>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;
		using SamplerType = SPtr<CoreVariantType<SamplerState, Core>>;

		using ParamStructDataType = typename TMaterialParamsTypes<Core>::StructParamDataType;
		using ParamTextureDataType = typename TMaterialParamsTypes<Core>::TextureParamDataType;
		using ParamBufferDataType = typename TMaterialParamsTypes<Core>::BufferParamDataType;
		using ParamSamplerStateDataType = typename TMaterialParamsTypes<Core>::SamplerStateParamDataType;

		/**
		 * Creates a new material params object and initializes enough room for parameters from the provided shader.
		 *
		 * @param[in]	shader					Shader containing the information about parameters and their types.
		 * @param[in]	initialParamVersion		Initial version number to assign to the parameters. Usually relevant if
		 *										you are replacing an existing MaterialParams object and wish to ensure
		 *										version number keeps getting incremented.
		 */
		TMaterialParams(const ShaderType& shader, UINT64 initialParamVersion);

		/** Constructor for serialization use only. */
		TMaterialParams() = default;

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
		 * Returns the value of a shader texture parameter with the specified name as a sprite texture. If the parameter
		 * name or type is not valid a warning will be logged and output value will not be retrieved. If the assigned
		 * texture is not a sprite texture then this returns null and you should use one of the getTexture() overloads
		 * instead.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void getSpriteTexture(const String& name, SpriteTextureType& value) const;

		/**
		 * Assigns a sprite texture to a shader texture parameter with the specified name. If the parameter name or type
		 * is not valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void setSpriteTexture(const String& name, const SpriteTextureType& value);
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
		 * Checks does the data parameter with the specified name currently contains animated data. This could be
		 * an animation curve or a color gradient.
		 */
		bool isAnimated(const String& name, UINT32 arrayIdx = 0);

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
		 * Equivalent to getTexture(const String&, HTexture&, TextureSurface&) except it uses the internal parameter
		 * reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs
		 * to this object.
		 */
		void getTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const;

		/**
		 * Equivalent to setTexture(const String&, const HTexture&, const TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void setTexture(const ParamData& param, const TextureType& value,
						const TextureSurface& surface = TextureSurface::COMPLETE);

		/**
		 * Equivalent to getSpriteTexture(const String&, HSpriteTexture&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void getSpriteTexture(const ParamData& param, SpriteTextureType& value) const;

		/**
		 * Equivalent to setSpriteTexture(const String&, const HSpriteTexture&) except it uses the internal parameter
		 * reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs
		 * to this object.
		 */
		void setSpriteTexture(const ParamData& param, const SpriteTextureType& value);

		/**
		 * Equivalent to getBuffer(const String&, SPtr<GpuBuffer>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void getBuffer(const ParamData& param, BufferType& value) const;

		/**
		 * Equivalent to setBuffer(const String&, const SPtr<GpuBuffer>&) except it uses the internal parameter reference
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
		 * Equivalent to setLoadStoreTexture(const String&, const HTexture&, TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void setLoadStoreTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface);

		/**
		 * Returns the type of texture that is currently assigned to the provided parameter. This can only be called on
		 * on texture parameters. Caller must guarantee the parameter reference is valid, is of a texture type and
		 * belongs to this object.
		 */
		MateralParamTextureType getTextureType(const ParamData& param) const;

		/**
		 * Checks does the provided parameter have a curve or gradient assigned. This can only be called on data parameters.
		 * Caller must guarantee the parameter reference is valid, is of a data type and belongs to this object.

		 */
		bool isAnimated(const ParamData& param, UINT32 arrayIdx) const;

		/**
		 * Returns a sprite texture that is used for populating the specified data parameter. This is only relevant
		 * for data parameters marked with the ShaderParamAttributeType::SpriteUV attribute.
		 */
		SpriteTextureType getOwningSpriteTexture(const ParamData& param) const;

		/**
		 * Equivalent to getSamplerState(const String&, SPtr<SamplerState>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void getSamplerState(const ParamData& param, SamplerType& value) const;

		/**
		 * Equivalent to setSamplerState(const String&, const SPtr<SamplerState>&) except it uses the internal parameter
		 * reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs
		 * to this object.
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
		/** @copydoc TMaterialParams::TMaterialParams(const ShaderType&, UINT64) */
		MaterialParams(const HShader& shader, UINT64 initialParamVersion = 1);

		/**
		 * Populates the provided buffer with parameters that can be used for syncing this object with its core-thread
		 * counterpart. Can be applied by calling ct::MaterialParams::setSyncData.
		 *
		 * @param[in]		buffer		Pre-allocated buffer to store the sync data in. Set to null to calculate the size
		 *								of the required buffer.
		 * @param[in, out]	size		Size of the provided allocated buffer. Or if the buffer is null, this parameter will
		 *								contain the required buffer size when the method executes.
		 * @param[in]		forceAll	If false, only the parameters that were changed since the last call will be synced.
		 *								Otherwise all parameters will be synced.
		 */
		void getSyncData(UINT8* buffer, UINT32& size, bool forceAll);

		/** Appends any resources stored by this object to the provided vector. */
		void getResourceDependencies(Vector<HResource>& resources);

		/** Appends any core objects stored by this object to the provided vector. */
		void getCoreObjectDependencies(Vector<CoreObject*>& coreObjects);

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
		
		/** @copydoc TMaterialParams::TMaterialParams(const ShaderType&, UINT64) */
		MaterialParams(const SPtr<Shader>& shader, UINT64 initialParamVersion = 1);

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
