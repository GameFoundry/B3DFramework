//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Material/BsMaterial.h"
#include "Renderer/BsRendererMaterialManager.h"
#include "Material/BsShaderVariation.h"
#include "Material/BsShader.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsRenderAPI.h"

#if BS_PROFILING_ENABLED
#	include "Profiling/BsProfilerGPU.h"
#endif

/** @addtogroup Renderer-Engine-Internal
 *  @{
 */

/** References the shader path in RendererMaterial implementation. */
#define RMAT_DEF(path)                                                           \
public:                                                                          \
	static void InitMetaDataInternal()                                           \
	{                                                                            \
		bs::RendererMaterialManager::RegisterMaterialInternal(&mMetaData, path); \
	};

/**
 * References the shader path in RendererMaterial implementation. Provides an InitDefinesInternal() method allowing the C++
 * code to provide preprocessor defines to be set when compiling the shader. Note that when changing these defines you need
 * to manually force the shader to be reimported.
 */
#define RMAT_DEF_CUSTOMIZED(path)                                                \
public:                                                                          \
	static void InitMetaDataInternal()                                           \
	{                                                                            \
		InitDefinesInternal(mMetaData.Defines);                                  \
		bs::RendererMaterialManager::RegisterMaterialInternal(&mMetaData, path); \
	};                                                                           \
	static void InitDefinesInternal(ShaderDefines& defines);

/** @} */

namespace bs
{
	namespace ct
	{
		/** @addtogroup Renderer-Engine-Internal
		 *  @{
		 */

		/**	Contains data common to all render material instances of a specific type. */
		struct RendererMaterialMetaData
		{
			Path ShaderPath;
			SPtr<ct::Shader> Shader;
			SPtr<ct::Shader> OverrideShader;
			SmallVector<RendererMaterialBase*, 4> Instances;
			ShaderVariations Variations;
			ShaderDefines Defines;

#if BS_PROFILING_ENABLED
			ProfilerString ProfilerSampleName;
#endif
		};

		/**
		 * Helper class that performs GPU profiling in the current block. Profiling sample is started when the class is
		 * constructed and ended upon destruction.
		 */
		struct RendererMaterialProfileBlock : ProfileGPUBlock
		{
			RendererMaterialProfileBlock(const RendererMaterialMetaData& metaData)
				: ProfileGPUBlock(metaData.ProfilerSampleName)
			{}
		};

#define BS_RENMAT_PROFILE_BLOCK RendererMaterialProfileBlock __sampleBlock(mMetaData);

		/**	Base class for all RendererMaterial instances, containing common data and methods. */
		class BS_EXPORT RendererMaterialBase
		{
		public:
			virtual ~RendererMaterialBase() = default;

			/** Returns the shader used by the material. */
			SPtr<Shader> GetShader() const { return mShader; }

			/** Returns the internal parameter set containing GPU bindable parameters. */
			SPtr<GpuParams> GetParams() const { return mParams; }

			/**
			 * Helper field to be set before construction. Identifiers the variation of the material to initialize this
			 * object with.
			 */
			u32 VarIdx;

		protected:
			friend class RendererMaterialManager;

			SPtr<GpuParams> mParams;
			SPtr<GraphicsPipelineState> mGfxPipeline;
			SPtr<ComputePipelineState> mComputePipeline;
			u32 mStencilRef = 0;

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
				T::InitMetaDataInternal();
			}

			/**	Forces the compiler to not optimize out construction of this type. */
			void Instantiate() {}
		};

		/** Wrapper class around Material that allows a simple way to load and set up materials used by the renderer. */
		template <class T>
		class RendererMaterial : public RendererMaterialBase
		{
		public:
			virtual ~RendererMaterial() = default;

			/**
			 * Retrieves an instance of this renderer material. If material has multiple variations the first available
			 * variation will be returned.
			 */
			static T* Get()
			{
				if(mMetaData.Instances[0] == nullptr)
				{
					RendererMaterialBase* mat = bs_alloc<T>();
					mat->VarIdx = 0;
					new(mat) T();

					mMetaData.Instances[0] = mat;
				}

				return (T*)mMetaData.Instances[0];
			}

			/** Retrieves an instance of a particular variation of this renderer material. */
			static T* Get(const ShaderVariation& variation)
			{
				if(variation.GetIdx() == (u32)-1)
					variation.SetIdx(mMetaData.Variations.Find(variation));

				u32 varIdx = variation.GetIdx();
				if(mMetaData.Instances[varIdx] == nullptr)
				{
					RendererMaterialBase* mat = bs_alloc<T>();
					mat->VarIdx = varIdx;
					new(mat) T();

					mMetaData.Instances[varIdx] = mat;
				}

				return (T*)mMetaData.Instances[varIdx];
			}

