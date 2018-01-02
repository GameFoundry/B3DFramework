//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Resources/BsIResourceListener.h"
#include "Material/BsMaterialParam.h"
#include "Material/BsMaterialParams.h"
#include "Material/BsTechnique.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsMatrix3.h"
#include "Math/BsMatrix4.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	template<bool Core> struct TGpuParamBlockBufferPtrType { };
	template<> struct TGpuParamBlockBufferPtrType<false> { typedef SPtr<GpuParamBlockBuffer> Type; };
	template<> struct TGpuParamBlockBufferPtrType<true> { typedef SPtr<ct::GpuParamBlockBuffer> Type; };

	template<bool Core> struct TGpuProgramType { };
	template<> struct TGpuProgramType<false> { typedef SPtr<GpuProgram> Type; };
	template<> struct TGpuProgramType<true> { typedef SPtr<ct::GpuProgram> Type; };

	template<bool Core> struct TShaderType {};
	template<> struct TShaderType < false > { typedef HShader Type; };
	template<> struct TShaderType < true > { typedef SPtr<ct::Shader> Type; };

	template<bool Core> struct TGpuParamBlockBufferType {};
	template<> struct TGpuParamBlockBufferType < false > { typedef GpuParamBlockBuffer Type; };
	template<> struct TGpuParamBlockBufferType < true > { typedef ct::GpuParamBlockBuffer Type; };
	
	template<bool Core> struct TGpuParamsSetType {};
	template<> struct TGpuParamsSetType < false > { typedef GpuParamsSet Type; };
	template<> struct TGpuParamsSetType < true > { typedef ct::GpuParamsSet Type; };

	/** Flags that signal in what way did the Material change. */
	enum class MaterialDirtyFlags
	{
		Normal				= 1 << 0,
		ResourceChanged		= 2 << 1
	};

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
		virtual void _markCoreDirty(MaterialDirtyFlags flags = MaterialDirtyFlags::Normal) { }

		/** @copydoc CoreObject::markDependenciesDirty */
		virtual void _markDependenciesDirty() { }

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		virtual void _markResourcesDirty() { }

		/** @} */
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
		BS_SCRIPT_EXPORT(n:Shader,pr:getter)
		ShaderType getShader() const { return mShader; }

		/** Returns the total number of techniques supported by this material. */
		UINT32 getNumTechniques() const { return (UINT32)mTechniques.size(); }

		/** Attempts to find a technique with the supported tag. Returns an index of the technique, or -1 if not found. */
		UINT32 findTechnique(const StringID& tag) const;

		/** Attempts to find a technique matching the provided variation. Returns an index of the technique, or -1 if not found. */
		UINT32 findTechnique(const ShaderVariation& variation) const;

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
		 * Copies internal material parameter data to the provided params set.
		 *
		 * @param[in]	paramsSet		Parameter set to update.
		 * @param[in]	updateAll		Normally the system will track dirty parameters since the last call to this method
		 *								(on a per-set basis), and only update the dirty ones. Set this to true if you want
		 *								to force all parameters to update, regardless of their dirty state.
		 */
		void updateParamsSet(const SPtr<GpuParamsSetType>& paramsSet, bool updateAll = false);

		/**   
		 * Assigns a float value to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetFloat)
		void setFloat(const String& name, float value, UINT32 arrayIdx = 0)	{ return getParamFloat(name).set(value, arrayIdx); }

		/**   
		 * Assigns a color to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetColor)
		void setColor(const String& name, const Color& value, UINT32 arrayIdx = 0) { return getParamColor(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 2D vector to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetVector2)
		void setVec2(const String& name, const Vector2& value, UINT32 arrayIdx = 0)	{ return getParamVec2(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 3D vector to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetVector3)
		void setVec3(const String& name, const Vector3& value, UINT32 arrayIdx = 0)	{ return getParamVec3(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 4D vector to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetVector4)
		void setVec4(const String& name, const Vector4& value, UINT32 arrayIdx = 0)	{ return getParamVec4(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 3x3 matrix to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetMatrix3)
		void setMat3(const String& name, const Matrix3& value, UINT32 arrayIdx = 0)	{ return getParamMat3(name).set(value, arrayIdx); }

		/**   
		 * Assigns a 4x4 matrix to the shader parameter with the specified name. 
		 *
		 * Optionally if the parameter is an array you may provide an array index to assign the value to.
		 */
		BS_SCRIPT_EXPORT(n:SetMatrix4)
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
		void setTexture(const String& name, const TextureType& value, const TextureSurface& surface = TextureSurface::COMPLETE)
		{
			return getParamTexture(name).set(value, surface);
		}

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
		BS_SCRIPT_EXPORT(n:GetFloat)
		float getFloat(const String& name, UINT32 arrayIdx = 0) const { return getParamFloat(name).get(arrayIdx); }

		/**
		 * Returns a color assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetColor)
		Color getColor(const String& name, UINT32 arrayIdx = 0) const { return getParamColor(name).get(arrayIdx); }

		/**
		 * Returns a 2D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetVector2)
		Vector2 getVec2(const String& name, UINT32 arrayIdx = 0) const { return getParamVec2(name).get(arrayIdx); }

		/**
		 * Returns a 3D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetVector3)
		Vector3 getVec3(const String& name, UINT32 arrayIdx = 0) const { return getParamVec3(name).get(arrayIdx); }

		/**
		 * Returns a 4D vector assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetVector4)
		Vector4 getVec4(const String& name, UINT32 arrayIdx = 0) const { return getParamVec4(name).get(arrayIdx); }

		/**
		 * Returns a 3x3 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetMatrix3)
		Matrix3 getMat3(const String& name, UINT32 arrayIdx = 0) const { return getParamMat3(name).get(arrayIdx); }

		/**
		 * Returns a 4x4 matrix assigned with the parameter with the specified name.
		 *
		 * Optionally if the parameter is an array you may provide an array index you which to retrieve.
		 */
		BS_SCRIPT_EXPORT(n:GetMatrix4)
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
		 * the techniques that matches the current renderer and render system. Only the techniques matching the provided
		 * variation will be used, unless @p allVariations is set to true, in which case @p variation parameter is ignored.
		 */
		void initializeTechniques(bool allVariations = true, const ShaderVariation& variation = ShaderVariation());

		/** Assigns all the default parameters specified in the shader to the material. */
		void initDefaultParameters();

		/** Throw an exception if no shader is set, or no acceptable technique was found. */
		void throwIfNotInitialized() const;

		ShaderType mShader;
		SPtr<MaterialParamsType> mParams;
		Vector<SPtr<TechniqueType>> mTechniques;
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) Material: public Resource, public TMaterial<false>, public IResourceListener
	{
	public:
		~Material() { }

		/**
		 * Sets a shader that will be used by the material. Material will be initialized using all compatible techniques
		 * from the shader. Shader must be set before doing any other operations with the material.
		 */
		BS_SCRIPT_EXPORT(n:Shader,pr:setter)
		void setShader(const HShader& shader);

		/** Retrieves an implementation of a material usable only from the core thread. */
		SPtr<ct::Material> getCore() const;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** Creates a deep copy of the material and returns the new object. */
		BS_SCRIPT_EXPORT(n:Clone)
		HMaterial clone();

		/**
		 * Creates a new empty material.
		 * 			
		 * @note	Make sure you call Material::setShader before using it.
		 */
		BS_SCRIPT_EXPORT(ec:Material)
		static HMaterial create();

		/** Creates a new material with the specified shader. */
		BS_SCRIPT_EXPORT(ec:Material)
		static HMaterial create(const HShader& shader);

		/** @name Internal
		 *  @{
		 */

		/** 
		 * Marks the core data as dirty. This causes the syncToCore() method to trigger the next time objects are synced 
		 * between core and sim threads. 
		 */
		void _markCoreDirty(MaterialDirtyFlags flags = MaterialDirtyFlags::Normal) override;

		/** @copydoc CoreObject::markDependenciesDirty */
		void _markDependenciesDirty() override;

		/** @copydoc IResourceListener::markListenerResourcesDirty */
		void _markResourcesDirty() override;

		/** @} */
	private:
		friend class MaterialManager;

		Material();
		Material(const HShader& shader);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

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

	namespace ct
	{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/** @copydoc MaterialBase */
	class BS_CORE_EXPORT Material : public CoreObject, public TMaterial<true>
	{
	public:
		~Material() { }

		/** @copydoc bs::Material::setShader */
		void setShader(const SPtr<Shader>& shader);

		/**
		 * Sets a shader that will be used by the material. Material will be initialized using a subset of compatible 
		 * techniques matching the provided variation. Shader must be set before doing any other operations with the
		 * material.
		 */
		void setShader(const SPtr<Shader>& shader, const ShaderVariation& variation);

		/** 
		 * Creates a new material with the specified shader. If the shader has multiple variations all of them are
		 * initialized and can then be retrieved through findTechnique(const ShaderVariation&). 
		 */
		static SPtr<Material> create(const SPtr<Shader>& shader);

		/** Creates a new material with the specified shader, using only the provided shader variation. */
		static SPtr<Material> create(const SPtr<Shader>& shader, const ShaderVariation& variation);
	private:
		friend class bs::Material;

		Material() { }
		Material(const SPtr<Shader>& shader);
		Material(const SPtr<Shader>& shader, const ShaderVariation& variation);
		Material(const SPtr<Shader>& shader, const Vector<SPtr<Technique>>& techniques,
			const SPtr<MaterialParams>& materialParams);

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;
	};

	/** @} */	
	}
}