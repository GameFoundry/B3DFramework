//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuParam.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	class MaterialParams;

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
	class BS_CORE_EXPORT TMaterialDataParam
	{ };

	/** @copydoc TMaterialDataParam */
	template<class T>
	class TMaterialDataParam<T, false>
	{
	public:
		TMaterialDataParam(const String& name, const SPtr<MaterialParams>& params, 
			const SPtr<Vector<TGpuDataParam<T, false>>>& gpuParams);
		TMaterialDataParam() { }

		/** @copydoc TGpuDataParam::set */
		void set(const T& value, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::get */
		T get(UINT32 arrayIdx = 0) const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mGPUParams == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		UINT32 mArraySize;
		SPtr<MaterialParams> mMaterialParams;
		SPtr<Vector<TGpuDataParam<T, false>>> mGPUParams;
	};

	/** @copydoc TMaterialDataParam */
	template<class T>
	class TMaterialDataParam<T, true>
	{
	public:
		TMaterialDataParam(const SPtr<Vector<TGpuDataParam<T, true>>>& params);
		TMaterialDataParam() { }

		/** @copydoc TGpuDataParam::set */
		void set(const T& value, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::get */
		T get(UINT32 arrayIdx = 0) const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mParams == nullptr;
		}

	protected:
		SPtr<Vector<TGpuDataParam<T, true>>> mParams;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamStruct
	{ };

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamStruct<false>
	{
	public:
		TMaterialParamStruct(const String& name, const SPtr<MaterialParams>& params,
			const SPtr<Vector<TGpuParamStruct<false>>>& gpuParams);
		TMaterialParamStruct() { }

		/** @copydoc TGpuParamStruct::set */
		void set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::get */
		void get(void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::getElementSize */
		UINT32 getElementSize() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mGPUParams == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		UINT32 mArraySize;
		SPtr<MaterialParams> mMaterialParams;
		SPtr<Vector<TGpuParamStruct<false>>> mGPUParams;
	};

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamStruct<true>
	{
	public:
		TMaterialParamStruct(const SPtr<Vector<TGpuParamStruct<true>>>& params);
		TMaterialParamStruct() { }

		/** @copydoc TGpuParamStruct::set */
		void set(const void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::get */
		void get(void* value, UINT32 sizeBytes, UINT32 arrayIdx = 0) const;

		/** @copydoc TGpuParamStruct::getElementSize */
		UINT32 getElementSize() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mParams == nullptr;
		}

	protected:
		SPtr<Vector<TGpuParamStruct<true>>> mParams;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamTexture
	{ };

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamTexture<false>
	{
	public:
		TMaterialParamTexture(const String& name, const SPtr<MaterialParams>& params, 
			const SPtr<Vector<TGpuParamTexture<false>>>& gpuParams);
		TMaterialParamTexture() { }

		/** @copydoc GpuParamTexture::set */
		void set(const HTexture& texture) const;

		/** @copydoc GpuParamTexture::get */
		HTexture get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mGPUParams == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		SPtr<MaterialParams> mMaterialParams;
		SPtr<Vector<TGpuParamTexture<false>>> mGPUParams;
	};

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamTexture<true>
	{
	public:
		TMaterialParamTexture(const SPtr<Vector<TGpuParamTexture<true>>>& params);
		TMaterialParamTexture() { }

		/** @copydoc GpuParamTexture::set */
		void set(const SPtr<TextureCore>& texture) const;

		/** @copydoc GpuParamTexture::get */
		SPtr<TextureCore> get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mParams == nullptr;
		}

	protected:
		SPtr<Vector<TGpuParamTexture<true>>> mParams;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamLoadStoreTexture
	{ };

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamLoadStoreTexture<false>
	{
	public:
		TMaterialParamLoadStoreTexture(const String& name, const SPtr<MaterialParams>& params,
			const SPtr<Vector<TGpuParamLoadStoreTexture<false>>>& gpuParams);
		TMaterialParamLoadStoreTexture() { }

		/** @copydoc GpuParamLoadStoreTexture::set */
		void set(const HTexture& texture, const TextureSurface& surface) const;

		/** @copydoc GpuParamLoadStoreTexture::get */
		HTexture get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mGPUParams == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		SPtr<MaterialParams> mMaterialParams;
		SPtr<Vector<TGpuParamLoadStoreTexture<false>>> mGPUParams;
	};


	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamLoadStoreTexture<true>
	{
	public:
		TMaterialParamLoadStoreTexture(const SPtr<Vector<TGpuParamLoadStoreTexture<true>>>& params);
		TMaterialParamLoadStoreTexture() { }

		/** @copydoc GpuParamLoadStoreTexture::set */
		void set(const SPtr<TextureCore>& texture, const TextureSurface& surface = TextureSurface()) const;

		/** @copydoc GpuParamLoadStoreTexture::get */
		SPtr<TextureCore> get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mParams == nullptr;
		}

	protected:
		SPtr<Vector<TGpuParamLoadStoreTexture<true>>> mParams;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamBuffer
	{ };

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamBuffer<false>
	{
	public:
		TMaterialParamBuffer(const String& name, const SPtr<MaterialParams>& params,
			const SPtr<Vector<TGpuParamBuffer<false>>>& gpuParams);
		TMaterialParamBuffer() { }

		/** @copydoc GpuParamBuffer::set */
		void set(const SPtr<GpuBuffer>& buffer) const;

		/** @copydoc GpuParamBuffer::get */
		SPtr<GpuBuffer> get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mGPUParams == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		SPtr<MaterialParams> mMaterialParams;
		SPtr<Vector<TGpuParamBuffer<false>>> mGPUParams;
	};

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamBuffer<true>
	{
	public:
		TMaterialParamBuffer(const SPtr<Vector<TGpuParamBuffer<true>>>& params);
		TMaterialParamBuffer() { }

		/** @copydoc GpuParamBuffer::set */
		void set(const SPtr<GpuBufferCore>& buffer) const;

		/** @copydoc GpuParamBuffer::get */
		SPtr<GpuBufferCore> get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mParams == nullptr;
		}

	protected:
		SPtr<Vector<TGpuParamBuffer<true>>> mParams;
	};

	/** @copydoc TMaterialDataParam */
	template<bool Core>
	class BS_CORE_EXPORT TMaterialParamSampState
	{ };

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamSampState<false>
	{
	public:
		TMaterialParamSampState(const String& name, const SPtr<MaterialParams>& params, 
			const SPtr<Vector<TGpuParamSampState<false>>>& gpuParams);
		TMaterialParamSampState() { }

		/** @copydoc GpuParamSampState::set */
		void set(const SPtr<SamplerState>& sampState) const;

		/** @copydoc GpuParamSampState::get */
		SPtr<SamplerState> get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mGPUParams == nullptr;
		}

	protected:
		UINT32 mParamIndex;
		SPtr<MaterialParams> mMaterialParams;
		SPtr<Vector<TGpuParamSampState<false>>> mGPUParams;
	};

	/** @copydoc TMaterialDataParam */
	template<>
	class BS_CORE_EXPORT TMaterialParamSampState<true>
	{
	public:
		TMaterialParamSampState(const SPtr<Vector<TGpuParamSampState<true>>>& params);
		TMaterialParamSampState() { }

		/** @copydoc GpuParamSampState::set */
		void set(const SPtr<SamplerStateCore>& sampState) const;

		/** @copydoc GpuParamSampState::get */
		SPtr<SamplerStateCore> get() const;

		/** Checks if param is initialized. */
		bool operator==(const nullptr_t& nullval) const
		{
			return mParams == nullptr;
		}

	protected:
		SPtr<Vector<TGpuParamSampState<true>>> mParams;
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

	typedef TMaterialDataParam<float, true> MaterialParamFloatCore;
	typedef TMaterialDataParam<Vector2, true> MaterialParamVec2Core;
	typedef TMaterialDataParam<Vector3, true> MaterialParamVec3Core;
	typedef TMaterialDataParam<Vector4, true> MaterialParamVec4Core;
	typedef TMaterialDataParam<int, true> MaterialParamIntCore;
	typedef TMaterialDataParam<Vector2I, true> MaterialParamVec2ICore;
	typedef TMaterialDataParam<Vector3I, true> MaterialParamVec3ICore;
	typedef TMaterialDataParam<Vector4I, true> MaterialParamVec4ICore;
	typedef TMaterialDataParam<Matrix3, true> MaterialParamMat3Core;
	typedef TMaterialDataParam<Matrix4, true> MaterialParamMat4Core;
	typedef TMaterialDataParam<Color, true> MaterialParamColorCore;

	typedef TMaterialParamStruct<false> MaterialParamStruct;
	typedef TMaterialParamStruct<true> MaterialParamStructCore;

	typedef TMaterialParamTexture<false> MaterialParamTexture;
	typedef TMaterialParamTexture<true> MaterialParamTextureCore;

	typedef TMaterialParamLoadStoreTexture<false> MaterialParamLoadStoreTexture;
	typedef TMaterialParamLoadStoreTexture<true> MaterialParamLoadStoreTextureCore;

	typedef TMaterialParamBuffer<false> MaterialParamBuffer;
	typedef TMaterialParamBuffer<true> MaterialParamBufferCore;

	typedef TMaterialParamSampState<false> MaterialParamSampState;
	typedef TMaterialParamSampState<true> MaterialParamSampStateCore;

	/** @} */
}