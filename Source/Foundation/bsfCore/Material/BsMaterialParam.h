//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuParam.h"
#include "RenderAPI/BsGpuParams.h"
#include "Animation/BsAnimationCurve.h"

namespace bs
{
	template <class T>
	class TAnimationCurve;
	class ColorGradientHDR;

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common functionality for all material data params. */
	template<int DATA_TYPE, bool Core>
	class BS_CORE_EXPORT TMaterialDataCommon
	{
	protected:
		using MaterialPtrType = SPtr<CoreVariantType<Material, Core>>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;

	public:
		TMaterialDataCommon() = default;
		TMaterialDataCommon(const String& name, const MaterialPtrType& material);

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		UINT32 mArraySize;
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
	 * For core-thread version of this class no shader-based caching is done, and instead this represents just a wrapper
	 * for multiple GPU parameters.
	 *
	 * @see		Material
	 */
	template<class T, bool Core>
	class BS_CORE_EXPORT TMaterialDataParam : public TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, Core>
	{
		using Base = TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, Core>;

	public:
		using TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, Core>::TMaterialDataCommon;

		/** @copydoc TGpuDataParam::set */
		void set(const T& value, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::get */
		T get(UINT32 arrayIdx = 0) const;
	};

	/** @copydoc TMaterialDataParam */
	template<class T, bool Core>
	class BS_CORE_EXPORT TMaterialCurveParam : public TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, Core>
	{
		using Base = TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, Core>;

	public:
		using TMaterialDataCommon<TGpuDataParamInfo<T>::TypeId, Core>::TMaterialDataCommon;

