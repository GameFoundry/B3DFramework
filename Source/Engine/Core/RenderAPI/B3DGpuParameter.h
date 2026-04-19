//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuPipelineParameterLayout.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DMatrixNxM.h"
#include "Math/B3DVector3I.h"
#include "Math/B3DVector4I.h"
#include "Image/B3DColor.h"
#include "RenderAPI/B3DGpuBuffer.h"

namespace b3d
{
	/** @addtogroup Implementation-Internal
	 *  @{
	 */

	/**
	 * Policy class that allows us to re-use this template class for matrices which might need transposing, and other
	 * types which do not. Matrix needs to be transposed for certain render systems depending on how they store them
	 * in memory.
	 */
	template <class Type>
	struct TransposePolicy
	{
		static Type Transpose(const Type& value) { return value; }

		static bool TransposeEnabled(bool enabled) { return false; }
	};

	/** Transpose policy for 3x3 matrix. */
	template <>
	struct TransposePolicy<Matrix3>
	{
		static Matrix3 Transpose(const Matrix3& value) { return value.Transpose(); }

		static bool TransposeEnabled(bool enabled) { return enabled; }
	};

	/**	Transpose policy for 4x4 matrix. */
	template <>
	struct TransposePolicy<Matrix4>
	{
		static Matrix4 Transpose(const Matrix4& value) { return value.Transpose(); }

		static bool TransposeEnabled(bool enabled) { return enabled; }
	};

	/**	Transpose policy for NxM matrix. */
	template <int N, int M>
	struct TransposePolicy<MatrixNxM<N, M>>
	{
		static MatrixNxM<M, N> Transpose(const MatrixNxM<N, M>& value) { return value.Transpose(); }

		static bool TransposeEnabled(bool enabled) { return enabled; }
	};

	/** @} */

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Information that controls how is the buffer viewed when bound to a GPU pipeline. */
	struct GpuBufferViewInformation
	{
		GpuBufferViewInformation() = default;

		GpuBufferViewInformation(u32 offset, u32 range)
			: Offset(offset), Range(range)
		{ }

		GpuBufferViewInformation(GpuBufferFormat format)
			: Format(format)
		{ }

		u32 Offset = 0; /**< Offset from which to start reading the buffer. Not relevant for simple storage buffers. */
		u32 Range = 0; /**< Range of the buffer which to bind. In bytes. Not relevant for simple storage buffers. */
		GpuBufferFormat Format = BF_UNKNOWN; /**< Format to interpret the buffer contents as. If not specified, default buffer format will be used. Only relevant for simple storage buffers. */
	};

	/**
	 * A handle that allows you to set a GpuProgram parameter. Internally keeps a reference to the GPU parameter buffer and
	 * the necessary offsets. You should specialize this type for specific parameter types.
	 *
	 * Object of this type must be returned by a Material. Setting/Getting parameter values will internally access a GPU
	 * parameter buffer attached to the Material this parameter was created from. Anything rendered with that material will
	 * then use those set values.
	 *
	 * @note
	 * Normally you can set a GpuProgram parameter by calling various set/get methods on a Material. This class primarily
	 * used an as optimization in performance critical bits of code where it is important to locate and set parameters
	 * quickly without any lookups (Mentioned set/get methods expect a parameter name). You just retrieve the handle once
	 * and then set the parameter value many times with minimal performance impact.
	 *
	 * @see		Material
	 */
	template <class T, bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterPrimitive
	{
	private:
		using GpuParamBufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;
		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;

	public:
		TGpuParameterPrimitive();
		TGpuParameterPrimitive(const GpuUniformBufferMemberInformation* parameterInformation, const GpuParamsType& parent);

		/**
		 * Sets a parameter value at the specified array index. If parameter does not contain an array leave the index at 0.
		 *
		 * @note
		 * Like with all GPU parameters, the actual GPU buffer will not be updated until rendering with material this
		 * parameter was created from starts on the render thread.
		 */
		void Set(const T& value, u32 arrayIdx = 0) const;

		/**
		 * Returns a value of a parameter at the specified array index. If parameter does not contain an array leave the
		 * index at 0.
		 *
		 * @note	No GPU reads are done. Data returned was cached when it was written.
		 */
		T Get(u32 arrayIdx = 0) const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mParameterInformation == nullptr;
		}

