//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DRenderThread.h"
#include "Material/B3DMaterial.h"
#include "Renderer/B3DRendererMaterialManager.h"
#include "Renderer/B3DRenderer.h"
#include "Material/B3DShaderVariation.h"
#include "Material/B3DShader.h"
#include "Material/B3DPass.h"
#include "Material/B3DShaderCompiler.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "Resources/B3DBuiltinResources.h"

#if B3D_PROFILING_ENABLED
#	include "Profiling/B3DProfilerGPU.h"
#endif

B3D_EXPORT B3D_LOG_CATEGORY_EXTERN(LogRendererMaterial, Log)

/** @addtogroup Renderer-Internal
 *  @{
 */

/** References the shader path in RendererMaterial implementation. */
#define RMAT_DEF(path)                                                           \
public:                                                                          \
	static void InitMetaDataInternal()                                           \
	{                                                                            \
		b3d::RendererMaterialManager::RegisterMaterial(&GetMetaData(), path);	 \
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
		InitDefinesInternal(GetMetaData().Defines);                              \
		b3d::RendererMaterialManager::RegisterMaterial(&GetMetaData(), path);	 \
	};                                                                           \
	static void InitDefinesInternal(ShaderDefines& defines);

/** @} */

namespace b3d
{
	namespace render
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Compilation state for RendererMaterial shader. */
		enum class RendererMaterialShaderState
		{
			NotInitialized, /**< Shader has not been initialized and is not in the process of being initialized. */
			InitializeInProgressOnWorkerThread, /**< Initialization in progress on the worker thread. */
			Initialized /**< Shader has finished initialization. */
		};

		/** Compilation state for RendererMaterial variation. */
		enum class RendererMaterialVariationState
		{
			NotCompiled, /**< Variation has not been compiled and is not in progress. */
			CompilationWaitingOnShader, /**< Variation compilation is waiting for the parent Shader to be initialized first. */
			CompilationInProgressOnWorkerThread, /**< Compilation is in progress on the worker thread. */
			Compiled /**< Variation has been compiled. */
		};

		/* Contains information about a single renderer material variation. */
		struct RendererMaterialVariationInformation
		{
			RendererMaterialBase* RendererMaterialInstance = nullptr; /**< Instance of the material to be created once the shader variation has been created. */
			TShared<Variation> ShaderVariation; /**< Shader variation used by the material. */
			RendererMaterialVariationState State = RendererMaterialVariationState::NotCompiled;

			TAsyncOp<RendererMaterialBase*> VariationCompileOperation{ AsyncOpEmpty() }; /**< Operation tracking variation creation as a whole. */
			TAsyncOp<bool> BackendCompileOperation{ AsyncOpEmpty() }; /**< Operation tracking ShaderVariation::Compile() progress on the render backend. */
		};

		/**	Contains data common to all render material variations for a specific render material. */
		struct RendererMaterialMetaData
		{
			Path ShaderPath;
			TShared<Shader> Shader;
			std::atomic<RendererMaterialShaderState> ShaderState = RendererMaterialShaderState::NotInitialized;
			TAsyncOp<TShared<render::Shader>> ShaderInitializeOperation{ AsyncOpEmpty() };

			ShaderVariations VariationParameterSet;
			ShaderDefines Defines;

			TInlineArray<RendererMaterialVariationInformation, 4> VariationInformation;

#if B3D_PROFILING_ENABLED
			ProfilerString ProfilerSampleName;
#endif
		};

		/**
		 * Helper class that performs GPU profiling in the current block. Profiling sample is started when the class is
		 * constructed and ended upon destruction.
		 */
		struct RendererMaterialProfileBlock : ProfileGPUBlock
		{
			RendererMaterialProfileBlock(GpuCommandBuffer& commandBuffer,  const RendererMaterialMetaData& metaData)
				: ProfileGPUBlock(commandBuffer, metaData.ProfilerSampleName)
			{}
		};

#define B3D_PROFILE_RENDERER_MATERIAL RendererMaterialProfileBlock __sampleBlock(commandBuffer, GetMetaData());

		/**	Base class for all RendererMaterial instances, containing common data and methods. */
		class B3D_EXPORT RendererMaterialBase
		{
		public:
			virtual ~RendererMaterialBase() = default;

			/** Initializes the material. Use this instead of the constructor to perform any one-time setup before using the material. */
			virtual void Initialize() {}

			/** Returns the shader used by the material. */
			TShared<Shader> GetShader() const { return mShader; }

