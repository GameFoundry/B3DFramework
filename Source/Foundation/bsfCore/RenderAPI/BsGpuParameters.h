//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsGpuParam.h"
#include "CoreThread/BsCoreObject.h"
#include "Resources/BsIResourceListener.h"
#include "Math/BsMatrixNxM.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Helper structure whose specializations convert an engine data type into a GPU program data parameter type.  */
	template <class T>
	struct TGpuDataParamInfo
	{
		enum
		{
			TypeId = GPDT_STRUCT
		};
	};

	template <>
	struct TGpuDataParamInfo<float>
	{
		enum
		{
			TypeId = GPDT_FLOAT1
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector2>
	{
		enum
		{
			TypeId = GPDT_FLOAT2
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector3>
	{
		enum
		{
			TypeId = GPDT_FLOAT3
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector4>
	{
		enum
		{
			TypeId = GPDT_FLOAT4
		};
	};

	template<> struct TGpuDataParamInfo<double>
	{
		enum
		{
			TypeId = GPDT_DOUBLE1
		};
	};
	template<> struct TGpuDataParamInfo<Vector2D>
	{
		enum
		{
			TypeId = GPDT_DOUBLE2
		};
	};
	template<> struct TGpuDataParamInfo<Vector3D>
	{
		enum
		{
			TypeId = GPDT_DOUBLE3
		};
	};
	template<> struct TGpuDataParamInfo<Vector4D>
	{
		enum
		{
			TypeId = GPDT_DOUBLE4
		};
	};

	template <>
	struct TGpuDataParamInfo<i32>
	{
		enum
		{
			TypeId = GPDT_INT1
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector2I>
	{
		enum
		{
			TypeId = GPDT_INT2
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector3I>
	{
		enum
		{
			TypeId = GPDT_INT3
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector4I>
	{
		enum
		{
			TypeId = GPDT_INT4
		};
	};

	template <>
	struct TGpuDataParamInfo<u32>
	{
		enum
		{
			TypeId = GPDT_UINT1
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector2UI>
	{
		enum
		{
			TypeId = GPDT_UINT2
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector3UI>
	{
		enum
		{
			TypeId = GPDT_UINT3
		};
	};

	template <>
	struct TGpuDataParamInfo<Vector4UI>
	{
		enum
		{
			TypeId = GPDT_UINT4
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix2>
	{
		enum
		{
			TypeId = GPDT_MATRIX_2X2
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix2x3>
	{
		enum
		{
			TypeId = GPDT_MATRIX_2X3
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix2x4>
	{
		enum
		{
			TypeId = GPDT_MATRIX_2X4
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix3>
	{
		enum
		{
			TypeId = GPDT_MATRIX_3X3
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix3x2>
	{
		enum
		{
			TypeId = GPDT_MATRIX_3X2
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix3x4>
	{
		enum
		{
			TypeId = GPDT_MATRIX_3X4
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix4>
	{
		enum
		{
			TypeId = GPDT_MATRIX_4X4
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix4x2>
	{
		enum
		{
			TypeId = GPDT_MATRIX_4X2
		};
	};

	template <>
	struct TGpuDataParamInfo<Matrix4x3>
	{
		enum
		{
			TypeId = GPDT_MATRIX_4X3
		};
	};

	template <>
	struct TGpuDataParamInfo<Color>
	{
		enum
		{
			TypeId = GPDT_COLOR
		};
	};

	class GpuPipelineParamInfoBase;

	/** Contains functionality common for both sim and core thread version of GpuParameters. */
	class B3D_CORE_EXPORT GpuParamsBase
	{
	public:
		virtual ~GpuParamsBase() = default;

		// Note: Disallow copy/assign because it would require some care when copying (copy internal data shared_ptr and
		// all the internal buffers too). Trivial to implement but not needed at this time. Un-delete and implement if necessary.
		GpuParamsBase(const GpuParamsBase& other) = delete;
		GpuParamsBase& operator=(const GpuParamsBase& rhs) = delete;

		/** Returns a description of all stored parameters. */
		SPtr<GpuParamDesc> GetParameterInformation(GpuProgramType type) const;

		/** Gets the object that contains the processed information about all parameters. */
		SPtr<GpuPipelineParamInfoBase> GetPipelineParameterInformation() const { return mParameterLayout; }

		/**
		 * Returns the size of a data parameter with the specified name, in bytes. Returns 0 if such parameter doesn't exist.
		 */
		u32 GetDataParameterSize(GpuProgramType type, const String& name) const;

		/** Checks if parameter with the specified name exists. */
		bool HasParameter(GpuProgramType type, const String& name) const;

		/**	Checks if texture parameter with the specified name exists. */
		bool HasSampledTexture(GpuProgramType type, const String& name) const;

		/**	Checks if load/store texture parameter with the specified name exists. */
		bool HasStorageTexture(GpuProgramType type, const String& name) const;

		/**	Checks if buffer parameter with the specified name exists. */
		bool HasStorageBuffer(GpuProgramType type, const String& name) const;

		/**	Checks if sampler state parameter with the specified name exists. */
		bool HasSamplerState(GpuProgramType type, const String& name) const;

		/** Checks if a parameter block with the specified name exists for the specific GPU program type. */
		bool HasUniformBuffer(GpuProgramType type, const String& name) const;

		/** Checks if a parameter block with the specified name exists. */
		bool HasUniformBuffer(const String& name) const;

		/**	Gets a descriptor for a parameter block buffer with the specified name. */
		GpuParameterBlockInformation* GetParameterBlockDesc(GpuProgramType type, const String& name) const;

		/** Marks the sim thread object as dirty, causing it to sync its contents with its core thread counterpart. */
		virtual void MarkCoreDirtyInternal() {}

		/** @copydoc IResourceListener::MarkListenerResourcesDirty */
		virtual void MarkResourcesDirtyInternal() {}

	protected:
		GpuParamsBase(const SPtr<GpuPipelineParamInfoBase>& parameterLayout);

		/**	Gets a descriptor for a data parameter with the specified name. */
		GpuDataParameterInformation* GetDataParameterInformation(GpuProgramType type, const String& name) const;

		SPtr<GpuPipelineParamInfoBase> mParameterLayout;
	};

	/** Templated version of GpuParameters that contains functionality for both sim and core thread versions of stored data. */
	template <bool Core>
	class B3D_CORE_EXPORT TGpuParams : public GpuParamsBase
	{
	public:
		using GpuParamsType = CoreVariantType<GpuParameters, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;
		using SamplerType = SPtr<CoreVariantType<SamplerState, Core>>;
		using UniformBufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;

		virtual ~TGpuParams();

		/**
		 * Returns a handle for the parameter with the specified name. Handle may then be stored and used for quickly
		 * setting or retrieving values to/from that parameter.
		 *
		 * Throws exception if parameter with that name and type doesn't exist.
		 *
		 * Parameter handles will be invalidated when their parent GpuParameters object changes.
		 */
		template <class T>
		void GetParameter(GpuProgramType type, const String& name, TGpuParameterPrimitive<T, Core>& output) const;

		/** @copydoc GetParameter */
		void GetStructParameter(GpuProgramType type, const String& name, TGpuParameterStruct<Core>& output) const;

		/** @copydoc GetParameter */
		void GetSampledTextureParameter(GpuProgramType type, const String& name, TGpuParameterSampledTexture<Core>& output) const;

		/** @copydoc GetParameter */
		void GetStorageTextureParameter(GpuProgramType type, const String& name, TGpuParameterStorageTexture<Core>& output) const;

		/** @copydoc GetParameter */
		void GetStorageBufferParameter(GpuProgramType type, const String& name, TGpuParameterBuffer<Core>& output) const;

		/** @copydoc GetParameter */
		void GetSamplerStateParameter(GpuProgramType type, const String& name, TGpuParameterSampler<Core>& output) const;

		/** Equivalent to GetParam(), but doesn't warn if the parameter cannot be found. Return true if the parameter was found. */
		template<class T> bool TryGetParameter(GpuProgramType type, const String& name, TGpuParameterPrimitive<T, Core>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetStructParameter(GpuProgramType type, const String& name, TGpuParameterStruct<Core>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetSampledTextureParameter(GpuProgramType type, const String& name, TGpuParameterSampledTexture<Core>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetStorageTextureParameter(GpuProgramType type, const String& name, TGpuParameterStorageTexture<Core>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetStorageBufferParameter(GpuProgramType type, const String& name, TGpuParameterBuffer<Core>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetSamplerStateParameter(GpuProgramType type, const String& name, TGpuParameterSampler<Core>& output) const;

		/**	Gets a uniform buffer from the specified set/slot/array index combination. */
		UniformBufferType GetUniformBuffer(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a texture bound to the specified set/slot/array index combination. */
		TextureType GetSampledTexture(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a storage texture bound to the specified set/slot/array index combination. */
		TextureType GetStorageTexture(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a buffer bound to the specified set/slot/array index combination. */
		BufferType GetStorageBuffer(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a sampler state bound to the specified set/slot/array index combination. */
		SamplerType GetSamplerState(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/** Gets information that determines which texture surfaces to bind as a sampled texture parameter. */
		const TextureSurface& GetTextureSurface(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/** Gets information that determines which texture surfaces to bind as a storage texture parameter. */
		const TextureSurface& GetStorageTextureSurface(u32 set, u32 slot, u32 arrayIndex = 0) const;

		/**
		 * Sets an uniform buffer at the specified set/slot combination. It is up to the caller to guarantee the provided buffer matches parameter block descriptor for this slot.
		 *
		 * @param	set			Set at which to bind the buffer, as defined by the pipeline GPU program.
		 * @param	slot		Slot at which to bind the buffer, as defined by the pipeline GPU program.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	offset		Dynamic offset in the buffer, at which the to start reading the buffer.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetUniformBuffer(u32 set, u32 slot, const UniformBufferType& buffer, u32 arrayIndex = 0, u32 offset = 0);

		/**
		 * Sets an uniform buffer with the specified name in the specified GPU program type. It is up to the caller to guarantee the provided buffer matches
		 * parameter block descriptor for this slot.
		 *
		 * @param	type		Type of the GPU program in which to bind the buffer.
		 * @param	name		Name of the buffer to bind.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	offset		Dynamic offset in the buffer, at which the to start reading the buffer.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		bool SetUniformBuffer(GpuProgramType type, const String& name, const UniformBufferType& buffer, u32 arrayIndex = 0, u32 offset = 0);

		/**
		 * Sets an uniform buffer with the specified name in all GPU programs containing a buffer with that name. It is up to the caller to guarantee the provided buffer matches
		 * parameter block descriptor for this slot.
		 *
		 * @param	name		Name of the buffer to bind.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	offset		Dynamic offset in the buffer, at which the to start reading the buffer.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		bool SetUniformBuffer(const String& name, const UniformBufferType& buffer, u32 arrayIndex = 0, u32 offset = 0);

		/** Equivalent to SetParamBlockBuffer(const String&, const UniformBufferType&, u32, u32), but doesn't warn if the parameter cannot be found. Return true if the parameter was found. */
		bool TrySetUniformBuffer(const String& name, const UniformBufferType& parameterBlockBuffer, u32 arrayIndex = 0, u32 offset = 0);

		/** Equivalent to SetParamBlockBuffer(GpuProgramType, const String&, const UniformBufferType&, u32, u32), but doesn't warn if the parameter cannot be found. Return true if the parameter was found. */
		bool TrySetUniformBuffer(GpuProgramType type, const String& name, const UniformBufferType& parameterBlockBuffer, u32 arrayIndex = 0, u32 offset = 0);

		/**
		 * Sets a texture at the specified set/slot combination.
		 * Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetSampledTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0);

		/**
		 * Sets a storage texture at the specified set/slot combination.
		 * Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetStorageTexture(u32 set, u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex = 0);

		/**
		 * Sets a storage buffer at the specified set/slot combination.
		 *
		 * @param	set			Set at which to bind the buffer, as defined by the pipeline GPU program.
		 * @param	slot		Slot at which to bind the buffer, as defined by the pipeline GPU program.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	view		Optional view information that controls how is the buffer viewed when bound to the pipeline.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetStorageBuffer(u32 set, u32 slot, const BufferType& buffer, u32 arrayIndex = 0, GpuStorageBufferViewInformation view = GpuStorageBufferViewInformation());

		/**
		 * Sets a sampler state at the specified set/slot combination.
		 * Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetSamplerState(u32 set, u32 slot, const SamplerType& sampler, u32 arrayIndex = 0);

		/**	Assigns a data value to the parameter with the specified name. */
		template <class T>
		void SetParameter(GpuProgramType type, const String& name, const T& value)
		{
			TGpuParameterPrimitive<T, Core> param;
			GetParameter(type, name, param);
			param.Set(value);
		}

		/**	Assigns a texture to the parameter with the specified name. */
		void SetSampledTexture(GpuProgramType type, const String& name, const TextureType& texture, const TextureSurface& surface = TextureSurface::kComplete)
		{
			TGpuParameterSampledTexture<Core> param;
			GetSampledTextureParameter(type, name, param);
			param.Set(texture, surface);
		}

		/**	Assigns a load/store texture to the parameter with the specified name. */
		void SetStorageTexture(GpuProgramType type, const String& name, const TextureType& texture, const TextureSurface& surface)
		{
			TGpuParameterStorageTexture<Core> param;
			GetStorageTextureParameter(type, name, param);
			param.Set(texture, surface);
		}

		/**
		 * Sets a storage buffer with the specified name in the specified GPU program type.
		 *
		 * @param	type		Type of the GPU program in which to bind the buffer.
		 * @param	name		Name of the buffer to bind.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	view		Optional view information that controls how is the buffer viewed when bound to the pipeline.
		 */
		void SetStorageBuffer(GpuProgramType type, const String& name, const BufferType& buffer, u32 arrayIndex = 0, GpuStorageBufferViewInformation view = GpuStorageBufferViewInformation())
		{
			TGpuParameterBuffer<Core> param;
			GetStorageBufferParameter(type, name, param);
			param.Set(buffer, arrayIndex, view);
		}

		/**	Assigns a sampler state to the parameter with the specified name. */
		void SetSamplerState(GpuProgramType type, const String& name, const SamplerType& sampler)
		{
			TGpuParameterSampler<Core> param;
			GetSamplerStateParameter(type, name, param);
			param.Set(sampler);
		}

	protected:
		TGpuParams(const SPtr<GpuPipelineParamInfoBase>& parameterLayout);

		/** @copydoc CoreObject::GetThisPtr */
		virtual SPtr<GpuParamsType> GetThisPtrInternal() const = 0;

		/** Data for a single bound texture. */
		struct TextureData
		{
			TextureType Texture;
			TextureSurface Surface;
		};

		/** Data for a single bound storage buffer. */
		struct StorageBufferData
		{
			StorageBufferData()
			{ }

			BufferType Buffer;
			GpuStorageBufferViewInformation View; /**< Controls how is the buffer viewed when bound to the pipeline. */
		};

		/** Data for a single bound uniform buffer. */
		struct UniformBufferData
		{
			UniformBufferType Buffer;
			u32 Offset = 0; /**< Dynamic buffer offset. */
		};

		UniformBufferData* mUniformBufferData = nullptr;
		TextureData* mSampledTextureData = nullptr;
		TextureData* mStorageTextureData = nullptr;
		StorageBufferData* mStorageBufferData = nullptr;
		SamplerType* mSamplerStates = nullptr;
	};

	/** @} */

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Contains descriptions for all parameters in a set of programs (ones for each stage) and allows you to write and read
	 * those parameters. All parameter values are stored internally on the CPU, and are only submitted to the GPU once the
	 * parameters are bound to the pipeline.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT GpuParameters : public CoreObject, public TGpuParams<false>, public IResourceListener
	{
	public:
		~GpuParameters() {}

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<ct::GpuParameters> GetCore() const;

		/**
		 * Creates new GpuParameters object that can serve for changing the GPU program parameters on the specified pipeline.
		 *
		 * @param[in]	pipelineState	Pipeline state for which this object can set parameters for.
		 * @return						New GpuParameters object.
		 */
		static SPtr<GpuParameters> Create(const SPtr<GraphicsPipelineState>& pipelineState);

		/** @copydoc GpuParameters::Create(const SPtr<GraphicsPipelineState>&) */
		static SPtr<GpuParameters> Create(const SPtr<ComputePipelineState>& pipelineState);

		/**
		 * Creates a new set of GPU parameters using an object describing the parameters for a pipeline.
		 *
		 * @param[in]	parameterLayout	Description of GPU parameters for a specific GPU pipeline state.
		 */
		static SPtr<GpuParameters> Create(const SPtr<GpuPipelineParamInfo>& parameterLayout);

		/** Contains a lookup table for sizes of all data parameters. Sizes are in bytes. */
		const static GpuDataParameterTypeInformationLookup kParamSizes;

		/** @name Internal
		 *  @{
		 */

		void MarkCoreDirtyInternal() override;
		void MarkResourcesDirtyInternal() override;

		/** @} */
	protected:
		GpuParameters(const SPtr<GpuPipelineParamInfo>& paramInfo);

		SPtr<GpuParameters> GetThisPtrInternal() const override;
		SPtr<ct::CoreObject> CreateCore() const override;

		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override { MarkCoreDirty(); }
		void NotifyResourceChanged(const HResource& resource) override { MarkCoreDirty(); }
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Core thread version of bs::GpuParameters.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT GpuParameters : public CoreObject, public TGpuParams<true>
		{
		public:
			virtual ~GpuParameters() = default;

		protected:
			friend class bs::GpuParameters;

			GpuParameters(const SPtr<GpuPipelineParamInfo>& parameterLayout);

			SPtr<GpuParameters> GetThisPtrInternal() const override;
			void SyncToCore(const CoreSyncData& data) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
