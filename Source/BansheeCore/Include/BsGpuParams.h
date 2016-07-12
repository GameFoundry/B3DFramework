//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuParam.h"
#include "BsCoreObject.h"
#include "BsIResourceListener.h"
#include "BsVectorNI.h"
#include "BsMatrixNxM.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Helper structure whose specializations convert an engine data type into a GPU program data parameter type.  */
	template<class T> struct TGpuDataParamInfo { };
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
	template<> struct TGpuDataParamInfo < Matrix2x4 > { enum { TypeId = GPDT_MATRIX_2X3 }; };
	template<> struct TGpuDataParamInfo < Matrix3 > { enum { TypeId = GPDT_MATRIX_3X3 }; };
	template<> struct TGpuDataParamInfo < Matrix3x2 > { enum { TypeId = GPDT_MATRIX_3X2 }; };
	template<> struct TGpuDataParamInfo < Matrix3x4 > { enum { TypeId = GPDT_MATRIX_3X4 }; };
	template<> struct TGpuDataParamInfo < Matrix4 > { enum { TypeId = GPDT_MATRIX_4X4 }; };
	template<> struct TGpuDataParamInfo < Matrix4x2 > { enum { TypeId = GPDT_MATRIX_4X2 }; };
	template<> struct TGpuDataParamInfo < Matrix4x3 > { enum { TypeId = GPDT_MATRIX_4X3 }; };
	template<> struct TGpuDataParamInfo < Color > { enum { TypeId = GPDT_COLOR }; };

	/** Contains functionality common for both sim and core thread version of GpuParams. */
	class BS_CORE_EXPORT GpuParamsBase
	{
	public:
		/**
		 * Creates new GpuParams object using the specified parameter descriptions.
		 *
		 * @param[in]	paramDesc			Reference to parameter descriptions that will be used for finding needed 
		 *									parameters.
		 * @param[in]	transposeMatrices	If true the stored matrices will be transposed before submitted to the GPU 
		 *									(some APIs require different matrix layout).
		 *
		 * @note	You normally do not want to call this manually. Instead use GpuProgram::createParameters.
		 */
		GpuParamsBase(const SPtr<GpuParamDesc>& paramDesc, bool transposeMatrices);
		virtual ~GpuParamsBase();

		// Note: Disallow copy/assign because it would require some care when copying (copy internal data shared_ptr and
		// all the internal buffers too). Trivial to implement but not needed at this time. Un-delete and implement if necessary.
		GpuParamsBase(const GpuParamsBase& other) = delete;
		GpuParamsBase& operator=(const GpuParamsBase& rhs) = delete;

		/** Returns a description of all stored parameters. */
		const GpuParamDesc& getParamDesc() const { return *mParamDesc; }

		/**
		 * Returns the size of a data parameter with the specified name, in bytes. Returns 0 if such parameter doesn't exist.
		 */
		UINT32 getDataParamSize(const String& name) const;

		/** Checks if parameter with the specified name exists. */
		bool hasParam(const String& name) const;

		/**	Checks if texture parameter with the specified name exists. */
		bool hasTexture(const String& name) const;

		/**	Checks if load/store texture parameter with the specified name exists. */
		bool hasLoadStoreTexture(const String& name) const;

		/**	Checks if buffer parameter with the specified name exists. */
		bool hasBuffer(const String& name) const;

		/**	Checks if sampler state parameter with the specified name exists. */
		bool hasSamplerState(const String& name) const;

		/** Checks if a parameter block with the specified name exists. */
		bool hasParamBlock(const String& name) const;

		/**	Gets a descriptor for a parameter block buffer with the specified name. */
		GpuParamBlockDesc* getParamBlockDesc(const String& name) const;

		/** Returns information that determines which texture surfaces to bind as load/store parameters. */
		const TextureSurface& getLoadStoreSurface(UINT32 slot) const;

		/**	Sets information that determines which texture surfaces to bind	as load/store parameters. */
		void setLoadStoreSurface(UINT32 slot, const TextureSurface& surface) const;

		/**	Checks whether matrices should be transformed before being written to the parameter buffer. */
		bool getTransposeMatrices() const { return mTransposeMatrices; }

		/** Marks the sim thread object as dirty, causing it to sync its contents with its core thread counterpart. */
		virtual void _markCoreDirty() { }

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		virtual void _markResourcesDirty() { }

	protected:
		/**	Gets a descriptor for a data parameter with the specified name. */
		GpuParamDataDesc* getParamDesc(const String& name) const;

		SPtr<GpuParamDesc> mParamDesc;

		UINT32 mNumParamBlocks;
		UINT32 mNumTextures;
		UINT32 mNumLoadStoreTextures;
		UINT32 mNumBuffers;
		UINT32 mNumSamplerStates;

		TextureSurface* mLoadStoreSurfaces;

		bool mTransposeMatrices;
	};

	template<bool Core> struct TGpuParamsTypes { };
	template<> struct TGpuParamsTypes < false >
	{
		typedef GpuParams GpuParamsType;
		typedef HTexture TextureType;
		typedef SPtr<GpuBuffer> BufferType;
		typedef SPtr<SamplerState> SamplerType;
		typedef SPtr<GpuParamBlockBuffer> ParamsBufferType;
	};

	template<> struct TGpuParamsTypes < true >
	{
		typedef GpuParamsCore GpuParamsType;
		typedef SPtr<TextureCore> TextureType;
		typedef SPtr<GpuBufferCore> BufferType;
		typedef SPtr<SamplerStateCore> SamplerType;
		typedef SPtr<GpuParamBlockBufferCore> ParamsBufferType;
	};

	/** Templated version of GpuParams that contains functionality for both sim and core thread versions of stored data. */
	template <bool Core>
	class BS_CORE_EXPORT TGpuParams : public GpuParamsBase
	{
	public:
		typedef typename TGpuParamsTypes<Core>::GpuParamsType GpuParamsType;
		typedef typename TGpuParamsTypes<Core>::TextureType TextureType;
		typedef typename TGpuParamsTypes<Core>::BufferType BufferType;
		typedef typename TGpuParamsTypes<Core>::SamplerType SamplerType;
		typedef typename TGpuParamsTypes<Core>::ParamsBufferType ParamsBufferType;

		TGpuParams(const SPtr<GpuParamDesc>& paramDesc, bool transposeMatrices);

		virtual ~TGpuParams();

		/**
		 * Binds a new parameter buffer to the specified slot. Any following parameter reads or writes that are referencing
		 * that buffer slot will use the new buffer.
		 *
		 * @note	
		 * This is useful if you want to share a parameter buffer among multiple GPU programs. You would only set the 
		 * values once and then share the buffer among all other GpuParams.
		 * @note
		 * It is up to the caller to guarantee the provided buffer matches parameter block descriptor for this slot.
		 */
		void setParamBlockBuffer(UINT32 slot, const ParamsBufferType& paramBlockBuffer);

		/**
		 * Replaces the parameter buffer with the specified name. Any following parameter reads or writes that are 
		 * referencing that buffer will use the new buffer.
		 *
		 * @note	
		 * This is useful if you want to share a parameter buffer among multiple GPU programs. You would only set the 
		 * values once and then share the buffer among all other GpuParams.
		 * @note
		 * It is up to the caller to guarantee the provided buffer matches parameter block descriptor for this slot.
		 */
		void setParamBlockBuffer(const String& name, const ParamsBufferType& paramBlockBuffer);

		/**
		 * Returns a handle for the parameter with the specified name. Handle may then be stored and used for quickly 
		 * setting or retrieving values to/from that parameter.
		 *
		 * Throws exception if parameter with that name and type doesn't exist.
		 *
		 * Parameter handles will be invalidated when their parent GpuParams object changes.
		 */
		template<class T> void getParam(const String& name, TGpuDataParam<T, Core>& output) const;

		/** @copydoc getParam */
		void getStructParam(const String& name, TGpuParamStruct<Core>& output) const;

		/** @copydoc getParam */
		void getTextureParam(const String& name, TGpuParamTexture<Core>& output) const;

		/** @copydoc getParam */
		void getLoadStoreTextureParam(const String& name, TGpuParamLoadStoreTexture<Core>& output) const;

		/** @copydoc getParam */
		void getBufferParam(const String& name, TGpuParamBuffer<Core>& output) const;

		/** @copydoc getParam */
		void getSamplerStateParam(const String& name, TGpuParamSampState<Core>& output) const;

		/**	Gets a parameter block buffer from the specified slot. */
		ParamsBufferType getParamBlockBuffer(UINT32 slot) const;

		/**	Gets a texture bound to the specified slot. */
		TextureType getTexture(UINT32 slot);

		/**	Gets a load/store texture bound to the specified slot. */
		TextureType getLoadStoreTexture(UINT32 slot);

		/**	Gets a buffer bound to the specified slot. */
		BufferType getBuffer(UINT32 slot);

		/**	Gets a sampler state bound to the specified slot. */
		SamplerType getSamplerState(UINT32 slot);

		/**	Sets a texture at the specified slot. */
		void setTexture(UINT32 slot, const TextureType& texture);

		/**	Sets a load/store texture at the specified slot. */
		void setLoadStoreTexture(UINT32 slot, const TextureType& texture, const TextureSurface& surface);

		/**	Sets a buffer at the specified slot. */
		void setBuffer(UINT32 slot, const BufferType& buffer);

		/**	Sets a sampler state at the specified slot. */
		void setSamplerState(UINT32 slot, const SamplerType& sampler);

	protected:
		/** @copydoc CoreObject::getThisPtr */
		virtual SPtr<GpuParamsType> _getThisPtr() const = 0;

		ParamsBufferType* mParamBlockBuffers;
		TextureType* mTextures;
		TextureType* mLoadStoreTextures;
		BufferType* mBuffers;
		SamplerType* mSamplerStates;
	};

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of GpuParams.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuParamsCore : public CoreObjectCore, public TGpuParams<true>
	{
	public:
		~GpuParamsCore() { }

		/** @copydoc GpuParamsBase::GpuParamsBase */
		static SPtr<GpuParamsCore> create(const SPtr<GpuParamDesc>& paramDesc, bool transposeMatrices);

	protected:
		friend class GpuParams;

		/** @copydoc GpuParamsBase::GpuParamsBase */
		GpuParamsCore(const SPtr<GpuParamDesc>& paramDesc, bool transposeMatrices);

		/** @copydoc CoreObject::getThisPtr */
		SPtr<GpuParamsCore> _getThisPtr() const override;

		/** @copydoc CoreObjectCore::syncToCore */
		void syncToCore(const CoreSyncData& data) override;
	};

	/** @} */
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Contains descriptions for all parameters in a GPU program and also allows you to write and read those parameters. 
	 * All parameter values are stored internally on the CPU, and are only submitted to the GPU once the parameters are 
	 * bound to the pipeline.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuParams : public CoreObject, public TGpuParams<false>, public IResourceListener
	{
	public:
		~GpuParams() { }

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<GpuParamsCore> getCore() const;

		/** @copydoc GpuParamsBase::GpuParamsBase */
		static SPtr<GpuParams> create(const SPtr<GpuParamDesc>& paramDesc, bool transposeMatrices);

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
		/** @copydoc GpuParamsBase::GpuParamsBase */
		GpuParams(const SPtr<GpuParamDesc>& paramDesc, bool transposeMatrices);

		/** @copydoc CoreObject::getThisPtr */
		SPtr<GpuParams> _getThisPtr() const override;

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

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
}