			/** Returns the internal parameter set containing GPU bindable parameters. */
			TShared<GpuParameterSet> GetGpuParameterSet() const { return mGpuParameterSet; }

			/** Creates a new instance of GPU parameters for this material. */
			virtual TShared<GpuParameterSet> CreateGpuParameterSet(u32 set = 0) const = 0;

			/** Returns the material's graphics pipeline state. This will be null if the material is a compute material. */
			TShared<GpuGraphicsPipelineState> GetGraphicsPipeline() const { return mGraphicsPipeline; }

			/** Returns the material's compute pipeline state. This will be null if the material is a graphics material. */
			TShared<GpuComputePipelineState> GetComputePipeline() const { return mComputePipeline; }

			/**
			 * Binds the materials and its parameters to the pipeline. This material will be used for rendering any subsequent
			 * draw calls, or executing dispatch calls. If @p bindParameters is false you need to call BindParameters() separately
			 * to bind material parameters (if any).
			 */
			void Bind(GpuCommandBuffer& commandBuffer, bool bindParameters = true) const;

			/** Binds the material parameters to the pipeline. */
			void BindParameters(GpuCommandBuffer& commandBuffer) const;

		protected:
			friend class b3d::RendererMaterialManager;

			TShared<GpuParameterSet> mGpuParameterSet;
			TShared<GpuGraphicsPipelineState> mGraphicsPipeline;
			TShared<GpuComputePipelineState> mComputePipeline;
			u32 mStencilReferenceValue = 0;

			ShaderVariationParameters mVariationParameters;
			TShared<Shader> mShader;
			TShared<GpuDevice> mGpuDevice;
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

		/** @} */

		/** @addtogroup Renderer
		 *  @{
		 */

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
			static T* Get();

			/** Retrieves an instance of a particular variation of this renderer material. */
			static T* Get(const ShaderVariationParameters& variationParameters);

			/** Returns the path to the built-in (non-overriden) shader used by this material. */
			static Path GetShaderPath() { return GetMetaData().ShaderPath; }

			/** Returns a set of dynamically defined defines used when compiling this shader. */
			static ShaderDefines GetShaderDefines() { return GetMetaData().Defines; }

			/** Creates a new instance of GPU parameters for this material. */
			TShared<GpuParameterSet> CreateGpuParameterSet(u32 set = 0) const override;

		protected:
			RendererMaterial();

			/** Initializes the renderer material. To be called right after construction. */
			void InitializeInternal(u32 variationIndex);

			/** Checks if the Shader object has been created. Not variations of the material can be compiled until the shader is created first. */
			static bool IsShaderInitialized();

			/** Checks if a particular variation has been compiled and initialized. */
			static bool IsRendereMaterialVariationCompiled(u32 variationIndex);

			/** Initializes the Shader object. This needs to be done before attempting to compile any instances for the material. */
			static TAsyncOp<TShared<Shader>> InitializeShader();

			/** Compiles and initializes a particular variation of the renderer material. */
			static TAsyncOp<RendererMaterialBase*> CompileRendererMaterialVariation(const ShaderVariationParameters& variationParameters);

			/** Compiles and initializes a particular variation of the renderer material. */
			static TAsyncOp<RendererMaterialBase*> CompileRendererMaterialVariation(u32 variationIndex);

			/** Compiles a particular shader variation. */
			static void CompileShaderVariation(u32 variationIndex);

			friend class b3d::RendererMaterialManager;

			/** Returns the metadata shared by all instances of this renderer material. */
			static RendererMaterialMetaData& GetMetaData()
			{
				static RendererMaterialMetaData metaData;
				return metaData;
			}

			static InitRendererMaterialStart<T> mInitOnStart;
		};

		template <class T>
		T* RendererMaterial<T>::Get()
		{
			if(!IsShaderInitialized())
			{
				TAsyncOp<TShared<Shader>> operation = InitializeShader();
				operation.BlockUntilComplete();
			}

			if(!IsRendereMaterialVariationCompiled(0))
			{
				TAsyncOp<RendererMaterialBase*> operation = CompileRendererMaterialVariation(0);
				operation.BlockUntilComplete();
			}

			return (T*)GetMetaData().VariationInformation[0].RendererMaterialInstance;
		}

