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
		_initDefines(mMetaData.defines);									\
		RendererMaterialManager::_registerMaterial(&mMetaData, path);		\
	};																		\
	static void _initDefines(ShaderDefines& defines);

/** @} */

namespace bs
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/**	Contains data common to all render material instances of a specific type. */
	struct RendererMaterialMetaData
	{
		SPtr<ShaderCore> shader;
		ShaderDefines defines;
	};

	/**	Base class for all RendererMaterial instances, containing common data and methods. */
	class BS_EXPORT RendererMaterialBase
	{
	public:
		virtual ~RendererMaterialBase() { }

		/**	Returns the internal material. */
		SPtr<MaterialCore> getMaterial() const { return mMaterial; }

		/** Returns the internal parameter set containing GPU bindable parameters. */
		SPtr<GpuParamsSetCore> getParamsSet() const { return mParamsSet; }

	protected:
		friend class RendererMaterialManager;

		SPtr<MaterialCore> mMaterial;
		SPtr<GpuParamsSetCore> mParamsSet;
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
		RendererMaterial()
		{
			mInitOnStart.instantiate();
			mMaterial = MaterialCore::create(mMetaData.shader);
			mParamsSet = mMaterial->createParamsSet();
		}

		virtual ~RendererMaterial() { }

	protected:
		friend class RendererMaterialManager;

		static RendererMaterialMetaData mMetaData;
		static InitRendererMaterialStart<T> mInitOnStart;
	};

	template<class T>
	InitRendererMaterialStart<T> RendererMaterial<T>::mInitOnStart;

	template<class T>
	RendererMaterialMetaData RendererMaterial<T>::mMetaData;

	/** @} */
}