		/** @copydoc TGpuDataParam::set */
		void set(TAnimationCurve<T> value, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::get */
		const TAnimationCurve<T>& get(UINT32 arrayIdx = 0) const;
	};
	
	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialColorGradientParam : public TMaterialDataCommon<GPDT_COLOR, Core>
	{
		using Base = TMaterialDataCommon<GPDT_COLOR, Core>;

	public:
		using TMaterialDataCommon<GPDT_COLOR, Core>::TMaterialDataCommon;

		/** @copydoc TGpuDataParam::set */
		void set(const ColorGradientHDR& value, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::get */
		const ColorGradientHDR& get(UINT32 arrayIdx = 0) const;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamStruct : public TMaterialDataCommon<GPDT_STRUCT, Core>
	{
		using Base = TMaterialDataCommon<GPDT_STRUCT, Core>;

	public:
		using TMaterialDataCommon<GPDT_STRUCT, Core>::TMaterialDataCommon;

		/** @copydoc TGpuParamStruct::set */
		void set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::get */
		void get(void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::getElementSize */
		UINT32 getElementSize() const;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamTexture
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, Core>>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;

	public:
		TMaterialParamTexture(const String& name, const MaterialPtrType& material);
		TMaterialParamTexture() { }

		/** @copydoc GpuParamTexture::set */
		void set(const TextureType& texture, const TextureSurface& surface = TextureSurface::COMPLETE) const;

		/** @copydoc GpuParamTexture::get */
		TextureType get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamSpriteTexture
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, Core>>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;
		using SpriteTextureType = CoreVariantHandleType<SpriteTexture, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;

	public:
		TMaterialParamSpriteTexture(const String& name, const MaterialPtrType& material);
		TMaterialParamSpriteTexture() { }

		/** @copydoc GpuParamTexture::set */
		void set(const SpriteTextureType& texture)const;

		/** @copydoc GpuParamTexture::get */
		SpriteTextureType get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamLoadStoreTexture
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, Core>>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;

	public:
		TMaterialParamLoadStoreTexture(const String& name, const MaterialPtrType& material);
		TMaterialParamLoadStoreTexture() { }

		/** @copydoc GpuParamLoadStoreTexture::set */
		void set(const TextureType& texture, const TextureSurface& surface = TextureSurface()) const;

		/** @copydoc GpuParamLoadStoreTexture::get */
		TextureType get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		MaterialPtrType mMaterial;
	};
	
	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamBuffer
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, Core>>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;

	public:
		TMaterialParamBuffer(const String& name, const MaterialPtrType& material);
		TMaterialParamBuffer() { }

		/** @copydoc GpuParamBuffer::set */
		void set(const BufferType& buffer) const;

		/** @copydoc GpuParamBuffer::get */
		BufferType get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamSampState
	{
		using MaterialPtrType = SPtr<CoreVariantType<Material, Core>>;
		using MaterialParamsType = CoreVariantType<MaterialParams, Core>;
		using SamplerStateType = SPtr<CoreVariantType<SamplerState, Core>>;

	public:
		TMaterialParamSampState(const String& name, const MaterialPtrType& material);
		TMaterialParamSampState() { }

		/** @copydoc GpuParamSampState::set */
		void set(const SamplerStateType& sampState) const;

		/** @copydoc GpuParamSampState::get */
		SamplerStateType get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mMaterial == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		MaterialPtrType mMaterial;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	typedef TMaterialDataParam<float, false> MaterialParamFloat;
	typedef TMaterialDataParam<Vector2, false> MaterialParamVec2;
	typedef TMaterialDataParam<Vector3, false> MaterialParamVec3;
	typedef TMaterialDataParam<Vector4, false> MaterialParamVec4;
	typedef TMaterialDataParam<int, false> MaterialParamInt;
	typedef TMaterialDataParam<Vector2I, false> MaterialParamVec2I;
	typedef TMaterialDataParam<Vector3I, false> MaterialParamVec3I;
	typedef TMaterialDataParam<Vector4I, false> MaterialParamVec4I;
	typedef TMaterialDataParam<Matrix3, false> MaterialParamMat3;
	typedef TMaterialDataParam<Matrix4, false> MaterialParamMat4;
	typedef TMaterialDataParam<Color, false> MaterialParamColor;

	typedef TMaterialParamStruct<false> MaterialParamStruct;
	typedef TMaterialParamTexture<false> MaterialParamTexture;
	typedef TMaterialParamLoadStoreTexture<false> MaterialParamLoadStoreTexture;
	typedef TMaterialParamBuffer<false> MaterialParamBuffer;
	typedef TMaterialParamSampState<false> MaterialParamSampState;

	typedef TMaterialCurveParam<float, false> MaterialParamFloatCurve;
	typedef TMaterialColorGradientParam<false> MaterialParamColorGradient;
	typedef TMaterialParamSpriteTexture<false> MaterialParamSpriteTexture;

	namespace ct
	{
		typedef TMaterialDataParam<float, true> MaterialParamFloat;
		typedef TMaterialDataParam<Vector2, true> MaterialParamVec2;
		typedef TMaterialDataParam<Vector3, true> MaterialParamVec3;
		typedef TMaterialDataParam<Vector4, true> MaterialParamVec4;
		typedef TMaterialDataParam<int, true> MaterialParamInt;
		typedef TMaterialDataParam<Vector2I, true> MaterialParamVec2I;
		typedef TMaterialDataParam<Vector3I, true> MaterialParamVec3I;
		typedef TMaterialDataParam<Vector4I, true> MaterialParamVec4I;
		typedef TMaterialDataParam<Matrix3, true> MaterialParamMat3;
		typedef TMaterialDataParam<Matrix4, true> MaterialParamMat4;
		typedef TMaterialDataParam<Color, true> MaterialParamColor;

		typedef TMaterialParamStruct<true> MaterialParamStruct;
		typedef TMaterialParamTexture<true> MaterialParamTexture;
		typedef TMaterialParamLoadStoreTexture<true> MaterialParamLoadStoreTexture;
		typedef TMaterialParamBuffer<true> MaterialParamBuffer;
		typedef TMaterialParamSampState<true> MaterialParamSampState;

		typedef TMaterialCurveParam<float, true> MaterialParamFloatCurve;
		typedef TMaterialColorGradientParam<true> MaterialParamColorGradient;
		typedef TMaterialParamSpriteTexture<true> MaterialParamSpriteTexture;
	}

	/** @} */
}