		template <class T>
		T* RendererMaterial<T>::Get(const ShaderVariationParameters& variationParameters)
		{
			if(!IsShaderInitialized())
			{
				TAsyncOp<TShared<Shader>> operation = InitializeShader();
				operation.BlockUntilComplete();
			}

			if(variationParameters.GetIndex() == ~0u)
			{
				variationParameters.SetIndex(GetMetaData().VariationParameterSet.Find(variationParameters));
			}

			const u32 variationIndex = variationParameters.GetIndex();

			if(!IsRendereMaterialVariationCompiled(variationIndex))
			{
				TAsyncOp<RendererMaterialBase*> operation = CompileRendererMaterialVariation(variationParameters);
				operation.BlockUntilComplete();
			}

			return (T*)GetMetaData().VariationInformation[variationIndex].RendererMaterialInstance;
		}

		template <class T>
		TAsyncOp<TShared<Shader>> RendererMaterial<T>::InitializeShader()
		{
			B3D_ASSERT(!IsShaderInitialized());

			if(GetMetaData().ShaderState == RendererMaterialShaderState::InitializeInProgressOnWorkerThread)
				return GetMetaData().ShaderInitializeOperation;

			GetMetaData().ShaderState = RendererMaterialShaderState::InitializeInProgressOnWorkerThread;

			// Finishes shader initialization on render thread (i.e. this thread).
			auto fnFinishInitializeShader = []()
			{
				if (!B3D_ENSURE(GetMetaData().Shader))
				{
					B3D_LOG(Error, LogRendererMaterial, "Cannot initialize renderer material. Failed to compile shader {0}.", GetMetaData().ShaderPath);
					return;
				}

				const Vector<TShared<Variation>> variations = GetMetaData().Shader->GetCompatibleVariations();

				static TInlineArray<RendererMaterialVariationInformation, 4> newVariationInformation;
				static ShaderVariations newVariationParameterSet;

				B3D_ASSERT(newVariationInformation.Empty());
				B3D_ASSERT(newVariationParameterSet.IsEmpty());

				newVariationInformation.resize((u32)variations.size());
				newVariationParameterSet = ShaderVariations();

				for(u32 variationIndex = 0; variationIndex < (u32)variations.size(); ++variationIndex)
				{
					const TShared<Variation>& shaderVariation = variations[variationIndex];

					for(u32 preliminaryVariationIndex = 0; preliminaryVariationIndex < GetMetaData().VariationInformation.size(); ++preliminaryVariationIndex)
					{
						const ShaderVariationParameters& preliminaryVariationParameters = GetMetaData().VariationParameterSet.Get(preliminaryVariationIndex);
						RendererMaterialVariationInformation& preliminaryVariationInformation = GetMetaData().VariationInformation[preliminaryVariationIndex];

						B3D_ASSERT(preliminaryVariationInformation.State == RendererMaterialVariationState::CompilationWaitingOnShader);

						if(preliminaryVariationParameters == shaderVariation->GetVariationParameters())
						{
							newVariationInformation[variationIndex].State = RendererMaterialVariationState::CompilationWaitingOnShader;
							newVariationInformation[variationIndex].VariationCompileOperation = preliminaryVariationInformation.VariationCompileOperation;
							break;
						}
					}

					newVariationInformation[variationIndex].ShaderVariation = shaderVariation;
					newVariationParameterSet.Add(shaderVariation->GetVariationParameters());
				}

				std::swap(newVariationInformation, GetMetaData().VariationInformation);
				std::swap(newVariationParameterSet, GetMetaData().VariationParameterSet);

				newVariationInformation.Clear();
				newVariationParameterSet.Clear();

				GetMetaData().ShaderState = RendererMaterialShaderState::Initialized;
				GetMetaData().ShaderInitializeOperation.CompleteOperation(GetMetaData().Shader);
				GetMetaData().ShaderInitializeOperation = TAsyncOp<TShared<Shader>>(AsyncOpEmpty());

				u32 variationIndex = 0;
				for(const auto& entry : GetMetaData().VariationInformation)
				{
					if(entry.State == RendererMaterialVariationState::CompilationWaitingOnShader)
						CompileShaderVariation(variationIndex);

					variationIndex++;
				}
			};

			TAsyncOp<void> initializeAsyncOp;
			initializeAsyncOp.DoWhenComplete(std::move(fnFinishInitializeShader));

			// Performs shader initialization asynchronously on a worker thread.
			auto fnInitializeShader = [initializeAsyncOp]() mutable
			{
				static const String kRendererMaterialShaderCachePrefix = "RendererMaterialShaders/";

				GetMetaData().Shader = ShaderCompilers::Instance().GetOrCompileShader<true>(GetMetaData().ShaderPath, kRendererMaterialShaderCachePrefix, GetMetaData().Defines);
				initializeAsyncOp.CompleteOperation();
			};

			GetMetaData().ShaderInitializeOperation = TAsyncOp<TShared<Shader>>();
			GetApplication().GetTaskScheduler().Post(SchedulerTask(std::move(fnInitializeShader), "Compile shader meta-data"));

			return GetMetaData().ShaderInitializeOperation;
		}

