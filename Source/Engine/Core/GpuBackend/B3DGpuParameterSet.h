//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuParameter.h"
#include "GpuBackend/B3DGpuBufferPool.h"
#include "CoreObject/B3DCoreObject.h"
#include "Resources/B3DIResourceListener.h"
#include "Math/B3DMatrixNxM.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
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

	/** Contains functionality common for both main and render thread versions of GpuParameterSet. */
	class B3D_EXPORT GpuParametersSetBase
	{
	public:
		virtual ~GpuParametersSetBase() = default;

		// Note: Disallow copy/assign because it would require some care when copying (copy internal data shared_ptr and
		// all the internal buffers too). Trivial to implement but not needed at this time. Un-delete and implement if necessary.
		GpuParametersSetBase(const GpuParametersSetBase& other) = delete;
		GpuParametersSetBase& operator=(const GpuParametersSetBase& rhs) = delete;

		/** Gets the object that contains the processed information about all parameters. */
		TShared<GpuPipelineParameterSetLayout> GetLayout() const { return mParameterSetLayout; }

		/** Returns the set that this object is responsible binding parameters for. */
		u32 GetSet() const { return mSet; }

		/** Checks if parameter with the specified name exists. */
		bool HasParameter(const StringView& name) const;

		/**	Checks if texture parameter with the specified name exists. */
		bool HasSampledTexture(const StringView& name) const;

		/**	Checks if load/store texture parameter with the specified name exists. */
		bool HasStorageTexture(const StringView& name) const;

		/**	Checks if buffer parameter with the specified name exists. */
		bool HasStorageBuffer(const StringView& name) const;

		/**	Checks if sampler state parameter with the specified name exists. */
		bool HasSamplerState(const StringView& name) const;

		/** Checks if a uniform buffer with the specified name exists for the specific GPU program type. */
		bool HasUniformBuffer(const StringView& name) const;

		/** Marks the main thread object as dirty, causing it to sync its contents with its render thread counterpart. */
		virtual void MarkRenderProxyDataDirtyInternal() {}

		/** @copydoc IResourceListener::MarkListenerResourcesDirty */
		virtual void MarkResourcesDirtyInternal() {}

	protected:
		GpuParametersSetBase(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex);

		TShared<GpuPipelineParameterSetLayout> mParameterSetLayout;
		u32 mSet = 0;
	};

	/** Templated version of GpuParameterSet that contains functionality for both main and render thread versions of stored data. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TGpuParameterSet : public GpuParametersSetBase
	{
	public:
		using GpuParametersType = CoreVariantType<GpuParameterSet, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using BufferType = TShared<CoreVariantType<GpuBuffer, IsRenderProxy>>;
		using UniformBufferType = TShared<CoreVariantType<GpuBuffer, IsRenderProxy>>;

		virtual ~TGpuParameterSet();

		/**
		 * Returns a handle for the parameter with the specified name. The handle writes and reads the parameter in this set, or in any
		 * other set with the same layout, without further lookups.
		 *
		 * Logs a warning if a parameter with that name and type doesn't exist.
		 */
		template <class T>
		void GetParameter(const StringView& name, TGpuParameterPrimitive<T, IsRenderProxy>& output) const;

		/** @copydoc GetParameter */
		void GetStructParameter(const StringView& name, TGpuParameterStruct<IsRenderProxy>& output) const;

		/** @copydoc GetParameter */
		void GetSampledTextureParameter(const StringView& name, TGpuParameterSampledTexture<IsRenderProxy>& output) const;

		/** @copydoc GetParameter */
		void GetStorageTextureParameter(const StringView& name, TGpuParameterStorageTexture<IsRenderProxy>& output) const;

		/** @copydoc GetParameter */
		void GetStorageBufferParameter(const StringView& name, TGpuParameterStorageBuffer<IsRenderProxy>& output) const;

		/** @copydoc GetParameter */
		void GetUniformBufferParameter(const StringView& name, TGpuParameterUniformBuffer<IsRenderProxy>& output) const;

		/** @copydoc GetParameter */
		void GetSamplerStateParameter(const StringView& name, TGpuParameterSampler<IsRenderProxy>& output) const;

		/** Equivalent to GetParam(), but doesn't warn if the parameter cannot be found. Return true if the parameter was found. */
		template<class T> bool TryGetParameter(const StringView& name, TGpuParameterPrimitive<T, IsRenderProxy>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetStructParameter(const StringView& name, TGpuParameterStruct<IsRenderProxy>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetSampledTextureParameter(const StringView& name, TGpuParameterSampledTexture<IsRenderProxy>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetStorageTextureParameter(const StringView& name, TGpuParameterStorageTexture<IsRenderProxy>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetStorageBufferParameter(const StringView& name, TGpuParameterStorageBuffer<IsRenderProxy>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetUniformBufferParameter(const StringView& name, TGpuParameterUniformBuffer<IsRenderProxy>& output) const;

		/** @copydoc TryGetParameter */
		bool TryGetSamplerStateParameter(const StringView& name, TGpuParameterSampler<IsRenderProxy>& output) const;

		/**	Gets a uniform buffer from the specified slot/array index combination. */
		UniformBufferType GetUniformBuffer(u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a texture bound to the specified slot/array index combination. */
		TextureType GetSampledTexture(u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a storage texture bound to the specified slot/array index combination. */
		TextureType GetStorageTexture(u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a buffer bound to the specified slot/array index combination. */
		BufferType GetStorageBuffer(u32 slot, u32 arrayIndex = 0) const;

		/**	Gets a sampler state bound to the specified slot/array index combination. */
		TShared<SamplerState> GetSamplerState(u32 slot, u32 arrayIndex = 0) const;

		/** Gets information that determines which texture surfaces to bind as a sampled texture parameter. */
		const TextureSurface& GetTextureSurface(u32 slot, u32 arrayIndex = 0) const;

		/** Gets information that determines which texture surfaces to bind as a storage texture parameter. */
		const TextureSurface& GetStorageTextureSurface(u32 slot, u32 arrayIndex = 0) const;

		/**
		 * Sets an uniform buffer at the specified slot. It is up to the caller to guarantee the provided buffer matches uniform descriptor for this slot.
		 *
		 * @param	slot		Slot at which to bind the buffer, as defined by the pipeline GPU program.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	offset		Dynamic offset in the buffer, at which the to start reading the buffer.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetUniformBuffer(u32 slot, const UniformBufferType& buffer, u32 arrayIndex = 0, u32 offset = 0);

		/**
		 * Sets an uniform buffer with the specified name in all GPU programs containing a buffer with that name. It is up to the caller to guarantee the provided buffer matches
		 * uniform buffer descriptor for this slot.
		 *
		 * @param	name		Name of the buffer to bind.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	offset		Dynamic offset in the buffer, at which the to start reading the buffer.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		bool SetUniformBuffer(const StringView& name, const UniformBufferType& buffer, u32 arrayIndex = 0, u32 offset = 0);

		/** Equivalent to SetUniformBuffer(const String&, const UniformBufferType&, u32, u32), but doesn't warn if the parameter cannot be found. Return true if the parameter was found. */
		bool TrySetUniformBuffer(const StringView& name, const UniformBufferType& uniformBuffer, u32 arrayIndex = 0, u32 offset = 0);

		/**
		 * Sets a texture at the specified slot.
		 * Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetSampledTexture(u32 slot, const TextureType& texture, const TextureSurface& surface = TextureSurface::kComplete, u32 arrayIndex = 0);

		/**
		 * Sets a storage texture at the specified slot.
		 * Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetStorageTexture(u32 slot, const TextureType& texture, const TextureSurface& surface, u32 arrayIndex = 0);

		/**
		 * Sets a storage buffer at the specified slot combination.
		 *
		 * @param	slot		Slot at which to bind the buffer, as defined by the pipeline GPU program.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	view		Optional view information that controls how is the buffer viewed when bound to the pipeline.
		 * @return				Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetStorageBuffer(u32 slot, const BufferType& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation());

		/**
		 * Sets a sampler state at the specified slot.
		 * Returns true if the operation succeeded, otherwise logs and errors and returns false.
		 */
		virtual bool SetSamplerState(u32 slot, const TShared<SamplerState>& sampler, u32 arrayIndex = 0);

		/**	Assigns a data value to the parameter with the specified name. */
		template <class T>
		void SetParameter(const StringView& name, const T& value)
		{
			TGpuParameterPrimitive<T, IsRenderProxy> param;
			GetParameter(name, param);
			param.Set(GetSelf(), value);
		}

		/**	Assigns a texture to the parameter with the specified name. */
		void SetSampledTexture(const StringView& name, const TextureType& texture, const TextureSurface& surface = TextureSurface::kComplete)
		{
			TGpuParameterSampledTexture<IsRenderProxy> param;
			GetSampledTextureParameter(name, param);
			param.Set(GetSelf(), texture, surface);
		}

		/**	Assigns a load/store texture to the parameter with the specified name. */
		void SetStorageTexture(const StringView& name, const TextureType& texture, const TextureSurface& surface)
		{
			TGpuParameterStorageTexture<IsRenderProxy> param;
			GetStorageTextureParameter(name, param);
			param.Set(GetSelf(), texture, surface);
		}

		/**
		 * Sets a storage buffer with the specified name.
		 *
		 * @param	name		Name of the buffer to bind.
		 * @param	buffer		Buffer to bind.
		 * @param	arrayIndex	In case the bind point represents an array, index to bind the buffer to.
		 * @param	view		Optional view information that controls how is the buffer viewed when bound to the pipeline.
		 */
		void SetStorageBuffer(const StringView& name, const BufferType& buffer, u32 arrayIndex = 0, GpuBufferViewInformation view = GpuBufferViewInformation())
		{
			TGpuParameterStorageBuffer<IsRenderProxy> param;
			GetStorageBufferParameter(name, param);
			param.Set(GetSelf(), buffer, arrayIndex, view);
		}

		/**	Assigns a sampler state to the parameter with the specified name. */
		void SetSamplerState(const StringView& name, const TShared<SamplerState>& sampler)
		{
			TGpuParameterSampler<IsRenderProxy> param;
			GetSamplerStateParameter(name, param);
			param.Set(GetSelf(), sampler);
		}

	protected:
		TGpuParameterSet(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex);

		virtual TShared<GpuParametersType> GetSelf() const = 0;

		/** Data for a single bound texture. */
		struct TextureData
		{
			TextureType Texture;
			TextureSurface Surface;
		};

		/** Data for a single bound storage buffer. */
		struct StorageBufferData
		{
			BufferType Buffer;
			GpuBufferViewInformation View; /**< Controls how is the buffer viewed when bound to the pipeline. */
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
		TShared<SamplerState>* mSamplerStates = nullptr;
	};

	/** @} */

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Contains descriptions for all parameters in a set of programs (ones for each stage) and allows you to write and read
	 * those parameters. All parameter values are stored internally on the CPU, and are only submitted to the GPU once the
	 * parameters are bound to the pipeline.
	 *
	 * @note	Main thread only.
	 */
	class B3D_EXPORT GpuParameterSet : public CoreObject, public TGpuParameterSet<false>, public IResourceListener
	{
	public:
		~GpuParameterSet() {}

		/**
		 * Creates a new set of GPU parameters using an object describing the parameters.
		 *
		 * @param	parameterSetLayout	Description of GPU parameters for a specific GPU pipeline state.
		 * @param	setIndex			Index of the parameter set within the pipeline.
		 */
		static TShared<GpuParameterSet> Create(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex = 0);

		/** Contains a lookup table for sizes of all data parameters. Sizes are in bytes. */
		const static GpuDataParameterTypeInformationLookup kParamSizes;

		/** @name Internal
		 *  @{
		 */

		void MarkRenderProxyDataDirtyInternal() override;
		void MarkResourcesDirtyInternal() override;

		/** @} */
	protected:
		struct SyncPacket;
		friend class render::GpuParameterSet;

		GpuParameterSet(const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout, u32 setIndex);

		TShared<GpuParameterSet> GetSelf() const override;
		TShared<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override { MarkRenderProxyDataDirty(); }
		void NotifyResourceChanged(const HResource& resource) override { MarkRenderProxyDataDirty(); }
	};

	/** @} */

	namespace render
	{
		struct GpuRenderPassAttachmentUsage;

		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Buffer or texture bound to a slot of a parameter set. See GpuParameterSet::ForEachBoundResource(). */
		struct GpuParameterSetResource
		{
			const UniformInformation* Uniform = nullptr; /**< Slot the resource is bound to. From the pipeline set layout, if one was provided. */
			u32 ArrayIndex = 0; /**< Index of the resource in the slot's array. */
			GpuBuffer* Buffer = nullptr; /**< Bound buffer of a uniform or storage buffer slot. */
			render::Texture* Texture = nullptr; /**< Bound texture of a sampled or storage texture slot. */
			TextureSurface Surface; /**< Surfaces of Texture the slot views. */
			GpuAccessFlags Access; /**< Read for uniform buffers and sampled textures. Storage slots read and write, unless the shader declares a read-only buffer. */
		};

		/**
		 * Render thread version of b3d::GpuParameterSet.
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT GpuParameterSet : public RenderProxy, public TGpuParameterSet<true>
		{
		public:
			virtual ~GpuParameterSet() = default;

			// Bring base class overloads into scope
			using TGpuParameterSet::SetUniformBuffer;
			using TGpuParameterSet::TrySetUniformBuffer;

			/** Returns the descriptor pool this parameter set was allocated from. */
			GpuParameterSetPool* GetOwnerPool() const { return mOwnerPool; }

			/**
			 * Writes the changes since the last flush into the native descriptors of the set: Set* calls, and bound buffers and textures
			 * whose native resource changed, such as on a discard write or a relocation. Native descriptors that a command buffer still
			 * uses are replaced instead of written. Only on the thread that owns the set's pool.
			 *
			 * @param	attachments		Attachments of the render pass the set is bound in, or empty outside of render passes. Sampled
			 *							textures among them are read in the layout of the attachment, on backends whose descriptors hold
			 *							image layouts.
			 */
			virtual void Flush(TArrayView<const GpuRenderPassAttachmentUsage> attachments = {}) { }

			/**
			 * Sets uniform buffer using a pool suballocation at the specified slot.
			 *
			 * @param slot				Binding slot within set.
			 * @param suballocation		Pool suballocation handle.
			 * @param arrayIndex		Array index if binding is an array.
			* @return					True if successful, false if binding not found.
			 */
			bool SetUniformBuffer(u32 slot, const GpuBufferSuballocation& suballocation, u32 arrayIndex = 0)
			{
				B3D_ASSERT(suballocation.IsValid());

				return SetUniformBuffer(slot, suballocation.GetBuffer(), arrayIndex, suballocation.GetSuballocationOffset());
			}

			/**
			 * Sets uniform buffer using a pool suballocation (by name).
			 *
			 * @param name				Parameter name.
			 * @param suballocation		Pool suballocation handle.
			 * @param arrayIndex		Array index if binding is an array.
			 * @return					True if successful, false if binding not found.
			 */
			bool SetUniformBuffer(const StringView& name, const GpuBufferSuballocation& suballocation, u32 arrayIndex = 0)
			{
				B3D_ASSERT(suballocation.IsValid());

				return SetUniformBuffer(name, suballocation.GetBuffer(), arrayIndex, suballocation.GetSuballocationOffset());
			}

			/**
			 * Tries to set uniform buffer using a pool suballocation (no warnings).
			 *
			 * @param name				Parameter name.
			 * @param suballocation		Pool suballocation handle
			 * @param arrayIndex		Array index if binding is an array
			 * @return					True if successful, false if binding not found
			 */
			bool TrySetUniformBuffer(const StringView& name, const GpuBufferSuballocation& suballocation, u32 arrayIndex = 0)
			{
				if (!suballocation.IsValid())
					return false;

				return TrySetUniformBuffer(name, suballocation.GetBuffer(), arrayIndex, suballocation.GetSuballocationOffset());
			}

			/**
			 * Calls @p fnVisit with a GpuParameterSetResource for every buffer and texture bound to the set. Empty slots and samplers are
			 * skipped.
			 *
			 * @param	pipelineSetLayout	If provided, only the slots of this layout that the set has with the same type are visited,
			 *								which are the slots a pipeline with the layout accesses. Otherwise every slot of the set is.
			 * @param	fnVisit				Callable taking a const GpuParameterSetResource&.
			 */
			template<class VisitFunction>
			void ForEachBoundResource(const GpuPipelineParameterSetLayout* pipelineSetLayout, VisitFunction&& fnVisit) const;

		protected:
			friend class b3d::GpuParameterSet;
			friend class b3d::GpuParameterSetPool;

			GpuParameterSet(const TShared<GpuPipelineParameterSetLayout>& parameterLayout, u32 setIndex);

			TShared<GpuParameterSet> GetSelf() const override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			/** Returns true if the calling thread owns the set's pool, and may therefore write the set. */
			bool IsOwnedByThisThread() const { return mOwnerPool == nullptr || mOwnerPool->GetOwnerThread() == B3D_CURRENT_THREAD_ID; }

			GpuParameterSetPool* mOwnerPool = nullptr;
		};

		template<class VisitFunction>
		void GpuParameterSet::ForEachBoundResource(const GpuPipelineParameterSetLayout* pipelineSetLayout, VisitFunction&& fnVisit) const
		{
			const GpuPipelineParameterSetLayout& setLayout = *mParameterSetLayout;
			const GpuPipelineParameterSetLayout& visitedLayout = pipelineSetLayout != nullptr ? *pipelineSetLayout : setLayout;
			for(u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
			{
				const GpuParameterType type = (GpuParameterType)typeIndex;
				if(type == GpuParameterType::Sampler)
					continue;

				for(u32 bindingIndex = 0; bindingIndex < visitedLayout.GetBindingCount(type); bindingIndex++)
				{
					const UniformInformation& uniform = *visitedLayout.TryGetUniformInformation(type, bindingIndex);
					u32 arraySize = uniform.ArraySize;
					if(pipelineSetLayout != nullptr)
					{
						const UniformInformation* setUniform = setLayout.TryGetUniformInformation(uniform.Slot);
						if(setUniform == nullptr || setUniform->Type != type)
							continue;

						arraySize = std::min(arraySize, setUniform->ArraySize);
					}

					GpuParameterSetResource resource;
					resource.Uniform = &uniform;
					for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						resource.ArrayIndex = arrayIndex;
						switch(type)
						{
						case GpuParameterType::UniformBuffer:
							resource.Buffer = GetUniformBuffer(uniform.Slot, arrayIndex).get();
							resource.Access = GpuAccessFlag::Read;
							break;
						case GpuParameterType::StorageBuffer:
							resource.Buffer = GetStorageBuffer(uniform.Slot, arrayIndex).get();
							resource.Access = GpuObjectParameterTypeInformation::IsReadWriteBuffer(uniform.ObjectType) ? GpuAccessFlag::Read | GpuAccessFlag::Write : GpuAccessFlags(GpuAccessFlag::Read);
							break;
						case GpuParameterType::SampledTexture:
							resource.Texture = GetSampledTexture(uniform.Slot, arrayIndex).get();
							resource.Surface = GetTextureSurface(uniform.Slot, arrayIndex);
							resource.Access = GpuAccessFlag::Read;
							break;
						case GpuParameterType::StorageTexture:
							resource.Texture = GetStorageTexture(uniform.Slot, arrayIndex).get();
							resource.Surface = GetStorageTextureSurface(uniform.Slot, arrayIndex);
							resource.Access = GpuAccessFlag::Read | GpuAccessFlag::Write;
							break;
						default:
							break;
						}

						if(resource.Buffer != nullptr || resource.Texture != nullptr)
							fnVisit(static_cast<const GpuParameterSetResource&>(resource));
					}
				}
			}
		}

		/** @} */
	} // namespace render
} // namespace b3d
