//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"
#include "BsIResourceListener.h"
#include "BsMaterialParam.h"
#include "BsMaterialParams.h"
#include "BsTechnique.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsMatrix3.h"
#include "BsMatrix4.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	template<bool Core> struct TGpuParamBlockBufferPtrType { };
	template<> struct TGpuParamBlockBufferPtrType<false> { typedef SPtr<GpuParamBlockBuffer> Type; };
	template<> struct TGpuParamBlockBufferPtrType<true> { typedef SPtr<GpuParamBlockBufferCore> Type; };

	template<bool Core> struct TGpuProgramType { };
	template<> struct TGpuProgramType<false> { typedef SPtr<GpuProgram> Type; };
	template<> struct TGpuProgramType<true> { typedef SPtr<GpuProgramCore> Type; };

	template<bool Core> struct TShaderType {};
	template<> struct TShaderType < false > { typedef HShader Type; };
	template<> struct TShaderType < true > { typedef SPtr<ShaderCore> Type; };

	template<bool Core> struct TGpuParamBlockBufferType {};
	template<> struct TGpuParamBlockBufferType < false > { typedef GpuParamBlockBuffer Type; };
	template<> struct TGpuParamBlockBufferType < true > { typedef GpuParamBlockBufferCore Type; };
	
	template<bool Core> struct TGpuParamsSetType {};
	template<> struct TGpuParamsSetType < false > { typedef GpuParamsSet Type; };
	template<> struct TGpuParamsSetType < true > { typedef GpuParamsSetCore Type; };

	/**
	 * Material that controls how objects are rendered. It is represented by a shader and parameters used to set up that
	 * shader. It provides a simple interface for manipulating the parameters.
	 */
	class BS_CORE_EXPORT MaterialBase
	{
	public:
		/** Data used to describe a structure defined within a shader. */
		struct StructData
		{
			StructData()
				:data(nullptr), size(0)
			{ }

			StructData(UINT32 _size)
				:size(_size)
			{
				data = std::shared_ptr<void>(bs_alloc(_size), (void(*)(void*))&bs_free);
			}

			/**
			 * Writes the specified data to the internal buffer. Caller must ensure size of the provided buffer is correct.
			 */
			void write(void* _data)
			{
				memcpy(data.get(), _data, size);
			}

			SPtr<void> data;
			UINT32 size;
		};

		MaterialBase() { }
		virtual ~MaterialBase() { }

		/** @name Internal
		 *  @{
		 */

		/** Marks the contents of the sim thread object as dirty, causing it to sync with its core thread counterpart. */
		virtual void _markCoreDirty() { }

		/** @} */
	protected:
		/** @copydoc CoreObject::markDependenciesDirty */
		virtual void _markDependenciesDirty() { }

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		virtual void _markResourcesDirty() { }
	};

	/** @copydoc MaterialBase */
	template<bool Core>
	class BS_CORE_EXPORT TMaterial : public MaterialBase
	{
	public:
		typedef typename TGpuParamTextureType<Core>::Type TextureType;
		typedef typename TGpuBufferType<Core>::Type BufferType;
		typedef typename TGpuParamSamplerStateType<Core>::Type SamplerStateType;
		typedef typename TGpuProgramType<Core>::Type GpuProgramType;
		typedef typename TPassType<Core>::Type PassType;
		typedef typename TTechniqueType<Core>::Type TechniqueType;
		typedef typename TShaderType<Core>::Type ShaderType;
		typedef typename TGpuParamsSetType<Core>::Type GpuParamsSetType;
		typedef typename TMaterialParamsType<Core>::Type MaterialParamsType;

		TMaterial() { }
		virtual ~TMaterial() { }

		/** Returns the currently active shader. */
		ShaderType getShader() const { return mShader; }

		/** Returns the total number of techniques supported by this material. */
		UINT32 getNumTechniques() const { return (UINT32)mTechniques.size(); }

		/** Attempts to find a technique with the supported tag. Returns an index of the technique, or -1 if not found. */
		UINT32 findTechnique(const StringID& tag) const;

		/** Finds the index of the default (primary) technique to use. */
		UINT32 getDefaultTechnique() const;

		/** 
		 * Returns the number of passes that are used by the technique at the specified index. 
		 *
		 * @param[in]	techniqueIdx	Index of the technique to retrieve the number of passes for. 0 is always guaranteed
		 *								to be the default technique.
		 * @return						Number of passes used by the technique.
		 */
		UINT32 getNumPasses(UINT32 techniqueIdx = 0) const;

		/** 
		 * Retrieves a specific shader pass from the provided technique. 
		 *
		 * @param[in]	passIdx			Sequential index of the pass to retrieve.
		 * @param[in]	techniqueIdx	Index of the technique to retrieve the pass for. 0 is always guaranteed to be
		 *								the default technique.
		 * @return						Pass if found, null otherwise.
		 */
		SPtr<PassType> getPass(UINT32 passIdx = 0, UINT32 techniqueIdx = 0) const;

		/**
		 * Creates a set of GpuParams that may be used for binding material parameters to the GPU. The expected behaviour
		 * is to create a set of GpuParams per-technique once, and then before binding them to the GPU call
		 * updateParamsSet() to ensure any dirty parameters are transfered from the material to GpuParams. You may also
		 * use the parameter set to manually modify parameters on a per-program basis, in which case no further updates from
		 * the material are necessary.
		 */
		SPtr<GpuParamsSetType> createParamsSet(UINT32 techniqueIdx = 0);

		/**
		 * Updates the provided parameter set by recording in it any changes that were made since the last call. It is
		 * assumed only a single parameter set exists per-material per-technique. If there are multiple the material
		 * will not be able to track which parameters were updated since the last call.
		 *
		 * @param[in]	paramsSet		Parameter set to update.
		 * @param[in]	dirtyBitIdx		Index to use when checking if parameters are dirty. Must be in range [0, 30]. Allows
		 *								the same material params to record dirty state for multiple sets of GPU params
		 *								(each with their own index). Dirty state for the specified index will be cleared
		 *								after the call.
		 * @param[in]	forceRefresh	If true all material parameters will be assigned to the params set, regardless if
		 *								they are marked dirty or not.
		 */
		void updateParamsSet(const SPtr<GpuParamsSetType>& paramsSet, UINT32 dirtyBitIdx = 0, bool forceRefresh = false);

		/**   
		 * Assigns a float value to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setFloat(const String& name, float value, UINT32 arrayIdx = 0)	{ return getParamFloat(name).set(value, arrayIdx); }

		/**   
		 * Assigns a color to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setColor(const String& name, const Color& value, UINT32 arrayIdx = 0) { return getParamColor(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 2D vector to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setVec2(const String& name, const Vector2& value, UINT32 arrayIdx = 0)	{ return getParamVec2(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 3D vector to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setVec3(const String& name, const Vector3& value, UINT32 arrayIdx = 0)	{ return getParamVec3(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 4D vector to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setVec4(const String& name, const Vector4& value, UINT32 arrayIdx = 0)	{ return getParamVec4(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 3x3 matrix to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setMat3(const String& name, const Matrix3& value, UINT32 arrayIdx = 0)	{ return getParamMat3(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 4x4 matrix to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setMat4(const String& name, const Matrix4& value, UINT32 arrayIdx = 0)	{ return getParamMat4(name).set(value, arrayIdx); }

		/**   
		 * Assigns a structure to the shader parameter with the specified name.
		 *  		
		 * Structure is provided as a raw buffer and caller must ensure structure in buffer matches what the shader expects.
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		void setStructData(const String& name, void* value, UINT32 size, UINT32 arrayIdx = 0) { return getParamStruct(name).set(value, size, arrayIdx); }

		/** Assigns a texture to the shader parameter with the specified name. */
		void setTexture(const String& name, const TextureType& value) { return getParamTexture(name).set(value); }

		/** Assigns a texture to be used for random load/store operations to the shader parameter with the specified name. */
		void setLoadStoreTexture(const String& name, const TextureType& value, const TextureSurface& surface)
		{ 
			return getParamLoadStoreTexture(name).set(value, surface); 
		}

		/** Assigns a buffer to the shader parameter with the specified name. */
		void setBuffer(const String& name, const BufferType& value) { return getParamBuffer(name).set(value); }

		/** Assigns a sampler state to the shader parameter with the specified name. */
		void setSamplerState(const String& name, const SamplerStateType& value) { return getParamSamplerState(name).set(value); }

		/**
		 * Returns a float value assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		float getFloat(const String& name, UINT32 arrayIdx = 0) const { return getParamFloat(name).get(arrayIdx); }

		/**
		 * Returns a color assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		Color getColor(const String& name, UINT32 arrayIdx = 0) const { return getParamColor(name).get(arrayIdx); }

		/**
		 * Returns a 2D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		Vector2 getVec2(const String& name, UINT32 arrayIdx = 0) const { return getParamVec2(name).get(arrayIdx); }

		/**
		 * Returns a 3D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		Vector3 getVec3(const String& name, UINT32 arrayIdx = 0) const { return getParamVec3(name).get(arrayIdx); }

		/**
		 * Returns a 4D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		Vector4 getVec4(const String& name, UINT32 arrayIdx = 0) const { return getParamVec4(name).get(arrayIdx); }

		/**
		 * Returns a 3x3 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		Matrix3 getMat3(const String& name, UINT32 arrayIdx = 0) const { return getParamMat3(name).get(arrayIdx); }

		/**
		 * Returns a 4x4 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		Matrix4 getMat4(const String& name, UINT32 arrayIdx = 0) const { return getParamMat4(name).get(arrayIdx); }

		/** Returns a texture assigned with the parameter with the specified name. */
		TextureType getTexture(const String& name) const { return getParamTexture(name).get(); }

		/** Returns a sampler state assigned with the parameter with the specified name. */
		SamplerStateType getSamplerState(const String& name) const	{ return getParamSamplerState(name).get(); }

		/**
		 * Returns a buffer representing a structure assigned to the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		MaterialBase::StructData getStructData(const String& name, UINT32 arrayIdx = 0) const
		{
			TMaterialParamStruct<Core> structParam = getParamStruct(name);

			MaterialBase::StructData data(structParam.getElementSize());
			structParam.get(data.data.get(), structParam.getElementSize(), arrayIdx);

			return data;
		}

		/**
		 * Returns a float GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter 
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<float, Core> getParamFloat(const String& name) const
		{
			TMaterialDataParam<float, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a color GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter 
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, 
		 * and then use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Color, Core> getParamColor(const String& name) const
		{
			TMaterialDataParam<Color, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a 2D vector GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter 
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Vector2, Core> getParamVec2(const String& name) const
		{
			TMaterialDataParam<Vector2, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a 3D vector GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Vector3, Core> getParamVec3(const String& name) const
		{
			TMaterialDataParam<Vector3, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a 4D vector GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Vector4, Core> getParamVec4(const String& name) const
		{
			TMaterialDataParam<Vector4, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a 3x3 matrix GPU parameter. This parameter may be used for more efficiently getting/setting GPU 
		 * parameter values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Matrix3, Core> getParamMat3(const String& name) const
		{
			TMaterialDataParam<Matrix3, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a 4x4 matrix GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note	
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialDataParam<Matrix4, Core> getParamMat4(const String& name) const
		{
			TMaterialDataParam<Matrix4, Core> gpuParam;
			getParam(name, gpuParam);

			return gpuParam;
		}

		/**
		 * Returns a structure GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamStruct<Core> getParamStruct(const String& name) const;

		/**
		 * Returns a texture GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter 
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamTexture<Core> getParamTexture(const String& name) const;

		/**
		 * Returns a GPU parameter for binding a load/store texture. This parameter may be used for more efficiently 
		 * getting/setting GPU parameter values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamLoadStoreTexture<Core> getParamLoadStoreTexture(const String& name) const;

		/**
		 * Returns a buffer GPU parameter. This parameter may be used for more efficiently getting/setting GPU parameter 
		 * values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamBuffer<Core> getParamBuffer(const String& name) const;

		/**
		 * Returns a sampler state GPU parameter. This parameter may be used for more efficiently getting/setting GPU 
		 * parameter values than calling Material::get* / Material::set* methods. 
		 *
		 * @note	
		 * Expected behavior is that you would retrieve this parameter when initially constructing the material, and then 
		 * use it throughout material lifetime to assign and retrieve parameter values.
		 * @note			
		 * If material shader changes this handle will be invalidated.
		 */
		TMaterialParamSampState<Core> getParamSamplerState(const String& name) const;

		/**
		 * Allows you to retrieve a handle to a parameter that you can then use for quickly setting and retrieving parameter
		 * data. This allows you to set/get parameter data without all the cost of extra lookups otherwise required.
		 * 			
		 * @note	
		 * All of these handles will be invalidated if material shader ever changes. It is up to the caller to keep track 
		 * of that.
		 */
		template <typename T>
		void getParam(const String& name, TMaterialDataParam<T, Core>& output) const;

		/**
		 * @name Internal
		 * @{
		 */

		/** 
		 * Returns an object containg all of material's parameters. Allows the caller to manipulate the parameters more
		 * directly. 
		 */
		SPtr<MaterialParamsType> _getInternalParams() const { return mParams; }

		/** @} */
	protected:
		/**
		 * Assigns a value from a raw buffer to the parameter with the specified name. Buffer must be of sizeof(T) * 
		 * numElements size and initialized.
		 *
		 * @note	Provided parameter must exist, no checking is done.
		 */
		template <typename T>
		void setParamValue(const String& name, UINT8* buffer, UINT32 numElements);

		/**
		 * Initializes the material by using the compatible techniques from the currently set shader. Shader must contain 
		 * the techniques that matches the current renderer and render system.
		 */
		void initializeTechniques();

		/** Assigns all the default parameters specified in the shader to the material. */
		void initDefaultParameters();

		/** Throw an exception if no shader is set, or no acceptable technique was found. */
		void throwIfNotInitialized() const;

		ShaderType mShader;
		SPtr<MaterialParamsType> mParams;
		Vector<SPtr<TechniqueType>> mTechniques;
	};

	/** @} */

	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class BS_CORE_EXPORT MaterialCore : public CoreObjectCore, public TMaterial<true>
	{
	public:
		~MaterialCore() { }

		/** @copydoc Material::setShader */
		void setShader(const SPtr<ShaderCore>& shader);

		/** Returns any custom data set by the renderer. */
		const Any& getRendererData() const { return mRendererData; }

		/** 
		 * Sets renderer-specific data on the material. This can by anything the renderer requires, as its not used by the
		 * material directly. 
		 */
		void setRendererData(const Any& data) { mRendererData = data; }

		/** 
		 * Returns a version number that increments whenever the material's shader changes. Allows external systems to know
		 * when they need to rebuild relevant cached data.
		 */
		UINT32 getVersion() const { return mVersion; }

		/** Creates a new material with the specified shader. */
		static SPtr<MaterialCore> create(const SPtr<ShaderCore>& shader);

	private:
		friend class Material;

		MaterialCore() { }
		MaterialCore(const SPtr<ShaderCore>& shader);
		MaterialCore(const SPtr<ShaderCore>& shader, const Vector<SPtr<TechniqueCore>>& techniques,
			const SPtr<MaterialParamsCore>& materialParams);

		/** @copydoc CoreObjectCore::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		Any mRendererData;
		UINT32 mVersion;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class BS_CORE_EXPORT Material : public Resource, public TMaterial<false>, public IResourceListener
	{
	public:
		~Material() { }

		/**
		 * Sets a shader that will be used by the material. Best technique within the provided shader will be used for the 
		 * material.
		 *
		 * @note	
		 * Shader must be set before doing any other operations with the material.
		 * @note			
		 * After setting the shader if you change the implementation of systems that a shader technique is dependent upon 
		 * (render system, renderer, etc), you will need to call this method again on all your Materials to make sure
		 * technique used is updated.
		 */
		void setShader(const HShader& shader);

		/** Retrieves an implementation of a material usable only from the core thread. */
		SPtr<MaterialCore> getCore() const;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** Creates a deep copy of the material and returns the new object. */
		HMaterial clone();

		/**
		 * Creates a new empty material.
		 * 			
		 * @note	Make sure you call Material::setShader before using it.
		 */
		static HMaterial create();

		/** Creates a new material with the specified shader. */
		static HMaterial create(const HShader& shader);

		/** @name Internal
		 *  @{
		 */

		/** @copydoc CoreObject::markCoreDirty */
		void _markCoreDirty() override;

		/** @} */
	private:
		friend class MaterialManager;

		Material();
		Material(const HShader& shader);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/** @copydoc CoreObject::markDependenciesDirty */
		void _markDependenciesDirty() override;

		/** @copydoc IResourceListener::markResourcesDirty */
		void _markResourcesDirty() override;

		/** @copydoc IResourceListener::getListenerResources */
		void getListenerResources(Vector<HResource>& resources) override;

		/** @copydoc IResourceListener::notifyResourceLoaded */
		void notifyResourceLoaded(const HResource& resource) override;

		/** @copydoc IResourceListener::notifyResourceChanged */
		void notifyResourceChanged(const HResource& resource) override;

		/** @copydoc Resource::getResourceDependencies */
		void getResourceDependencies(FrameVector<HResource>& dependencies) const override;

		/**	Performs material initialization when all resources are ready. */
		void initializeIfLoaded();

		/** 
		 * Uses the provided list of parameters to try to set every parameter in this material. Parameter whose name, type
		 * or size don't match are ignored and will not be set.
		 */
		void setParams(const SPtr<MaterialParams>& params);

		UINT32 mLoadFlags;
		
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
	public:
		friend class MaterialRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}