		template <class T>
		TAsyncOp<RendererMaterialBase*> RendererMaterial<T>::CompileRendererMaterialVariation(const ShaderVariationParameters& variationParameters)
		{
			if(!IsShaderInitialized())
			{
				B3D_ASSERT(GetMetaData().ShaderState == RendererMaterialShaderState::InitializeInProgressOnWorkerThread);

				// Until the shader is ready we use the parameter set only for variations that are queued for compilation
				const u32 foundVariationIndex = GetMetaData().VariationParameterSet.Find(variationParameters);
				if(foundVariationIndex == ~0u)
				{
					RendererMaterialVariationInformation variationInformation;
					variationInformation.State = RendererMaterialVariationState::CompilationWaitingOnShader;
					variationInformation.VariationCompileOperation = TAsyncOp<RendererMaterialBase*>();

					GetMetaData().VariationInformation.Add(variationInformation);
					GetMetaData().VariationParameterSet.Add(variationParameters);

					return variationInformation.VariationCompileOperation;
				}
				else
				{
					B3D_ASSERT(foundVariationIndex >= GetMetaData().VariationInformation.Size());
					B3D_ASSERT(GetMetaData().VariationInformation[foundVariationIndex].State == RendererMaterialVariationState::CompilationWaitingOnShader);

					return GetMetaData().VariationInformation[foundVariationIndex].VariationCompileOperation;
				}
			}

			if(variationParameters.GetIndex() == ~0u)
				variationParameters.SetIndex(GetMetaData().VariationParameterSet.Find(variationParameters));

			const u32 variationIndex = variationParameters.GetIndex();
			if(!B3D_ENSURE(variationIndex != ~0u))
			{
				B3D_LOG(Error, LogRendererMaterial, "Cannot compile renderer material variation for {0}. Variation parameters cannot be found.", GetMetaData().ShaderPath);
			}

			return CompileRendererMaterialVariation(variationIndex);
		}

		template <class T>
		TAsyncOp<RendererMaterialBase*> RendererMaterial<T>::CompileRendererMaterialVariation(u32 variationIndex)
		{
			B3D_ASSERT(IsShaderInitialized());
			B3D_ASSERT(variationIndex != ~0u);
			B3D_ASSERT(!IsRendereMaterialVariationCompiled(variationIndex));

			RendererMaterialVariationInformation& variationInformation = GetMetaData().VariationInformation[variationIndex];
			if(variationInformation.State == RendererMaterialVariationState::CompilationInProgressOnWorkerThread || variationInformation.State == RendererMaterialVariationState::CompilationWaitingOnShader)
				return variationInformation.VariationCompileOperation;

			variationInformation.VariationCompileOperation = TAsyncOp<RendererMaterialBase*>();
			CompileShaderVariation(variationIndex);

			return variationInformation.VariationCompileOperation;
		}

		template<class T>
		void RendererMaterial<T>::CompileShaderVariation(u32 variationIndex)
		{
			B3D_ASSERT(GetMetaData().ShaderState == RendererMaterialShaderState::Initialized && GetMetaData().Shader != nullptr);

			RendererMaterialVariationInformation& variationInformation = GetMetaData().VariationInformation[variationIndex];
			B3D_ASSERT(variationInformation.State == RendererMaterialVariationState::NotCompiled || variationInformation.State == RendererMaterialVariationState::CompilationWaitingOnShader);

			variationInformation.State = RendererMaterialVariationState::CompilationInProgressOnWorkerThread;
			variationInformation.BackendCompileOperation = variationInformation.ShaderVariation->Compile();
			variationInformation.BackendCompileOperation.DoWhenComplete([variationIndex]
			{
				B3D_ASSERT(GetMetaData().VariationInformation[variationIndex].State == RendererMaterialVariationState::CompilationInProgressOnWorkerThread);

				RendererMaterialVariationInformation& variationInformation = GetMetaData().VariationInformation[variationIndex];
				B3D_ASSERT(variationInformation.State == RendererMaterialVariationState::CompilationInProgressOnWorkerThread);
				B3D_ASSERT(variationInformation.RendererMaterialInstance == nullptr);

				RendererMaterial* const rendererMaterialInstance = new(B3DAllocate<T>()) T();
				rendererMaterialInstance->InitializeInternal(variationIndex);

				variationInformation.RendererMaterialInstance = rendererMaterialInstance;

				variationInformation.BackendCompileOperation = TAsyncOp<bool>(AsyncOpEmpty());
				variationInformation.State = RendererMaterialVariationState::Compiled;

				variationInformation.VariationCompileOperation.CompleteOperation(variationInformation.RendererMaterialInstance);
			});
		}
	
