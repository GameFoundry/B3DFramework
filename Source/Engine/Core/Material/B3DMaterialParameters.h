//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Allocators/B3DStaticAlloc.h"
#include "Math/B3DVector2.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	template <class T>
	class TAnimationCurve;
	class ColorGradientHDR;

	/** @addtogroup Material-Internal
	 *  @{
	 */

	struct ShaderDataParameterInformation;
	struct ShaderObjectParameterInformation;
	struct ShaderParameterAttribute;

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

	/** Common functionality for MaterialParameters and render::MaterialParameters. */
	class B3D_EXPORT MaterialParametersBase
	{
	public:
		/** Type of material parameter. */
		enum class ParamType
		{
			Data,
			Texture,
			Sampler,
			Buffer
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
			ParamType Type;
			GpuDataParameterType DataType;
			u32 Index;
			u32 ArraySize;
			mutable u64 Version;
		};

		/** Information about a single data parameter in a material. */
		struct DataParamInfo
		{
			u32 Offset;

			TAnimationCurve<float>* FloatCurve;
			ColorGradientHDR* ColorGradient;
			u32 SpriteTextureIdx;
		};

		/** Meta data for a single struct parameter. */
		struct StructParameterMetaData
		{
			u32 Offset;
			u32 DataSize;
		};

		/**
		 * Creates a new material params object and initializes enough room for parameters from the provided parameter data.
		 */
		MaterialParametersBase(
			const Map<String, ShaderDataParameterInformation>& dataParams,
			const Map<String, ShaderObjectParameterInformation>& textureParams,
			const Map<String, ShaderObjectParameterInformation>& bufferParams,
			const Map<String, ShaderObjectParameterInformation>& samplerParams,
			u64 initialParamVersion);

		/** Constructor for serialization use only. */
		MaterialParametersBase() = default;
		virtual ~MaterialParametersBase();

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
		void GetDataParam(const String& name, u32 arrayIdx, T& output) const
		{
			GpuDataParameterType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = GetParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if(result != GetParamResult::Success)
				return;

			const DataParamInfo& paramInfo = mDataParameterMetaData[param->Index + arrayIdx];

			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[dataType];
			u32 paramTypeSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			memcpy(output, &mDataParamsBuffer[paramInfo.Offset], sizeof(paramTypeSize));
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
		void SetDataParam(const String& name, u32 arrayIdx, const T& input) const
		{
			GpuDataParameterType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = GetParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if(result != GetParamResult::Success)
				return;

			const DataParamInfo& paramInfo = mDataParameterMetaData[param->Index + arrayIdx];

			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[dataType];
			u32 paramTypeSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			memcpy(&mDataParamsBuffer[paramInfo.Offset], input, sizeof(paramTypeSize));
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
		const TAnimationCurve<T>& GetCurveParam(const String& name, u32 arrayIdx) const
		{
			static TAnimationCurve<T> EMPTY_CURVE;
			GpuDataParameterType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = GetParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if(result != GetParamResult::Success)
				return EMPTY_CURVE;

			return GetCurveParam<T>(*param, arrayIdx);
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
		void SetCurveParam(const String& name, u32 arrayIdx, TAnimationCurve<T> input) const
		{
			GpuDataParameterType dataType = TGpuDataParamInfo<T>::TypeId;

			const ParamData* param = nullptr;
			auto result = GetParamData(name, ParamType::Data, dataType, arrayIdx, &param);
			if(result != GetParamResult::Success)
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
		const ColorGradientHDR& GetColorGradientParam(const String& name, u32 arrayIdx) const;

		/**
		 * Sets a color gradient to a shader color parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 * @param[in]	input		New value of the parameter.
		 */
		void SetColorGradientParam(const String& name, u32 arrayIdx, const ColorGradientHDR& input);

		/**
		 * Returns an index of the parameter with the specified name. Index can be used in a call to getParamData(u32) to
		 * get the actual parameter data.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @return					Index of the parameter, or -1 if not found.
		 */
		u32 GetParamIndex(const String& name) const;

		/**
		 * Returns an index of the parameter with the specified name. Index can be used in a call to getParamData(u32) to
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
		GetParamResult GetParamIndex(const String& name, ParamType type, GpuDataParameterType dataType, u32 arrayIdx, u32& output) const;

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
		GetParamResult GetParamData(const String& name, ParamType type, GpuDataParameterType dataType, u32 arrayIdx, const ParamData** output) const;

		/**
		 * Returns information about a parameter at the specified global index, as retrieved by getParamIndex().
		 */
		const ParamData* GetParamData(u32 index) const { return &mParams[index]; }

		/** Returns the total number of parameters managed by this object. */
		u32 GetParameterCount() const { return (u32)mParams.size(); }

		/**
		 * Logs an error that was reported by getParamData().
		 *
		 * @param[in]	errorCode	Information about the error.
		 * @param[in]	name		Name of the shader parameter for which the error occurred.
		 * @param[in]	arrayIdx	Array index for which the error occurred.
		 */
		void ReportGetParamError(GetParamResult errorCode, const String& name, u32 arrayIdx) const;

		/**
		 * Equivalent to getDataParam(const String&, u32, T&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		template <typename T>
		void GetDataParam(const ParamData& param, u32 arrayIdx, T& output) const
		{
			GpuDataParameterType dataType = (GpuDataParameterType)TGpuDataParamInfo<T>::TypeId;

			const DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];

			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[dataType];
			u32 paramTypeSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			B3D_ASSERT(sizeof(output) == paramTypeSize);
			memcpy(&output, &mDataParamsBuffer[paramInfo.Offset], paramTypeSize);
		}

		/**
		 * Equivalent to setDataParam(const String&, u32, T&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		template <typename T>
		void SetDataParam(const ParamData& param, u32 arrayIdx, const T& input)
		{
			GpuDataParameterType dataType = (GpuDataParameterType)TGpuDataParamInfo<T>::TypeId;

			DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];
			if(paramInfo.FloatCurve)
			{
				B3DPoolFree(paramInfo.FloatCurve);
				paramInfo.FloatCurve = nullptr;
			}

			if(paramInfo.ColorGradient)
			{
				B3DPoolFree(paramInfo.ColorGradient);
				paramInfo.ColorGradient = nullptr;
			}

			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[dataType];
			u32 paramTypeSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;

			B3D_ASSERT(sizeof(input) == paramTypeSize);
			memcpy(&mDataParamsBuffer[paramInfo.Offset], &input, paramTypeSize);

			param.Version = ++mParamVersion;
		}

		/**
		 * Equivalent to getCurveParam(const String&, u32) except it uses the internal parameter reference directly,
		 * avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		template <typename T>
		const TAnimationCurve<T>& GetCurveParam(const ParamData& param, u32 arrayIdx) const
		{
			GpuDataParameterType dataType = (GpuDataParameterType)TGpuDataParamInfo<T>::TypeId;

			// Only supported for float types
			if(dataType == GPDT_FLOAT1)
			{
				const DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];
				if(paramInfo.FloatCurve)
					return *paramInfo.FloatCurve;
			}

			static TAnimationCurve<T> EMPTY_CURVE;
			return EMPTY_CURVE;
		}

		/**
		 * Equivalent to setCurveParam(const String&, u32, const TAnimationCurve<T>&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		template <typename T>
		void SetCurveParam(const ParamData& param, u32 arrayIdx, TAnimationCurve<T> input)
		{
			GpuDataParameterType dataType = (GpuDataParameterType)TGpuDataParamInfo<T>::TypeId;

			// Only supported for float types
			if(dataType == GPDT_FLOAT1)
			{
				DataParamInfo& paramInfo = mDataParameterMetaData[param.Index + arrayIdx];
				if(paramInfo.FloatCurve)
					B3DPoolFree(paramInfo.FloatCurve);

				paramInfo.FloatCurve = B3DPoolNew<TAnimationCurve<T>>(std::move(input));

				param.Version = ++mParamVersion;
			}
		}

		/**
		 * Equivalent to getColorGradientParam(const String&, u32) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		const ColorGradientHDR& GetColorGradientParam(const ParamData& param, u32 arrayIdx) const;

		/**
		 * Equivalent to setColorGradientParam(const String&, u32, const ColorGradient&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void SetColorGradientParam(const ParamData& param, u32 arrayIdx, const ColorGradientHDR& input);

		/** Returns pointer to the internal data buffer for a data parameter at the specified index. */
		u8* GetData(u32 index) const
		{
			return &mDataParamsBuffer[mDataParameterMetaData[index].Offset];
		}

		/** Returns a counter that gets incremented whenever a parameter gets updated. */
		u64 GetParamVersion() const { return mParamVersion; }

	protected:
		const static u32 kStaticBufferSize = 256;

		UnorderedMap<String, u32> mParamLookup;
		Vector<ParamData> mParams;

		TInlineArray<DataParamInfo, 16> mDataParameterMetaData;
		TInlineArray<StructParameterMetaData, 2> mStructParameterMetaData;
		u8* mDataParamsBuffer = nullptr;

		u32 mDataSize = 0;
		u32 mTextureParameterCount = 0;
		u32 mBufferParameterCount = 0;
		u32 mSamplerParameterCount = 0;

		mutable u64 mParamVersion = 1;
		mutable StaticAlloc<kStaticBufferSize> mAlloc;
	};

	/** Data for a single texture parameter. */
	class B3D_EXPORT MaterialParamTextureDataRenderProxy
	{
	public:
		SPtr<render::Texture> Texture;
		SPtr<render::SpriteImage> SpriteImage;
		bool IsLoadStore;
		TextureSurface Surface;
	};

	/** Data for a single texture parameter. */
	class B3D_EXPORT MaterialParamTextureData : public IReflectable
	{
	public:
		HTexture Texture;
		HSpriteImage SpriteImage;
		bool IsLoadStore;
		TextureSurface Surface;

		friend class MaterialParamTextureDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** Data for a single buffer parameter. */
	class B3D_EXPORT MaterialParamBufferDataRenderProxy
	{
	public:
		SPtr<render::GpuBuffer> Value;
	};

	/** Data for a single buffer parameter. */
	class B3D_EXPORT MaterialParamBufferData : public IReflectable
	{
	public:
		SPtr<GpuBuffer> Value;

		friend class MaterialParamBufferDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** Data for a single sampler state parameter. */
	class B3D_EXPORT MaterialParamSamplerStateDataRenderProxy
	{
	public:
		SPtr<SamplerState> Value;
	};

	/** Data for a single sampler state parameter. */
	class B3D_EXPORT MaterialParamSamplerStateData : public IReflectable
	{
	public:
		SPtr<SamplerState> Value;

		friend class MaterialParamSamplerStateDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** Helper typedefs that reference types used by either render or main thread implementation of TMaterialParams<IsRenderProxy>. */
	template <bool IsRenderProxy>
	struct TMaterialParamsTypes
	{};

	template <>
	struct TMaterialParamsTypes<false>
	{
		typedef MaterialParamTextureData TextureParamDataType;
		typedef MaterialParamBufferData BufferParamDataType;
		typedef MaterialParamSamplerStateData SamplerStateParamDataType;
	};

	template <>
	struct TMaterialParamsTypes<true>
	{
		typedef MaterialParamTextureDataRenderProxy TextureParamDataType;
		typedef MaterialParamBufferDataRenderProxy BufferParamDataType;
		typedef MaterialParamSamplerStateDataRenderProxy SamplerStateParamDataType;
	};

	/** Common code that may be specialized for both MaterialParams and render::MaterialParams. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameters : public MaterialParametersBase
	{
	public:
		using GpuParametersType = CoreVariantType<GpuParameterSet, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using ShaderType = CoreVariantHandleType<Shader, IsRenderProxy>;
		using SpriteImageType = CoreVariantHandleType<SpriteImage, IsRenderProxy>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;

		using ParamTextureDataType = typename TMaterialParamsTypes<IsRenderProxy>::TextureParamDataType;
		using ParamBufferDataType = typename TMaterialParamsTypes<IsRenderProxy>::BufferParamDataType;
		using ParamSamplerStateDataType = typename TMaterialParamsTypes<IsRenderProxy>::SamplerStateParamDataType;

		/**
		 * Creates a new material params object and initializes enough room for parameters from the provided shader.
		 *
		 * @param[in]	shader					Shader containing the information about parameters and their types.
		 * @param[in]	initialParamVersion		Initial version number to assign to the parameters. Usually relevant if
		 *										you are replacing an existing MaterialParams object and wish to ensure
		 *										version number keeps getting incremented.
		 */
		TMaterialParameters(const ShaderType& shader, u64 initialParamVersion);

		/** Constructor for serialization use only. */
		TMaterialParameters() = default;

		virtual ~TMaterialParameters();

		/**
		 * Returns the value of a shader structure parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Pre-allocated buffer of @p size bytes where the value will be retrieved.
		 * @param[in]	size		Size of the buffer into which to write the value. Must match parameter struct's size.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 */
		void GetStructData(const String& name, void* value, u32 size, u32 arrayIdx) const;

		/**
		 * Sets the value of a shader structure parameter with the specified name at the specified array index. If the
		 * parameter name, index or type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		Buffer of @p size bytes containing the new value of the structure.
		 * @param[in]	size		Size of the buffer from which to retrieve the value. Must match parameter struct's size.
		 * @param[in]	arrayIdx	If the parameter is an array, index of the entry to access.
		 */
		void SetStructData(const String& name, const void* value, u32 size, u32 arrayIdx);

		/**
		 * Returns the value of a shader texture parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 * @param[out]	surface		Surface describing which part of the texture is being accessed.
		 */
		void GetTexture(const String& name, TextureType& value, TextureSurface& surface) const;

		/**
		 * Sets the value of a shader texture parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 * @param[in]	surface		Surface describing which part of the texture is being accessed.
		 */
		void SetTexture(const String& name, const TextureType& value, const TextureSurface& surface = TextureSurface::kComplete);

		/**
		 * Returns the value of a shader texture parameter with the specified name as a sprite image. If the parameter
		 * name or type is not valid a warning will be logged and output value will not be retrieved. If the assigned
		 * texture is not a sprite texture then this returns null and you should use one of the GetTexture() overloads
		 * instead.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void GetSpriteImage(const String& name, SpriteImageType& value) const;

		/**
		 * Assigns a sprite image to a shader texture parameter with the specified name. If the parameter name or type
		 * is not valid a warning will be logged and output value will not be set.
		 *
		 * @param	name		Name of the shader parameter.
		 * @param	value		New value of the parameter.
		 */
		void SetSpriteImage(const String& name, const SpriteImageType& value);
		/**
		 * Returns the value of a shader load/store texture parameter with the specified name. If the parameter name or
		 * type is not valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 * @param[out]	surface		Surface describing which part of the texture is being accessed.
		 */
		void GetStorageTexture(const String& name, TextureType& value, TextureSurface& surface) const;

		/**
		 * Sets the value of a shader load/store texture parameter with the specified name. If the parameter name or
		 * type is not valid a warning will be logged and the value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 * @param[in]	surface		Surface describing which part of the texture is being accessed.
		 */
		void SetStorageTexture(const String& name, const TextureType& value, const TextureSurface& surface);

		/**
		 * Returns the value of a shader buffer parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be retrieved.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void GetBuffer(const String& name, BufferType& value) const;

		/**
		 * Sets the value of a shader buffer parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void SetBuffer(const String& name, const BufferType& value);

		/**
		 * Sets the value of a shader sampler state parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[out]	value		Output value of the parameter.
		 */
		void GetSamplerState(const String& name, SPtr<SamplerState>& value) const;

		/**
		 * Sets the value of a shader sampler state parameter with the specified name. If the parameter name or type is not
		 * valid a warning will be logged and output value will not be set.
		 *
		 * @param[in]	name		Name of the shader parameter.
		 * @param[in]	value		New value of the parameter.
		 */
		void SetSamplerState(const String& name, const SPtr<SamplerState>& value);

		/**
		 * Checks does the data parameter with the specified name currently contains animated data. This could be
		 * an animation curve or a color gradient.
		 */
		bool IsAnimated(const String& name, u32 arrayIdx = 0);

		/**
		 * Equivalent to getStructData(const String&, u32, void*, u32) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void GetStructData(const ParamData& param, void* value, u32 size, u32 arrayIdx) const;

		/**
		 * Equivalent to setStructData(const String&, u32, void*, u32) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void SetStructData(const ParamData& param, const void* value, u32 size, u32 arrayIdx);

		/**
		 * Returns a size of a struct parameter in bytes, using the internal parameter reference. Caller must guarantee the
		 * parameter reference is valid and belongs to this object.
		 */
		u32 GetStructSize(const ParamData& param) const;

		/**
		 * Equivalent to getTexture(const String&, HTexture&, TextureSurface&) except it uses the internal parameter
		 * reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs
		 * to this object.
		 */
		void GetTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const;

		/**
		 * Equivalent to setTexture(const String&, const HTexture&, const TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void SetTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface = TextureSurface::kComplete);

		/**
		 * Equivalent to GetSpriteImage(const String&, HSpriteImage&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void GetSpriteImage(const ParamData& param, SpriteImageType& value) const;

		/**
		 * Equivalent to SetSpriteImage(const String&, const HSpriteImage&) except it uses the internal parameter
		 * reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs
		 * to this object.
		 */
		void SetSpriteImage(const ParamData& param, const SpriteImageType& value);

		/**
		 * Equivalent to getBuffer(const String&, SPtr<GpuBuffer>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void GetBuffer(const ParamData& param, BufferType& value) const;

		/**
		 * Equivalent to setBuffer(const String&, const SPtr<GpuBuffer>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void SetBuffer(const ParamData& param, const BufferType& value);

		/**
		 * Equivalent to getLoadStoreTexture(const String&, HTexture&, TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void GetStorageTexture(const ParamData& param, TextureType& value, TextureSurface& surface) const;

		/**
		 * Equivalent to setLoadStoreTexture(const String&, const HTexture&, TextureSurface&) except it uses the internal
		 * parameter reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid
		 * and belongs to this object.
		 */
		void SetStorageTexture(const ParamData& param, const TextureType& value, const TextureSurface& surface);

		/**
		 * Returns the type of texture that is currently assigned to the provided parameter. This can only be called on
		 * on texture parameters. Caller must guarantee the parameter reference is valid, is of a texture type and
		 * belongs to this object.
		 */
		MateralParamTextureType GetTextureType(const ParamData& param) const;

		/**
		 * Checks does the provided parameter have a curve or gradient assigned. This can only be called on data parameters.
		 * Caller must guarantee the parameter reference is valid, is of a data type and belongs to this object.

		 */
		bool IsAnimated(const ParamData& param, u32 arrayIdx) const;

		/**
		 * Returns a sprite texture that is used for populating the specified data parameter. This is only relevant
		 * for data parameters marked with the ShaderParamAttributeType::SpriteUV attribute.
		 */
		SpriteImageType GetOwningSpriteImage(const ParamData& param) const;

		/**
		 * Equivalent to getSamplerState(const String&, SPtr<SamplerState>&) except it uses the internal parameter reference
		 * directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs to this
		 * object.
		 */
		void GetSamplerState(const ParamData& param, SPtr<SamplerState>& value) const;

		/**
		 * Equivalent to setSamplerState(const String&, const SPtr<SamplerState>&) except it uses the internal parameter
		 * reference directly, avoiding the name lookup. Caller must guarantee the parameter reference is valid and belongs
		 * to this object.
		 */
		void SetSamplerState(const ParamData& param, const SPtr<SamplerState>& value);

		/**
		 * Returns the default texture (one assigned when no other is provided), if available for the specified parameter.
		 * Parameter is represented using the internal parameter reference and the caller must guarantee the parameter
		 * eference is valid and belongs to this object.
		 */
		void GetDefaultTexture(const ParamData& param, TextureType& value) const;

		/**
		 * Returns the default sampler state (one assigned when no other is provided), if available for the specified
		 * parameter. Parameter is represented using the internal parameter reference and the caller must guarantee the
		 * parameter reference is valid and belongs to this object.
		 */
		void GetDefaultSamplerState(const ParamData& param, SPtr<SamplerState>& value) const;

	protected:
		TInlineArray<ParamTextureDataType, 8> mTextureParameters;
		TInlineArray<ParamBufferDataType, 4> mBufferParameters;
		TInlineArray<ParamSamplerStateDataType, 2> mSamplerParameters;
		TextureType* mDefaultTextureParams = nullptr;
		SPtr<SamplerState>* mDefaultSamplerStateParams = nullptr;
	};

	/**
	 * Contains all parameter values set in a Material. This is similar to GpuParameterSet which also stores parameter values,
	 * however GpuParameterSet objects are built for use on the GPU-side and don't store parameters that don't exist in a
	 * compiled GPU program. This object on the other hand stores all parameters defined in a shader, regardless or not if
	 * they actually exist in the GPU program.
	 *
	 * @note
	 * This introduces redundancy as parameters stored by GpuParameterSet and this object are duplicated. If this is an issue
	 * the implementation can be modified to only store parameters not included in GpuParameterSet.
	 * @note
	 * The reason why parameters in this class and GpuParameterSet differ is most often compiler optimizations. If a compiler
	 * optimizes out a variable in a GPU program we should still be able to store it, either for later when the variable
	 * will be introduced, or for other variations that might have that variable implemented.
	 */
	class B3D_EXPORT MaterialParameters : public IReflectable, public TMaterialParameters<false>
	{
	public:
		struct SyncPacket;

		/** @copydoc TMaterialParameters::TMaterialParameters(const ShaderType&, u64) */
		MaterialParameters(const HShader& shader, u64 initialParamVersion = 1);

		/**
		 * Creates sync packet that can be used for bringing the render proxy up to date.
		 *
		 * @param allocator			Allocator with which to allocate the sync packet.
		 * @param forceAll			By default only dirty parameter will be synced. If you wish to sync all parameters
		 *							set this to true.
		 * @return					Sync packet if there are any dirty parameters, null otherwise.
		 */
		SyncPacket* CreateSyncPacket(FrameAllocator& allocator, bool forceAll);

		/** Appends any resources stored by this object to the provided vector. */
		void GetResourceDependencies(Vector<HResource>& resources);

		/** Appends any core objects stored by this object to the provided vector. */
		void GetCoreObjectDependencies(Vector<CoreObject*>& coreObjects);

	private:
		friend class render::MaterialParameters;

		mutable u64 mLastSyncVersion;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		MaterialParameters() {} // Only for serialization

		friend class MaterialParametersRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	namespace render
	{
		/** Render thread version of MaterialParams. */
		class B3D_EXPORT MaterialParameters : public TMaterialParameters<true>
		{
		public:
			/** Initializes the render proxy its main thread counterpart. */
			MaterialParameters(const SPtr<Shader>& shader, const SPtr<b3d::MaterialParameters>& params);

			/** @copydoc TMaterialParameters::TMaterialParameters(const ShaderType&, u64) */
			MaterialParameters(const SPtr<Shader>& shader, u64 initialParamVersion = 1);

			/**
			 * Updates the stored parameters from the provided sync packet, allowing changes to be transfered between core objects and
			 * render proxies. Packet must be retrieved from b3d::MaterialParams::CreateSyncPacket(). Sync packet is destroyed
			 * using the provided allocator after it has been applied.
			 */
			void ApplyAndDestroySyncPacket(FrameAllocator& allocator, const b3d::MaterialParameters::SyncPacket& syncPacket);
		};
	} // namespace render

	/** @} */
} // namespace b3d
