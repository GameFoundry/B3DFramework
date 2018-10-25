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
	template<class T> struct TGpuDataParamInfo { enum { TypeId = GPDT_STRUCT };};
	template<> struct TGpuDataParamInfo < float > { enum { TypeId = GPDT_FLOAT1 }; };
	template<> struct TGpuDataParamInfo < Vector2 > { enum { TypeId = GPDT_FLOAT2 }; };
	template<> struct TGpuDataParamInfo < Vector3 > { enum { TypeId = GPDT_FLOAT3 }; };
	template<> struct TGpuDataParamInfo < Vector4 > { enum { TypeId = GPDT_FLOAT4 }; };
	template<> struct TGpuDataParamInfo < int > { enum { TypeId = GPDT_INT1 }; };
	template<> struct TGpuDataParamInfo < Vector2I > { enum { TypeId = GPDT_INT2 }; };
	template<> struct TGpuDataParamInfo < Vector3I > { enum { TypeId = GPDT_INT3 }; };
	template<> struct TGpuDataParamInfo < Vector4I > { enum { TypeId = GPDT_INT4 }; };
	template<> struct TGpuDataParamInfo < Matrix2 > { enum { TypeId = GPDT_MATRIX_2X2 }; };
	template<> struct TGpuDataParamInfo < Matrix2x3 > { enum { TypeId = GPDT_MATRIX_2X3 }; };
	template<> struct TGpuDataParamInfo < Matrix2x4 > { enum { TypeId = GPDT_MATRIX_2X4 }; };
	template<> struct TGpuDataParamInfo < Matrix3 > { enum { TypeId = GPDT_MATRIX_3X3 }; };
	template<> struct TGpuDataParamInfo < Matrix3x2 > { enum { TypeId = GPDT_MATRIX_3X2 }; };
	template<> struct TGpuDataParamInfo < Matrix3x4 > { enum { TypeId = GPDT_MATRIX_3X4 }; };
	template<> struct TGpuDataParamInfo < Matrix4 > { enum { TypeId = GPDT_MATRIX_4X4 }; };
	template<> struct TGpuDataParamInfo < Matrix4x2 > { enum { TypeId = GPDT_MATRIX_4X2 }; };
	template<> struct TGpuDataParamInfo < Matrix4x3 > { enum { TypeId = GPDT_MATRIX_4X3 }; };
	template<> struct TGpuDataParamInfo < Color > { enum { TypeId = GPDT_COLOR }; };

	class GpuPipelineParamInfoBase;

	/** Contains functionality common for both sim and core thread version of GpuParams. */
	class BS_CORE_EXPORT GpuParamsBase
	{
	public:
		virtual ~GpuParamsBase() = default;

		// Note: Disallow copy/assign because it would require some care when copying (copy internal data shared_ptr and
		// all the internal buffers too). Trivial to implement but not needed at this time. Un-delete and implement if necessary.
		GpuParamsBase(const GpuParamsBase& other) = delete;
		GpuParamsBase& operator=(const GpuParamsBase& rhs) = delete;

		/** Returns a description of all stored parameters. */
		SPtr<GpuParamDesc> getParamDesc(GpuProgramType type) const;

		/** Gets the object that contains the processed information about all parameters. */
		SPtr<GpuPipelineParamInfoBase> getParamInfo() const { return mParamInfo; }

		/**
		 * Returns the size of a data parameter with the specified name, in bytes. Returns 0 if such parameter doesn't exist.
		 */
		UINT32 getDataParamSize(GpuProgramType type, const String& name) const;

		/** Checks if parameter with the specified name exists. */
		bool hasParam(GpuProgramType type, const String& name) const;

		/**	Checks if texture parameter with the specified name exists. */
		bool hasTexture(GpuProgramType type, const String& name) const;

		/**	Checks if load/store texture parameter with the specified name exists. */
		bool hasLoadStoreTexture(GpuProgramType type, const String& name) const;

		/**	Checks if buffer parameter with the specified name exists. */
		bool hasBuffer(GpuProgramType type, const String& name) const;

		/**	Checks if sampler state parameter with the specified name exists. */
		bool hasSamplerState(GpuProgramType type, const String& name) const;

		/** Checks if a parameter block with the specified name exists. */
		bool hasParamBlock(GpuProgramType type, const String& name) const;

		/**	Gets a descriptor for a parameter block buffer with the specified name. */
		GpuParamBlockDesc* getParamBlockDesc(GpuProgramType type, const String& name) const;

		/** Marks the sim thread object as dirty, causing it to sync its contents with its core thread counterpart. */
		virtual void _markCoreDirty() { }

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		virtual void _markResourcesDirty() { }

	protected:
		GpuParamsBase(const SPtr<GpuPipelineParamInfoBase>& paramInfo);

		/**	Gets a descriptor for a data parameter with the specified name. */
		GpuParamDataDesc* getParamDesc(GpuProgramType type, const String& name) const;

		SPtr<GpuPipelineParamInfoBase> mParamInfo;
	};

	/** Templated version of GpuParams that contains functionality for both sim and core thread versions of stored data. */
	template <bool Core>
	class BS_CORE_EXPORT TGpuParams : public GpuParamsBase
	{
	public:
		using GpuParamsType = CoreVariantType<GpuParams, Core>;
		using TextureType = CoreVariantHandleType<Texture, Core>;
		using BufferType = SPtr<CoreVariantType<GpuBuffer, Core>>;
		using SamplerType = SPtr<CoreVariantType<SamplerState, Core>>;
		using ParamsBufferType = SPtr<CoreVariantType<GpuParamBlockBuffer, Core>>;

		virtual ~TGpuParams();

		/**
		 * Returns a handle for the parameter with the specified name. Handle may then be stored and used for quickly 
		 * setting or retrieving values to/from that parameter.
		 *
		 * Throws exception if parameter with that name and type doesn't exist.
		 *
		 * Parameter handles will be invalidated when their parent GpuParams object changes.
		 */
		template<class T> void getParam(GpuProgramType type, const String& name, TGpuDataParam<T, Core>& output) const;

		/** @copydoc getParam */
		void getStructParam(GpuProgramType type, const String& name, TGpuParamStruct<Core>& output) const;

		/** @copydoc getParam */
		void getTextureParam(GpuProgramType type, const String& name, TGpuParamTexture<Core>& output) const;

		/** @copydoc getParam */
		void getLoadStoreTextureParam(GpuProgramType type, const String& name, TGpuParamLoadStoreTexture<Core>& output) const;

		/** @copydoc getParam */
		void getBufferParam(GpuProgramType type, const String& name, TGpuParamBuffer<Core>& output) const;

		/** @copydoc getParam */
		void getSamplerStateParam(GpuProgramType type, const String& name, TGpuParamSampState<Core>& output) const;

		/**	Gets a parameter block buffer from the specified set/slot combination. */
		ParamsBufferType getParamBlockBuffer(UINT32 set, UINT32 slot) const;

		/**	Gets a texture bound to the specified set/slot combination. */
		TextureType getTexture(UINT32 set, UINT32 slot) const;

		/**	Gets a load/store texture bound to the specified set/slot combination. */
		TextureType getLoadStoreTexture(UINT32 set, UINT32 slot) const;

		/**	Gets a buffer bound to the specified set/slot combination. */
		BufferType getBuffer(UINT32 set, UINT32 slot) const;

		/**	Gets a sampler state bound to the specified set/slot combination. */
		SamplerType getSamplerState(UINT32 set, UINT32 slot) const;

		/** Gets information that determines which texture surfaces to bind as a sampled texture parameter. */
		const TextureSurface& getTextureSurface(UINT32 set, UINT32 slot) const;

		/** Gets information that determines which texture surfaces to bind as a load/store parameter. */
		const TextureSurface& getLoadStoreSurface(UINT32 set, UINT32 slot) const;

		/**
		 * Assigns the provided parameter block buffer to a buffer with the specified name, for the specified GPU program
		 * stage. Any following parameter reads or writes that are referencing that buffer will use the new buffer.
		 *
		 * It is up to the caller to guarantee the provided buffer matches parameter block descriptor for this slot.
		 */
		void setParamBlockBuffer(GpuProgramType type, const String& name, const ParamsBufferType& paramBlockBuffer);

		/**
		 * Assigns the provided parameter block buffer to a buffer with the specified name, for any stages that reference
		 * the buffer. Any following parameter reads or writes that are referencing that buffer will use the new buffer.
		 *
		 * It is up to the caller to guarantee the provided buffer matches parameter block descriptor for this slot.
		 * It is up to the caller that all stages using this buffer name refer to the same buffer type.
		 */
		void setParamBlockBuffer(const String& name, const ParamsBufferType& paramBlockBuffer);

		/**
		 * Sets the parameter buffer with the specified set/slot combination.Any following parameter reads or writes that are 
		 * referencing that buffer will use the new buffer. Set/slot information for a specific buffer can be extracted
		 * from GPUProgram's GpuParamDesc structure.
		 *
		 * It is up to the caller to guarantee the provided buffer matches parameter block descriptor for this slot.
		 */
		virtual void setParamBlockBuffer(UINT32 set, UINT32 slot, const ParamsBufferType& paramBlockBuffer);

		/**	Sets a texture at the specified set/slot combination. */
		virtual void setTexture(UINT32 set, UINT32 slot, const TextureType& texture, 
								const TextureSurface& surface = TextureSurface::COMPLETE);

		/**	Sets a load/store texture at the specified set/slot combination. */
		virtual void setLoadStoreTexture(UINT32 set, UINT32 slot, const TextureType& texture, const TextureSurface& surface);

		/**	Sets a buffer at the specified set/slot combination. */
		virtual void setBuffer(UINT32 set, UINT32 slot, const BufferType& buffer);

		/**	Sets a sampler state at the specified set/slot combination. */
		virtual void setSamplerState(UINT32 set, UINT32 slot, const SamplerType& sampler);

		/**	Assigns a data value to the parameter with the specified name. */
		template<class T> void setParam(GpuProgramType type, const String& name, const T& value)
		{ TGpuDataParam<T, Core> param; getParam(type, name, param); param.set(value); }

		/**	Assigns a texture to the parameter with the specified name. */
		void setTexture(GpuProgramType type, const String& name, const TextureType& texture,
						const TextureSurface& surface = TextureSurface::COMPLETE)
		{ TGpuParamTexture<Core> param; getTextureParam(type, name, param); param.set(texture, surface); }

		/**	Assigns a load/store texture to the parameter with the specified name. */
		void setLoadStoreTexture(GpuProgramType type, const String& name, const TextureType& texture, const TextureSurface& surface)
		{ TGpuParamLoadStoreTexture<Core> param; getLoadStoreTextureParam(type, name, param); param.set(texture, surface); }

		/**	Assigns a buffer to the parameter with the specified name. */
		void setBuffer(GpuProgramType type, const String& name, const BufferType& buffer)
		{ TGpuParamBuffer<Core> param; getBufferParam(type, name, param); param.set(buffer); }

		/**	Assigns a sampler state to the parameter with the specified name. */
		void setSamplerState(GpuProgramType type, const String& name, const SamplerType& sampler)
		{ TGpuParamSampState<Core> param; getSamplerStateParam(type, name, param); param.set(sampler); }

	protected:
		TGpuParams(const SPtr<GpuPipelineParamInfoBase>& paramInfo);

		/** @copydoc CoreObject::getThisPtr */
		virtual SPtr<GpuParamsType> _getThisPtr() const = 0;

		/** Data for a single bound texture. */
		struct TextureData
		{
			TextureType texture;
			TextureSurface surface;
		};

		ParamsBufferType* mParamBlockBuffers = nullptr;
		TextureData* mSampledTextureData = nullptr;
		TextureData* mLoadStoreTextureData = nullptr;
		BufferType* mBuffers = nullptr;
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
	class BS_CORE_EXPORT GpuParams : public CoreObject, public TGpuParams<false>, public IResourceListener
	{
	public:
		~GpuParams() { }

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<ct::GpuParams> getCore() const;

		/**
		 * Creates new GpuParams object that can serve for changing the GPU program parameters on the specified pipeline.
		 *
		 * @param[in]	pipelineState	Pipeline state for which this object can set parameters for.
		 * @return						New GpuParams object.
		 */
		static SPtr<GpuParams> create(const SPtr<GraphicsPipelineState>& pipelineState);

		/** @copydoc GpuParams::create(const SPtr<GraphicsPipelineState>&) */
		static SPtr<GpuParams> create(const SPtr<ComputePipelineState>& pipelineState);

		/** 
		 * Creates a new set of GPU parameters using an object describing the parameters for a pipeline.
		 * 
		 * @param[in]	paramInfo	Description of GPU parameters for a specific GPU pipeline state.
		 */
		static SPtr<GpuParams> create(const SPtr<GpuPipelineParamInfo>& paramInfo);

		/** Contains a lookup table for sizes of all data parameters. Sizes are in bytes. */
		const static GpuDataParamInfos PARAM_SIZES;

		/** @name Internal
		 *  @{
		 */

		/** @copydoc GpuParamsBase::_markCoreDirty */
		void _markCoreDirty() override;

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		void _markResourcesDirty() override;

		/** @} */
	protected:
		friend class HardwareBufferManager;

		GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo);

		/** @copydoc CoreObject::getThisPtr */
		SPtr<GpuParams> _getThisPtr() const override;

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc IResourceListener::getListenerResources */
		void getListenerResources(Vector<HResource>& resources) override;

		/** @copydoc IResourceListener::notifyResourceLoaded */
		void notifyResourceLoaded(const HResource& resource) override { markCoreDirty(); }

		/** @copydoc IResourceListener::notifyResourceChanged */
		void notifyResourceChanged(const HResource& resource) override { markCoreDirty(); }
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of bs::GpuParams.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuParams : public CoreObject, public TGpuParams<true>
	{
	public:
		virtual ~GpuParams() { }

		/** 
		 * @copydoc bs::GpuParams::create(const SPtr<GraphicsPipelineState>&)
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the buffer be created on.
		 */
		static SPtr<GpuParams> create(const SPtr<GraphicsPipelineState>& pipelineState,
										  GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc bs::GpuParams::create(const SPtr<ComputePipelineState>&) 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the buffer be created on.
		 */
		static SPtr<GpuParams> create(const SPtr<ComputePipelineState>& pipelineState,
										  GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc bs::GpuParams::create(const SPtr<GpuPipelineParamInfo>&)
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the buffer be created on.
		 */
		static SPtr<GpuParams> create(const SPtr<GpuPipelineParamInfo>& paramInfo,
										  GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		friend class bs::GpuParams;
		friend class HardwareBufferManager;

		GpuParams(const SPtr<GpuPipelineParamInfo>& paramInfo, GpuDeviceFlags deviceMask);

		/** @copydoc CoreObject::getThisPtr */
		SPtr<GpuParams> _getThisPtr() const override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;
	};

	/** @} */
	}
}