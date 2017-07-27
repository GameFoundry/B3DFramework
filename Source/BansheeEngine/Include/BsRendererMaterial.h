//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsMaterial.h"
#include "BsRendererMaterialManager.h"
#include "BsShaderDefines.h"

/** @addtogroup Renderer-Engine-Internal
 *  @{
 */

/** References the shader path in RendererMaterial implementation. */
#define RMAT_DEF(path)														\
	public:																	\
	static void _initMetaData()												\
	{																		\
		_initVariations(mMetaData.variations);								\
		RendererMaterialManager::_registerMaterial(&mMetaData, path);		\
	};																		\
	static void _initVariations(ShaderVariations& variations);

/** @} */

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/** 
	 * Contains information about a single variation of a RendererMaterial. Each variation can have a separate set of
	 * \#defines that control shader compilation.
	 */
	class BS_EXPORT ShaderVariation
	{
	public:
		/** Possible types of a variation parameter. */
		enum ParamType
		{
			Int,
			UInt,
			Float,
			Bool
		};

		/** Name, type and value of a variation parameter. */
		struct Param
		{
			Param()
				:i(0), type(Int)
			{ }

			Param(const String& name, INT32 val)
				:i(val), name(name), type(Int)
			{ }

			Param(const String& name, UINT32 val)
				:ui(val), name(name), type(Int)
			{ }

			Param(const String& name, float val)
				:f(val), name(name), type(Float)
			{ }

			Param(const String& name, bool val)
				:i(val ? 1 : 0), name(name), type(Bool)
			{ }

			union
			{
				INT32 i;
				UINT32 ui;
				float f;
			};

			String name;
			ParamType type;
		};

		ShaderVariation() { }
		
		/** Creates a new shader variation with the specified parameters. */
		ShaderVariation(const SmallVector<Param, 4>& params);

		/** 
		 * Returns the value of a signed integer parameter with the specified name. Returns 0 if the parameter cannot be 
		 * found.
		 */
		INT32 getInt(const String& name);

		/** 
		 * Returns the value of a unsigned integer parameter with the specified name. Returns 0 if the parameter cannot be
		 * found.
		 */
		UINT32 getUInt(const String& name);

		/** Returns the value of a float parameter with the specified name. Returns 0 if the parameter cannot be found.  */
		float getFloat(const String& name);

		/** 
		 * Returns the value of a boolean parameter with the specified name. Returns false if the parameter cannot be 
		 * found.
		 */
		bool getBool(const String& name);

		/** Converts all the variation parameters in a ShaderDefines object, that may be consumed by the shader compiler. */
		ShaderDefines getDefines() const;

		/** 
		 * Returns a unique index of this variation, relative to all other variations registered in ShaderVariations object.
		 */
		UINT32 getIdx() const { return mIdx;  }

		/** Returns all the variation parameters. */
		const UnorderedMap<String, Param>& getParams() const { return mParams; }

	private:
		friend class ShaderVariations;

		UnorderedMap<String, Param> mParams;
		UINT32 mIdx = -1;
	};

	/** A container for all variations of a single RendererMaterial. */
	class BS_EXPORT ShaderVariations
	{
	public:
		/** Registers a new variation. */
		void add(ShaderVariation& variation);

		/** Returns a variation at the specified index. Variations are indexed sequentially as they are added. */
		const ShaderVariation& get(UINT32 idx) { return mVariations[idx]; }

		/** Returns a list of all variations. */
		const SmallVector<ShaderVariation, 4>& getVariations() const { return mVariations; }

	private:
		SmallVector<ShaderVariation, 4> mVariations;
		UINT32 mNextIdx = 0;
	};

	/**	Contains data common to all render material instances of a specific type. */
	struct RendererMaterialMetaData
	{
		SmallVector<SPtr<Shader>, 4> shaders;
		SmallVector<RendererMaterialBase*, 4> instances;
		ShaderVariations variations;
	};

	/**	Base class for all RendererMaterial instances, containing common data and methods. */
	class BS_EXPORT RendererMaterialBase
	{
	public:
		RendererMaterialBase() { }
		virtual ~RendererMaterialBase() { }

		/**	Returns the internal material. */
		SPtr<Material> getMaterial() const { return mMaterial; }

		/** Returns the internal parameter set containing GPU bindable parameters. */
		SPtr<GpuParamsSet> getParamsSet() const { return mParamsSet; }

		/** 
		 * Helper field to be set before construction. Identifiers the variation of the material to initialize this 
		 * object with. 
		 */
		UINT32 _varIdx;
	protected:
		friend class RendererMaterialManager;

		SPtr<Material> mMaterial;
		SPtr<GpuParamsSet> mParamsSet;
		ShaderVariation mVariation;
	};

	/**	Helper class to initialize all renderer materials as soon as the library is loaded. */
	template <class T>
	struct InitRendererMaterialStart
	{
	public:
		InitRendererMaterialStart()
		{
			T::_initMetaData();
		}

		/**	Forces the compiler to not optimize out construction of this type. */
		void instantiate() { }
	};

	/** Wrapper class around Material that allows a simple way to load and set up materials used by the renderer. */
	template<class T>
	class RendererMaterial : public RendererMaterialBase
	{
	public:
		virtual ~RendererMaterial() { }

		/** 
		 * Retrieves an instance of this renderer material. If material has multiple variations the first available
		 * variation will be returned. 
		 */
		static T* get()
		{
			if(mMetaData.instances[0] == nullptr)
			{
				RendererMaterialBase* mat = bs_alloc<T>();
				mat->_varIdx = 0;
				new (mat) T();
				
				mMetaData.instances[0] = mat;
			}

			return (T*)mMetaData.instances[0];
		}

		/** Retrieves an instance of a particular variation of this renderer material. */
		static T* get(const ShaderVariation& variation)
		{
			UINT32 varIdx = variation.getIdx();

			if(mMetaData.instances[varIdx] == nullptr)
			{
				RendererMaterialBase* mat = bs_alloc<T>();
				mat->_varIdx = varIdx;
				new (mat) T();
				
				mMetaData.instances[varIdx] = mat;
			}

			return (T*)mMetaData.instances[varIdx];
		}

	protected:
		RendererMaterial()
		{
			mInitOnStart.instantiate();
			mMaterial = Material::create(mMetaData.shaders[_varIdx]);
			mParamsSet = mMaterial->createParamsSet();

			if(!mMetaData.variations.getVariations().empty())
				mVariation = mMetaData.variations.get(_varIdx);
		}

		friend class RendererMaterialManager;

		static RendererMaterialMetaData mMetaData;
		static InitRendererMaterialStart<T> mInitOnStart;
	};

	template<class T>
	InitRendererMaterialStart<T> RendererMaterial<T>::mInitOnStart;

	template<class T>
	RendererMaterialMetaData RendererMaterial<T>::mMetaData;

	/** @} */
}}