//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "Material/BsMaterial.h"
#include "Renderer/BsRendererMaterialManager.h"
#include "Material/BsShaderVariation.h"
#include "Material/BsShader.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsRenderAPI.h"

/** @addtogroup Renderer-Engine-Internal
 *  @{
 */

/** References the shader path in RendererMaterial implementation. */
#define RMAT_DEF(path)														\
	public:																	\
	static void _initMetaData()												\
	{																		\
		bs::RendererMaterialManager::_registerMaterial(&mMetaData, path);	\
	};																		\

/** 
 * References the shader path in RendererMaterial implementation. Provides an _initDefines() method allowing the C++
 * code to provide preprocessor defines to be set when compiling the shader. Note that when changing these defines you need
 * to manually force the shader to be reimported.
 */
#define RMAT_DEF_CUSTOMIZED(path)											\
	public:																	\
	static void _initMetaData()												\
	{																		\
		_initDefines(mMetaData.defines);									\
		bs::RendererMaterialManager::_registerMaterial(&mMetaData, path);	\
	};																		\
	static void _initDefines(ShaderDefines& defines);

/** @} */

namespace bs { namespace ct
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/**	Contains data common to all render material instances of a specific type. */
	struct RendererMaterialMetaData
	{
		SPtr<Shader> shader;
		SmallVector<RendererMaterialBase*, 4> instances;
		ShaderVariations variations;
		ShaderDefines defines;
	};

	/**	Base class for all RendererMaterial instances, containing common data and methods. */
	class BS_EXPORT RendererMaterialBase
	{
	public:
		RendererMaterialBase() { }
		virtual ~RendererMaterialBase() { }

		/** Returns the shader used by the material. */
		SPtr<Shader> getShader() const { return mShader; }

		/** Returns the internal parameter set containing GPU bindable parameters. */
		SPtr<GpuParams> getParams() const { return mParams; }

		/** 
		 * Helper field to be set before construction. Identifiers the variation of the material to initialize this 
		 * object with. 
		 */
		UINT32 _varIdx;
	protected:
		friend class RendererMaterialManager;

		SPtr<GpuParams> mParams;
		SPtr<GraphicsPipelineState> mGfxPipeline;
		SPtr<ComputePipelineState> mComputePipeline;
		UINT32 mStencilRef = 0;

		ShaderVariation mVariation;
		SPtr<Shader> mShader;
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
			if(variation.getIdx() == (UINT32)-1)
				variation.setIdx(mMetaData.variations.find(variation));

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

		/** 
		 * Binds the materials and its parameters to the pipeline. This material will be used for rendering any subsequent
		 * draw calls, or executing dispatch calls.
		 */
		void bind() const
		{
			RenderAPI& rapi = RenderAPI::instance();

			if(mGfxPipeline)
			{
				rapi.setGraphicsPipeline(mGfxPipeline);
				rapi.setStencilRef(mStencilRef);
			}
			else
				rapi.setComputePipeline(mComputePipeline);

			rapi.setGpuParams(mParams);
		}

	protected:
		RendererMaterial()
		{
			mInitOnStart.instantiate();
			mShader = mMetaData.shader;
			mVariation = mMetaData.variations.get(_varIdx);

			const Vector<SPtr<Technique>>& techniques = mMetaData.shader->getTechniques();
			for(auto& entry : techniques)
			{
				if(!entry->isSupported())
					continue;

				if(entry->getVariation() == mVariation)
				{
					SPtr<Pass> pass = entry->getPass(0);

					mGfxPipeline = pass->getGraphicsPipelineState();
					if (mGfxPipeline != nullptr)
						mParams = GpuParams::create(mGfxPipeline);
					else
					{
						mComputePipeline = pass->getComputePipelineState();
						mParams = GpuParams::create(mComputePipeline);
					}

					// Assign default values from the shader
					const auto& textureParams = mMetaData.shader->getTextureParams();
					for(auto& param : textureParams)
					{
						UINT32 defaultValueIdx = param.second.defaultValueIdx;
						if(defaultValueIdx == (UINT32)-1)
							continue;

						for (UINT32 i = 0; i < 6; i++)
						{
							GpuProgramType progType = (GpuProgramType)i;

							for(auto& varName : param.second.gpuVariableNames)
							{
								if(mParams->hasTexture(progType, varName))
								{
									SPtr<Texture> texture = mMetaData.shader->getDefaultTexture(defaultValueIdx);
									mParams->setTexture(progType, varName, texture);
								}
							}
						}
					}

					const auto& samplerParams = mMetaData.shader->getSamplerParams();
					for(auto& param : samplerParams)
					{
						UINT32 defaultValueIdx = param.second.defaultValueIdx;
						if(defaultValueIdx == (UINT32)-1)
							continue;

						for (UINT32 i = 0; i < 6; i++)
						{
							GpuProgramType progType = (GpuProgramType)i;

							for(auto& varName : param.second.gpuVariableNames)
							{
								if(mParams->hasSamplerState(progType, varName))
								{
									SPtr<SamplerState> samplerState = mMetaData.shader->getDefaultSampler(defaultValueIdx);
									mParams->setSamplerState(progType, varName, samplerState);
								}
							}
						}
					}

					mStencilRef = pass->getStencilRefValue();
				}
			}
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