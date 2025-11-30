//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DGpuParameter.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Animation/B3DAnimationCurve.h"

namespace b3d
{
	template <class T>
	class TAnimationCurve;
	class ColorGradientHDR;

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common functionality for all material data params. */
	template <int DATA_TYPE, bool IsRenderProxy>
	class B3D_EXPORT TMaterialDataCommon
	{
	protected:
		using MaterialPtrType = SPtr<CoreVariantType<Material, IsRenderProxy>>;
		using MaterialParamsType = CoreVariantType<MaterialParameters, IsRenderProxy>;

	public:
		TMaterialDataCommon() = default;
		TMaterialDataCommon(const String& name, const MaterialPtrType& material);

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		u32 mParamIndex;
		u32 mArraySize;
		MaterialPtrType mMaterial;
	};

	/**
	 * A handle that allows you to set a Material parameter. Internally keeps a reference to the material parameters so that
	 * possibly expensive lookup of parameter name can be avoided each time the parameter is accessed, and instead the
	 * handle can be cached.
	 *
	 * @note
	 * This is pretty much identical to GPU parameter version (for example TGpuDataParam), except that this will get/set
	 * parameter values on all GPU programs attached to the material, while TGpuDataParam works only for single GPU
	 * program's parameters. Also, additional parameters that might be optimized out in the GPU program will still exist
	 * here as long as they're  defined in the shader used by the material, which is not the case with TGpuDataParam.
	 * @note
	 * For render thread version of this class no shader-based caching is done, and instead this represents just a wrapper
	 * for multiple GPU parameters.
	 *
	 * @see		Material
	 */
	template <class T, bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterPrimitive : public TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, IsRenderProxy>
	{
		using Base = TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, IsRenderProxy>;

	public:
		using TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, IsRenderProxy>::TMaterialDataCommon;

		/** @copydoc TGpuDataParam::Set */
		void Set(const T& value, u32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::Get */
		T Get(u32 arrayIdx = 0) const;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <class T, bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterCurve : public TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, IsRenderProxy>
	{
		using Base = TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, IsRenderProxy>;

	public:
		using TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, IsRenderProxy>::TMaterialDataCommon;

		/** @copydoc TGpuDataParam::Set */
		void Set(TAnimationCurve<T> value, u32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::Get */
		const TAnimationCurve<T>& Get(u32 arrayIdx = 0) const;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterColorGradient : public TMaterialDataCommon<GPDT_COLOR, IsRenderProxy>
	{
		using Base = TMaterialDataCommon<GPDT_COLOR, IsRenderProxy>;

	public:
		using TMaterialDataCommon<GPDT_COLOR, IsRenderProxy>::TMaterialDataCommon;

		/** @copydoc TGpuDataParam::Set */
		void Set(const ColorGradientHDR& value, u32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::Get */
		const ColorGradientHDR& Get(u32 arrayIdx = 0) const;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterStruct : public TMaterialDataCommon<GPDT_STRUCT, IsRenderProxy>
	{
		using Base = TMaterialDataCommon<GPDT_STRUCT, IsRenderProxy>;

	public:
		using TMaterialDataCommon<GPDT_STRUCT, IsRenderProxy>::TMaterialDataCommon;

		/** @copydoc TGpuParamStruct::Set */
		void Set(const void* value, u32 sizeBytes, u32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::Get */
		void Get(void* value, u32 sizeBytes, u32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::GetElementSize */
		u32 GetElementSize() const;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterSampledTexture
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, IsRenderProxy>>;
		using MaterialParamsType = CoreVariantType<MaterialParameters, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

	public:
		TMaterialParameterSampledTexture(const String& name, const MaterialPtrType& material);

		TMaterialParameterSampledTexture() {}

		/** @copydoc GpuParamTexture::Set */
		void Set(const TextureType& texture, const TextureSurface& surface = TextureSurface::kComplete) const;

		/** @copydoc GpuParamTexture::Get */
		TextureType Get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		u32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParamSpriteImage
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, IsRenderProxy>>;
		using MaterialParamsType = CoreVariantType<MaterialParameters, IsRenderProxy>;
		using SpriteImageType = CoreVariantHandleType<SpriteImage, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

	public:
		TMaterialParamSpriteImage(const String& name, const MaterialPtrType& material);

		TMaterialParamSpriteImage() {}

		/** @copydoc GpuParamTexture::Set */
		void Set(const SpriteImageType& image) const;

		/** @copydoc GpuParamTexture::Get */
		SpriteImageType Get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		u32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterStorageTexture
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, IsRenderProxy>>;
		using MaterialParamsType = CoreVariantType<MaterialParameters, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

	public:
		TMaterialParameterStorageTexture(const String& name, const MaterialPtrType& material);

		TMaterialParameterStorageTexture() {}

		/** @copydoc GpuParamLoadStoreTexture::Set */
		void Set(const TextureType& texture, const TextureSurface& surface = TextureSurface()) const;

		/** @copydoc GpuParamLoadStoreTexture::Get */
		TextureType Get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		u32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterBuffer
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, IsRenderProxy>>;
		using MaterialParamsType = CoreVariantType<MaterialParameters, IsRenderProxy>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;

	public:
		TMaterialParameterBuffer(const String& name, const MaterialPtrType& material);

		TMaterialParameterBuffer() {}

		/** @copydoc GpuParamBuffer::Set */
		void Set(const BufferType& buffer) const;

		/** @copydoc GpuParamBuffer::Get */
		BufferType Get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		u32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TMaterialParameterSampler
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, IsRenderProxy>>;
		using MaterialParamsType = CoreVariantType<MaterialParameters, IsRenderProxy>;

	public:
		TMaterialParameterSampler(const String& name, const MaterialPtrType& material);

		TMaterialParameterSampler() {}

		/** @copydoc GpuParamSampState::Set */
		void Set(const SPtr<SamplerState>& sampState) const;

		/** @copydoc GpuParamSampState::Get */
		SPtr<SamplerState> Get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		u32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	typedef TMaterialParameterPrimitive<float, false> MaterialParameterFloat;
	typedef TMaterialParameterPrimitive<double, false> MaterialParameterDouble;
	typedef TMaterialParameterPrimitive<Vector2, false> MaterialParameterVector2;
	typedef TMaterialParameterPrimitive<Vector3, false> MaterialParameterVector3;
	typedef TMaterialParameterPrimitive<Vector4, false> MaterialParameterVector4;
	typedef TMaterialParameterPrimitive<i32, false> MaterialParameterI32;
	typedef TMaterialParameterPrimitive<Vector2I, false> MaterialParameterVector2I;
	typedef TMaterialParameterPrimitive<Vector3I, false> MaterialParameterVector3I;
	typedef TMaterialParameterPrimitive<Vector4I, false> MaterialParameterVector4I;
	typedef TMaterialParameterPrimitive<u32, false> MaterialParameterU32;
	typedef TMaterialParameterPrimitive<Vector2UI, false> MaterialParameterVector2UI;
	typedef TMaterialParameterPrimitive<Vector3UI, false> MaterialParameterVector3UI;
	typedef TMaterialParameterPrimitive<Vector4UI, false> MaterialParameterVector4UI;
	typedef TMaterialParameterPrimitive<Matrix3, false> MaterialParameterMatrix3;
	typedef TMaterialParameterPrimitive<Matrix4, false> MaterialParameterMatrix4;
	typedef TMaterialParameterPrimitive<Color, false> MaterialParameterColor;

	typedef TMaterialParameterStruct<false> MaterialParameterStruct;
	typedef TMaterialParameterSampledTexture<false> MaterialParameterSampledTexture;
	typedef TMaterialParameterStorageTexture<false> MaterialParameterStorageTexture;
	typedef TMaterialParameterBuffer<false> MaterialParameterBuffer;
	typedef TMaterialParameterSampler<false> MaterialParameterSampler;

	typedef TMaterialParameterCurve<float, false> MaterialParameterFloatCurve;
	typedef TMaterialParameterColorGradient<false> MaterialParameterColorGradient;
	typedef TMaterialParamSpriteImage<false> MaterialParameterSpriteTexture;

	namespace render
	{
		typedef TMaterialParameterPrimitive<float, true> MaterialParameterFloat;
		typedef TMaterialParameterPrimitive<double, true> MaterialParameterDouble;
		typedef TMaterialParameterPrimitive<Vector2, true> MaterialParameterVector2;
		typedef TMaterialParameterPrimitive<Vector3, true> MaterialParameterVector3;
		typedef TMaterialParameterPrimitive<Vector4, true> MaterialParameterVector4;
		typedef TMaterialParameterPrimitive<i32, true> MaterialParameterI32;
		typedef TMaterialParameterPrimitive<Vector2I, true> MaterialParameterVector2I;
		typedef TMaterialParameterPrimitive<Vector3I, true> MaterialParameterVector3I;
		typedef TMaterialParameterPrimitive<Vector4I, true> MaterialParameterVector4I;
		typedef TMaterialParameterPrimitive<u32, true> MaterialParameterU32;
		typedef TMaterialParameterPrimitive<Vector2UI, true> MaterialParameterVector2UI;
		typedef TMaterialParameterPrimitive<Vector3UI, true> MaterialParameterVector3UI;
		typedef TMaterialParameterPrimitive<Vector4UI, true> MaterialParameterVector4UI;
		typedef TMaterialParameterPrimitive<Matrix3, true> MaterialParameterMatrix3;
		typedef TMaterialParameterPrimitive<Matrix4, true> MaterialParameterMatrix4;
		typedef TMaterialParameterPrimitive<Color, true> MaterialParameterColor;

		typedef TMaterialParameterStruct<true> MaterialParameterStruct;
		typedef TMaterialParameterSampledTexture<true> MaterialParameterSampledTexture;
		typedef TMaterialParameterStorageTexture<true> MaterialParameterStorageTexture;
		typedef TMaterialParameterBuffer<true> MaterialParameterBuffer;
		typedef TMaterialParameterSampler<true> MaterialParameterSampler;

		typedef TMaterialParameterCurve<float, true> MaterialParameterFloatCurve;
		typedef TMaterialParameterColorGradient<true> MaterialParameterColorGradient;
		typedef TMaterialParamSpriteImage<true> MaterialParameterSpriteTexture;
	} // namespace render

	/** @} */
} // namespace b3d