	protected:
		GpuParamsType mParent;
		const GpuUniformBufferMemberInformation* mParameterInformation = nullptr;
	};

	/** @copydoc TGpuParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterStruct
	{
	public:
		using GpuParamBufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;
		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;

		TGpuParameterStruct();
		TGpuParameterStruct(const GpuUniformBufferMemberInformation* parameterInformation, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::Set */
		void Set(const void* value, u32 sizeBytes, u32 arrayIdx = 0) const;

		/** @copydoc TGpuDataParam::Get */
		void Get(void* value, u32 sizeBytes, u32 arrayIdx = 0) const;

		/**	Returns the size of the struct in bytes. */
		u32 GetElementSize() const;

		/**	Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return mParameterInformation == nullptr;
		}

	protected:
		GpuParamsType mParent;
		const GpuUniformBufferMemberInformation* mParameterInformation = nullptr;
	};

	/** @copydoc TGpuParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterSampledTexture
	{
	private:
		friend class GpuParameterSet;
		friend class render::GpuParameterSet;

		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

	public:
		TGpuParameterSampledTexture();
		TGpuParameterSampledTexture(const GpuParameterBinding& binding, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::Set */
		void Set(const TextureType& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0) const;

		/** @copydoc TGpuDataParam::Get */
		TextureType Get(u32 arrayIndex = 0) const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return !mBinding.IsValid();
		}

	protected:
		GpuParamsType mParent;
		GpuParameterBinding mBinding;
	};

	/** @copydoc TGpuParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterStorageTexture
	{
	private:
		friend class GpuParameterSet;
		friend class render::GpuParameterSet;

		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

	public:
		TGpuParameterStorageTexture();
		TGpuParameterStorageTexture(const GpuParameterBinding& binding, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::Set */
		void Set(const TextureType& texture, const TextureSurface& surface = TextureSurface(), u32 arrayIndex = 0) const;

		/** @copydoc TGpuDataParam::Get */
		TextureType Get(u32 arrayIndex = 0) const;

		/**	Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return !mBinding.IsValid();
		}

	protected:
		GpuParamsType mParent;
		GpuParameterBinding mBinding;
	};

	/** @copydoc TGpuParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterStorageBuffer
	{
	private:
		friend class GpuParameterSet;
		friend class render::GpuParameterSet;

		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;

	public:
		TGpuParameterStorageBuffer();
		TGpuParameterStorageBuffer(const GpuParameterBinding& binding, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::Set */
		void Set(const BufferType& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation()) const;

		/** @copydoc TGpuDataParam::Get */
		BufferType Get(u32 arrayIndex = 0) const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return !mBinding.IsValid();
		}

	protected:
		GpuParamsType mParent;
		GpuParameterBinding mBinding;
	};

	/** @copydoc TGpuParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterUniformBuffer
	{
	private:
		friend class GpuParameterSet;
		friend class render::GpuParameterSet;

		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, IsRenderProxy>>;

	public:
		TGpuParameterUniformBuffer();
		TGpuParameterUniformBuffer(const GpuParameterBinding& binding, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::Set */
		void Set(const BufferType& buffer) const;

		/** @copydoc TGpuDataParam::Set */
		template<bool Condition = IsRenderProxy, std::enable_if_t<Condition, i32> = 0>
		void Set(const render::GpuBufferSuballocation& bufferSuballocation) const;

		/** @copydoc TGpuDataParam::Get */
		BufferType Get() const;

		/** Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return !mBinding.IsValid();
		}

	protected:
		GpuParamsType mParent;
		GpuParameterBinding mBinding;
	};

	/** @copydoc TGpuParameterPrimitive */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterSampler
	{
	private:
		friend class GpuParameterSet;
		friend class render::GpuParameterSet;

		using GpuParamsType = SPtr<CoreVariantType<GpuParameterSet, IsRenderProxy>>;

	public:
		TGpuParameterSampler();
		TGpuParameterSampler(const GpuParameterBinding& binding, const GpuParamsType& parent);

		/** @copydoc TGpuDataParam::Set */
		void Set(const SPtr<SamplerState>& samplerState, u32 arrayIndex = 0) const;

		/** @copydoc TGpuDataParam::Get */
		SPtr<SamplerState> Get(u32 arrayIndex = 0) const;

		/**	Checks if param is initialized. */
		bool operator==(const std::nullptr_t& nullval) const
		{
			return !mBinding.IsValid();
		}

	protected:
		GpuParamsType mParent;
		GpuParameterBinding mBinding;
	};

	typedef TGpuParameterPrimitive<float, false> GpuParameterFloat;
	typedef TGpuParameterPrimitive<double, false> GpuParameterDouble;
	typedef TGpuParameterPrimitive<Vector2, false> GpuParameterVector2;
	typedef TGpuParameterPrimitive<Vector3, false> GpuParameterVector3;
	typedef TGpuParameterPrimitive<Vector4, false> GpuParameterVector4;
	typedef TGpuParameterPrimitive<i32, false> GpuParameterI32;
	typedef TGpuParameterPrimitive<Vector2I, false> GpuParameterVector2I;
	typedef TGpuParameterPrimitive<Vector3I, false> GpuParameterVector3I;
	typedef TGpuParameterPrimitive<Vector4I, false> GpuParameterVector4I;
	typedef TGpuParameterPrimitive<u32, false> GpuParameterU32;
	typedef TGpuParameterPrimitive<Vector2UI, false> GpuParameterVector2UI;
	typedef TGpuParameterPrimitive<Vector3UI, false> GpuParameterVector3UI;
	typedef TGpuParameterPrimitive<Vector4UI, false> GpuParameterVector4UI;
	typedef TGpuParameterPrimitive<Matrix3, false> GpuParameterMatrix3;
	typedef TGpuParameterPrimitive<Matrix4, false> GpuParameterMatrix4;
	typedef TGpuParameterPrimitive<Color, false> GpuParameterColor;

	typedef TGpuParameterStruct<false> GpuParameterStruct;
	typedef TGpuParameterStorageBuffer<false> GpuParameterStorageBuffer;
	typedef TGpuParameterUniformBuffer<false> GpuParameterUniformBuffer;
	typedef TGpuParameterSampler<false> GpuParameterSampler;
	typedef TGpuParameterSampledTexture<false> GpuParameterSampledTexture;
	typedef TGpuParameterStorageTexture<false> GpuParameterStorageTexture;

	namespace render
	{
		typedef TGpuParameterPrimitive<float, true> GpuParameterFloat;
		typedef TGpuParameterPrimitive<double, true> GpuParameterDouble;
		typedef TGpuParameterPrimitive<Vector2, true> GpuParameterVector2;
		typedef TGpuParameterPrimitive<Vector3, true> GpuParameterVector3;
		typedef TGpuParameterPrimitive<Vector4, true> GpuParameterVector4;
		typedef TGpuParameterPrimitive<i32, true> GpuParameterI32;
		typedef TGpuParameterPrimitive<Vector2I, true> GpuParameterVector2I;
		typedef TGpuParameterPrimitive<Vector3I, true> GpuParameterVector3I;
		typedef TGpuParameterPrimitive<Vector4I, true> GpuParameterVector4I;
		typedef TGpuParameterPrimitive<u32, true> GpuParameterU32;
		typedef TGpuParameterPrimitive<Vector2UI, true> GpuParameterVector2UI;
		typedef TGpuParameterPrimitive<Vector3UI, true> GpuParameterVector3UI;
		typedef TGpuParameterPrimitive<Vector4UI, true> GpuParameterVector4UI;
		typedef TGpuParameterPrimitive<Matrix3, true> GpuParameterMatrix3;
		typedef TGpuParameterPrimitive<Matrix4, true> GpuParameterMatrix4;
		typedef TGpuParameterPrimitive<Color, true> GpuParameterColor;

		typedef TGpuParameterStruct<true> GpuParameterStruct;
		typedef TGpuParameterStorageBuffer<true> GpuParameterStorageBuffer;
		typedef TGpuParameterUniformBuffer<true> GpuParameterUniformBuffer;
		typedef TGpuParameterSampler<true> GpuParameterSampler;
		typedef TGpuParameterSampledTexture<true> GpuParameterSampledTexture;
		typedef TGpuParameterStorageTexture<true> GpuParameterStorageTexture;
	} // namespace render

	/** @} */
} // namespace b3d