			/**
			 * Sets a shader that is to be used instead of the default shader for this material. Set to null to revert back
			 * to using the default shader. All existing instances of the material will be invalidated (get() methods need to
			 * be called again).
			 */
			static void SetOverride(const SPtr<Shader>& shader)
			{
				if(mMetaData.OverrideShader == shader)
					return;

				for(u32 i = 0; i < mMetaData.Instances.Size(); i++)
				{
					if(mMetaData.Instances[i] != nullptr)
						bs_delete(mMetaData.Instances[i]);

					mMetaData.Instances[i] = nullptr;
				}

				mMetaData.OverrideShader = shader;
			}

			/** Returns the path to the built-in (non-overriden) shader used by this material. */
			static Path GetShaderPath() { return mMetaData.ShaderPath; }

			/** Returns a set of dynamically defined defines used when compiling this shader. */
			static ShaderDefines GetShaderDefines() { return mMetaData.Defines; }

			/**
			 * Binds the materials and its parameters to the pipeline. This material will be used for rendering any subsequent
			 * draw calls, or executing dispatch calls. If @p bindParams is false you need to call bindParams() separately
			 * to bind material parameters (if any).
			 */
			void Bind(bool bindParams = true) const
			{
				RenderAPI& rapi = RenderAPI::Instance();

				if(mGfxPipeline)
				{
					rapi.SetGraphicsPipeline(mGfxPipeline);
					rapi.SetStencilRef(mStencilRef);
				}
				else
					rapi.SetComputePipeline(mComputePipeline);

				if(bindParams)
					rapi.SetGpuParams(mParams);
			}

			/** Binds the material parameters to the pipeline. */
			void BindParams() const
			{
				RenderAPI& rapi = RenderAPI::Instance();
				rapi.SetGpuParams(mParams);
			}

		protected:
			RendererMaterial()
			{
				mInitOnStart.Instantiate();

				if(mMetaData.OverrideShader)
					mShader = mMetaData.OverrideShader;
				else
					mShader = mMetaData.Shader;

				mVariation = mMetaData.Variations.Get(VarIdx);

				const Vector<SPtr<Technique>>& techniques = mShader->GetTechniques();
				for(auto& entry : techniques)
				{
					if(!entry->IsSupported())
						continue;

					if(entry->GetVariation() == mVariation)
					{
						SPtr<Pass> pass = entry->GetPass(0);
						pass->Compile();

						mGfxPipeline = pass->GetGraphicsPipelineState();
						if(mGfxPipeline != nullptr)
							mParams = GpuParams::Create(mGfxPipeline);
						else
						{
							mComputePipeline = pass->GetComputePipelineState();
							mParams = GpuParams::Create(mComputePipeline);
						}

						// Assign default values from the shader
						const auto& textureParams = mShader->GetTextureParams();
						for(auto& param : textureParams)
						{
							u32 defaultValueIdx = param.second.DefaultValueIdx;
							if(defaultValueIdx == (u32)-1)
								continue;

							for(u32 i = 0; i < 6; i++)
							{
								GpuProgramType progType = (GpuProgramType)i;

								for(auto& varName : param.second.GpuVariableNames)
								{
									if(mParams->HasTexture(progType, varName))
									{
										SPtr<Texture> texture = mShader->GetDefaultTexture(defaultValueIdx);
										mParams->SetTexture(progType, varName, texture);
									}
								}
							}
						}

						const auto& samplerParams = mShader->GetSamplerParams();
						for(auto& param : samplerParams)
						{
							u32 defaultValueIdx = param.second.DefaultValueIdx;
							if(defaultValueIdx == (u32)-1)
								continue;

							for(u32 i = 0; i < 6; i++)
							{
								GpuProgramType progType = (GpuProgramType)i;

								for(auto& varName : param.second.GpuVariableNames)
								{
									if(mParams->HasSamplerState(progType, varName))
									{
										SPtr<SamplerState> samplerState = mShader->GetDefaultSampler(defaultValueIdx);
										mParams->SetSamplerState(progType, varName, samplerState);
									}
								}
							}
						}

						mStencilRef = pass->GetStencilRefValue();
					}
				}
			}

			friend class RendererMaterialManager;

			static RendererMaterialMetaData mMetaData;
			static InitRendererMaterialStart<T> mInitOnStart;
		};

		template <class T>
		InitRendererMaterialStart<T> RendererMaterial<T>::mInitOnStart;

		template <class T>
		RendererMaterialMetaData RendererMaterial<T>::mMetaData;

		/** @} */
	} // namespace ct
} // namespace bs