		template <class T>
		bool RendererMaterial<T>::IsShaderInitialized()
		{
			return GetMetaData().ShaderState == RendererMaterialShaderState::Initialized;
		}

		template <class T>
		bool RendererMaterial<T>::IsRendereMaterialVariationCompiled(u32 variationIndex)
		{
			if(variationIndex == ~0u)
				return false;

			if(GetMetaData().VariationInformation.size() <= variationIndex)
				return false;

			return GetMetaData().VariationInformation[variationIndex].State == RendererMaterialVariationState::Compiled && GetMetaData().VariationInformation[variationIndex].RendererMaterialInstance != nullptr;
		}

		template<class T>
		RendererMaterial<T>::RendererMaterial()
		{
			mInitOnStart.Instantiate();
		}

		template<class T>
		void RendererMaterial<T>::InitializeInternal(u32 variationIndex)
		{
			mGpuDevice = GetApplication().GetPrimaryGpuDevice();
			mShader = GetMetaData().Shader;
			mVariationParameters = GetMetaData().VariationParameterSet.Get(variationIndex);

			const TShared<Variation>& shaderVariation = GetMetaData().VariationInformation[variationIndex].ShaderVariation;
			B3D_ASSERT(shaderVariation->IsSupported());
			B3D_ASSERT(shaderVariation->GetVariationParameters() == mVariationParameters);

			const TShared<Pass> pass = shaderVariation->GetPass(0);
			pass->Compile();

			mGraphicsPipeline = pass->GetGraphicsPipelineState();
			if(mGraphicsPipeline == nullptr)
				mComputePipeline = pass->GetComputePipelineState();

			mGpuParameterSet = CreateGpuParameterSet();

			// Assign default values from the shader
			const auto& textureParams = mShader->GetTextureParameters();
			for(auto& param : textureParams)
			{
				u32 defaultValueIdx = param.second.DefaultValueIndex;
				if(defaultValueIdx == (u32)-1)
					continue;

				for(auto& varName : param.second.GpuVariableNames)
				{
					if(mGpuParameterSet->HasSampledTexture(varName))
					{
						const TShared<Texture> texture = param.second.Type == GPOT_TEXTURE3D ? mShader->GetDefault3DTexture(defaultValueIdx) : mShader->GetDefault2DTexture(defaultValueIdx);
						mGpuParameterSet->SetSampledTexture(varName, texture);
					}
				}
			}

			const auto& samplerParams = mShader->GetSamplerParameters();
			for(auto& param : samplerParams)
			{
				u32 defaultValueIdx = param.second.DefaultValueIndex;
				if(defaultValueIdx == ~0u)
					continue;

				for(auto& varName : param.second.GpuVariableNames)
				{
					if(mGpuParameterSet->HasSamplerState(varName))
					{
						TShared<SamplerState> samplerState = mShader->GetDefaultSampler(defaultValueIdx);
						mGpuParameterSet->SetSamplerState(varName, samplerState);
					}
				}
			}

			mStencilReferenceValue = pass->GetStencilRefValue();

			Initialize();
		}

		template <class T>
		InitRendererMaterialStart<T> RendererMaterial<T>::mInitOnStart;

		template <class T>
		TShared<GpuParameterSet> RendererMaterial<T>::CreateGpuParameterSet(u32 set) const
		{
			GpuParameterSetPool& pool = GetRenderer()->GetParameterSetPool();

			if(mGraphicsPipeline != nullptr)
				return pool.Create(mGraphicsPipeline->GetParameterLayout()->GetSet(set), set);
			else if(mComputePipeline != nullptr)
				return pool.Create(mComputePipeline->GetParameterLayout()->GetSet(set), set);

			return nullptr;
		}

		/** @} */
	} // namespace render
} // namespace b3d
