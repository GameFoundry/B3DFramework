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
#include "Material/B3DShaderRegistry.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuWorkContext.h"
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

		/** Compilation state for a RendererMaterial shader. */
		enum class RendererMaterialShaderState
		{
			NotInitialized, /**< Shader has not been initialized and is not in the process of being initialized. */
			InitializeInProgress, /**< Shader initialization (CPU compile + variation enumeration) is in progress. */
			Initialized /**< Shader has finished initialization. */
		};

		/** Compilation state for a RendererMaterial variation. */
		enum class RendererMaterialVariationState
		{
			NotCompiled, /**< Variation has not been compiled and is not in progress. */
			CompilationInProgress, /**< Variation compilation is in progress (pinned to the render thread). */
			Compiled /**< Variation has been compiled and its pipeline products are published. */
		};

		/**
		 * Contains information about a single renderer material variation. The compile products below are
		 * shared by all per-context instances of this variation: they are written exactly once, on the render
		 * thread, when the variation transitions to Compiled, and only ever read afterwards.
		 */
		struct RendererMaterialVariationInformation
		{
			TShared<Variation> ShaderVariation; /**< Shader variation used by the material. */
			RendererMaterialVariationState State = RendererMaterialVariationState::NotCompiled;

			/** Tracks the in-flight compile. Completes with true on success, false on failure. */
			TAsyncOp<bool> CompileOperation{ AsyncOpEmpty() };

			TShared<GpuGraphicsPipelineState> GraphicsPipeline; /**< Shared pipeline (null for compute materials). */
			TShared<GpuComputePipelineState> ComputePipeline; /**< Shared pipeline (null for graphics materials). */
			u32 StencilReferenceValue = 0; /**< Shared stencil reference value read from the compiled pass. */
		};

		/**
		 * Contains data common to all variations and per-context instances of a specific renderer material.
		 * There is exactly one (function-local static) instance per material type. All shader/variation state
		 * and the shared compile products are guarded by StateMutex; the per-context material instances live in
		 * the owning GpuWorkContext (see RendererMaterialContextCache), not here.
		 */
		struct RendererMaterialMetaData
		{
			Path ShaderPath;
			TShared<Shader> Shader;
			RendererMaterialShaderState ShaderState = RendererMaterialShaderState::NotInitialized;
			TAsyncOp<TShared<render::Shader>> ShaderInitializeOperation{ AsyncOpEmpty() };

			ShaderVariations VariationParameterSet;
			ShaderDefines Defines;

			TInlineArray<RendererMaterialVariationInformation, 4> VariationInformation;

			/**
			 * Guards every read and transition of the shader/variation state and the shared compile products.
			 * Never held across a blocking wait, an async-op completion, or compile work - callers copy the
			 * operation to wait on out from under the lock, release it, then wait.
			 */
			Mutex StateMutex;

			/** Total variations compiled for this material across all threads. Diagnostics + exactly-once test. */
			std::atomic<u32> VariationCompileCount = 0;

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
			/** Cache prefix under which renderer-material shaders are resolved by ShaderRegistry, and the key the offline shader cook writes them under. */
			static constexpr const char* kRendererMaterialShaderCachePrefix = "RendererMaterialShaders/";

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

			/**
			 * Context that owns this instance and whose parameter set pool backs CreateGpuParameterSet(). The
			 * context outlives the instance (it destroys it), so this never dangles.
			 */
			GpuWorkContext* mOwnerContext = nullptr;
		};

		/**
		 * Per-context cache of renderer material instances for a single material type, registered as a
		 * context-local object on the GpuWorkContext that owns them (keyed by the material's metadata). Holds
		 * one instance per variation index (gaps are null), plus the in-flight GetAsync() operation for each
		 * index so that repeat GetAsync() calls for the same variation on this context return the same operation
		 * and the same instance. Accessed only on the context's owning thread/fiber - GetAsync()'s continuations
		 * are SameThread-pinned there, and the cache operations never yield mid-update, so cooperative scheduling
		 * serializes all access and no locking is needed. Destroyed with its context, on the context's owning
		 * thread, after the context's GPU work has drained - so each instance's parameter set frees immediately
		 * into the still-alive context pool.
		 */
		template <class T>
		class RendererMaterialContextCache final : public IGpuWorkContextLocal
		{
		public:
			~RendererMaterialContextCache() override
			{
				for(RendererMaterialBase* instance : Instances)
				{
					if(instance != nullptr)
						B3DDelete(instance);
				}
			}

			TInlineArray<RendererMaterialBase*, 4> Instances; /**< Material instances indexed by variation index. */

			/**
			 * In-flight GetAsync() operations indexed by variation index, reset to empty once the operation has
			 * settled. Lets repeat GetAsync() calls for a variation, on this context, return the same operation.
			 */
			TInlineArray<TAsyncOp<T*>, 4> PendingOps;
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
			 * Retrieves the render thread's instance of this material (its first variation). Render thread only -
			 * use the GpuWorkContext overloads to obtain an instance on a worker thread/fiber.
			 */
			static T* Get();

			/**
			 * Retrieves the render thread's instance of a particular variation of this material. Render thread
			 * only - use the GpuWorkContext overloads to obtain an instance on a worker thread/fiber.
			 */
			static T* Get(const ShaderVariationParameters& variationParameters);

			/**
			 * Retrieves the instance of this material (its first variation) owned by @p gpuContext, creating it on
			 * first use. May be called from any thread/fiber that owns @p gpuContext. The returned instance must
			 * only be used on the context's owning thread/fiber and must not outlive the context. Returns null if
			 * the shader/variation failed to compile.
			 */
			static T* Get(GpuWorkContext& gpuContext);

			/** Retrieves the instance of a particular variation owned by @p gpuContext. See Get(GpuWorkContext&). */
			static T* Get(GpuWorkContext& gpuContext, const ShaderVariationParameters& variationParameters);

			/**
			 * Non-blocking variant of Get(GpuWorkContext&): kicks shader/variation compilation (shared across all
			 * threads) and per-context instance creation if needed, and returns an operation that completes with
			 * the context's instance for this material's first variation - or null if the shader/variation failed
			 * to compile. Must be called from the scheduler thread/fiber that owns @p gpuContext; the
			 * returned operation must be driven to completion before the context is destroyed, and the instance it
			 * yields obeys the same usage/lifetime contract as Get(GpuWorkContext&).
			 */
			static TAsyncOp<T*> GetAsync(GpuWorkContext& gpuContext);

			/** Non-blocking variant of Get(GpuWorkContext&, const ShaderVariationParameters&). See GetAsync(GpuWorkContext&). */
			static TAsyncOp<T*> GetAsync(GpuWorkContext& gpuContext, const ShaderVariationParameters& variationParameters);

			/**
			 * Render-thread convenience overload of GetAsync(GpuWorkContext&) operating on the renderer's primary
			 * context. Render thread only; returns a completed operation yielding null if called off it.
			 */
			static TAsyncOp<T*> GetAsync();

			/** Render-thread convenience overload of GetAsync(GpuWorkContext&, ...). See GetAsync(). */
			static TAsyncOp<T*> GetAsync(const ShaderVariationParameters& variationParameters);

			/** Returns the path to the built-in (non-overriden) shader used by this material. */
			static Path GetShaderPath() { return GetMetaData().ShaderPath; }

			/** Returns a set of dynamically defined defines used when compiling this shader. */
			static ShaderDefines GetShaderDefines() { return GetMetaData().Defines; }

			/** Creates a new instance of GPU parameters for this material, allocated from the owning context's pool. */
			TShared<GpuParameterSet> CreateGpuParameterSet(u32 set = 0) const override;

		protected:
			RendererMaterial();

			/**
			 * Initializes a freshly constructed instance for @p variationIndex against @p gpuContext: copies the
			 * shared compile products, allocates the parameter set from the context's pool and assigns shader
			 * defaults. The variation must already be compiled. To be called right after construction.
			 */
			void InitializeInternal(GpuWorkContext& gpuContext, u32 variationIndex);

			friend class b3d::RendererMaterialManager;

			/** Returns the metadata shared by all instances of this renderer material. */
			static RendererMaterialMetaData& GetMetaData()
			{
				static RendererMaterialMetaData metaData;
				return metaData;
			}

		private:
			/**
			 * Returns the shared, in-flight shader-initialization operation, starting the (compile-once) shader
			 * compile if it has not begun. If the shader is already initialized a completed operation carrying the
			 * shader is returned; on failure the operation completes with null. Never blocks. Safe from any thread.
			 */
			static TAsyncOp<TShared<Shader>> InitializeShader();

			/** Render-thread finalize of shader initialization: enumerates variations and publishes the shader. */
			static void FinishShaderInitializationOnRenderThread(TShared<Shader> compiledShader);

			/**
			 * Ensures the shader is compiled and its variations enumerated, blocking (fiber-yieldably) until so.
			 * Safe to call from any thread; the first caller drives compilation, the rest wait. Returns false if
			 * compilation failed.
			 */
			static bool EnsureShaderInitialized();

			/**
			 * Returns the shared, in-flight compile operation for @p variationIndex, starting the (compile-once,
			 * render-thread-pinned) compile if it has not begun. If the variation is already compiled a completed
			 * operation with true is returned; on failure (or an invalid index) it completes with false. Never
			 * blocks. Safe from any thread. Shader must be initialized.
			 */
			static TAsyncOp<bool> CompileRendererMaterialVariation(u32 variationIndex);

			/** Render-thread compile of a single variation: compiles the pass and publishes the shared pipeline products. */
			static void CompileRendererMaterialVariationOnRenderThread(u32 variationIndex);

			/**
			 * Ensures the variation at @p variationIndex is compiled, blocking (fiber-yieldably) until so. The
			 * actual compile is pinned to the render thread. Returns false on failure. Shader must be initialized.
			 */
			static bool EnsureVariationCompiled(u32 variationIndex);

			/**
			 * Returns @p gpuContext's instance for an already-compiled variation, constructing and caching it on
			 * first use. Must run on the context's owning thread; assumes the variation's products are published.
			 */
			static T* CreateInstanceForCompiledVariation(GpuWorkContext& gpuContext, u32 variationIndex);

			/** Ensures the variation is compiled, then returns @p gpuContext's instance for it, creating it on first use. */
			static T* GetOrCreateInstance(GpuWorkContext& gpuContext, u32 variationIndex);

			/**
			 * Drives @p resultOp to completion: chains on the shared variation-compile operation and, once it
			 * succeeds, creates this context's instance (on the owning thread, via the SameThread continuation)
			 * and completes @p resultOp with it (or null on compile failure). Must run on the owning thread.
			 */
			static void CreateInstanceWhenVariationCompiles(GpuWorkContext& gpuContext, u32 variationIndex, TAsyncOp<T*> resultOp);

			/** Resolves (and caches into @p variationParameters) the variation index, or ~0u if not found. Shader must be initialized. */
			static u32 ResolveVariationIndex(const ShaderVariationParameters& variationParameters);

			/**
			 * Shared driver behind the GetAsync(GpuWorkContext&) overloads. @p resolveIndex is invoked once the
			 * shader is known initialized and returns the variation index (or ~0u). When the shader is already
			 * initialized this resolves and dispatches synchronously; otherwise it chains on the shader-init op.
			 */
			template <class ResolverType>
			static TAsyncOp<T*> GetAsyncInternal(GpuWorkContext& gpuContext, ResolverType&& resolveIndex);

			static InitRendererMaterialStart<T> mInitOnStart;
		};

		template <class T>
		T* RendererMaterial<T>::Get()
		{
			// Default Get() is render-thread only; its instances are just the renderer's primary-context instances.
			if(!EnsureRenderThread())
				return nullptr;

			return Get(GetRenderer()->GetGpuContext());
		}

		template <class T>
		T* RendererMaterial<T>::Get(const ShaderVariationParameters& variationParameters)
		{
			if(!EnsureRenderThread())
				return nullptr;

			return Get(GetRenderer()->GetGpuContext(), variationParameters);
		}

		template <class T>
		T* RendererMaterial<T>::Get(GpuWorkContext& gpuContext)
		{
			// Fast path: this context already has the first variation's instance - no shared state, no locks.
			if(IGpuWorkContextLocal* const existing = gpuContext.GetLocal(&GetMetaData()))
			{
				RendererMaterialContextCache<T>& cache = static_cast<RendererMaterialContextCache<T>&>(*existing);
				if(!cache.Instances.Empty() && cache.Instances[0] != nullptr)
					return static_cast<T*>(cache.Instances[0]);
			}

			if(!EnsureShaderInitialized())
				return nullptr;

			return GetOrCreateInstance(gpuContext, 0);
		}

		template <class T>
		T* RendererMaterial<T>::Get(GpuWorkContext& gpuContext, const ShaderVariationParameters& variationParameters)
		{
			// Fast path: this params object already resolved its index and the context has the instance cached.
			const u32 cachedIndex = variationParameters.GetIndex();
			if(cachedIndex != ~0u)
			{
				if(IGpuWorkContextLocal* const existing = gpuContext.GetLocal(&GetMetaData()))
				{
					RendererMaterialContextCache<T>& cache = static_cast<RendererMaterialContextCache<T>&>(*existing);
					if(cachedIndex < cache.Instances.size() && cache.Instances[cachedIndex] != nullptr)
						return static_cast<T*>(cache.Instances[cachedIndex]);
				}
			}

			if(!EnsureShaderInitialized())
				return nullptr;

			const u32 variationIndex = ResolveVariationIndex(variationParameters);
			if(variationIndex == ~0u)
				return nullptr;

			return GetOrCreateInstance(gpuContext, variationIndex);
		}

		template <class T>
		TAsyncOp<T*> RendererMaterial<T>::GetAsync(GpuWorkContext& gpuContext)
		{
			// Default overload targets the first variation (index 0), matching Get(GpuWorkContext&).
			return GetAsyncInternal(gpuContext, []() { return 0u; });
		}

		template <class T>
		TAsyncOp<T*> RendererMaterial<T>::GetAsync(GpuWorkContext& gpuContext, const ShaderVariationParameters& variationParameters)
		{
			const ShaderVariationParameters params = variationParameters;
			return GetAsyncInternal(gpuContext, [params]() { return ResolveVariationIndex(params); });
		}

		template <class T>
		TAsyncOp<T*> RendererMaterial<T>::GetAsync()
		{
			if(!EnsureRenderThread())
			{
				TAsyncOp<T*> op;
				op.CompleteOperation(nullptr);
				return op;
			}

			return GetAsync(GetRenderer()->GetGpuContext());
		}

		template <class T>
		TAsyncOp<T*> RendererMaterial<T>::GetAsync(const ShaderVariationParameters& variationParameters)
		{
			if(!EnsureRenderThread())
			{
				TAsyncOp<T*> op;
				op.CompleteOperation(nullptr);
				return op;
			}

			return GetAsync(GetRenderer()->GetGpuContext(), variationParameters);
		}

		template <class T>
		template <class ResolverType>
		TAsyncOp<T*> RendererMaterial<T>::GetAsyncInternal(GpuWorkContext& gpuContext, ResolverType&& resolveIndex)
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			bool shaderReady;
			{
				Lock lock(metaData.StateMutex);
				shaderReady = metaData.ShaderState == RendererMaterialShaderState::Initialized && metaData.Shader != nullptr;
			}

			// Common path: the shader is already initialized, so resolve the index now and dispatch through the
			// per-context instance cache (fully deduped by variation index), reusing the cached in-flight operation
			// or a completed one if the instance already exists, otherwise starting a fresh chain.
			if(shaderReady)
			{
				const u32 variationIndex = resolveIndex();
				if(variationIndex == ~0u)
				{
					TAsyncOp<T*> op;
					op.CompleteOperation(nullptr);
					return op;
				}

				RendererMaterialContextCache<T>& cache = gpuContext.GetOrCreateLocal<RendererMaterialContextCache<T>>(&GetMetaData(), []() { return B3DMakeUnique<RendererMaterialContextCache<T>>(); });

				// Instance already built - hand back a completed operation.
				if(variationIndex < cache.Instances.size() && cache.Instances[variationIndex] != nullptr)
				{
					TAsyncOp<T*> op;
					op.CompleteOperation(static_cast<T*>(cache.Instances[variationIndex]));
					return op;
				}

				// A request for this variation is already in flight on this context - share its operation.
				if(variationIndex < cache.PendingOps.size() && cache.PendingOps[variationIndex] != nullptr)
					return cache.PendingOps[variationIndex];

				TAsyncOp<T*> resultOp;
				if(variationIndex >= cache.PendingOps.size())
					cache.PendingOps.resize(variationIndex + 1, TAsyncOp<T*>(AsyncOpEmpty()));
				cache.PendingOps[variationIndex] = resultOp;

				CreateInstanceWhenVariationCompiles(gpuContext, variationIndex, resultOp);
				return resultOp;
			}

			// Cold path: the shader is still compiling. Chain on the shared shader-init operation; the index is
			// resolved once it is ready. There is no per-context op dedup in this narrow window, but the shared
			// compile and the instance cache still ensure a single compile and a single instance per context.
			TAsyncOp<T*> resultOp;
			GpuWorkContext* const contextPtr = &gpuContext;

			InitializeShader().DoWhenComplete(
				[contextPtr, resultOp, resolveIndex = std::forward<ResolverType>(resolveIndex)]() mutable
				{
					RendererMaterialMetaData& innerMetaData = GetMetaData();
					bool ready;
					{
						Lock lock(innerMetaData.StateMutex);
						ready = innerMetaData.ShaderState == RendererMaterialShaderState::Initialized && innerMetaData.Shader != nullptr;
					}

					if(!ready)
					{
						resultOp.CompleteOperation(nullptr);
						return;
					}

					const u32 variationIndex = resolveIndex();
					if(variationIndex == ~0u)
					{
						resultOp.CompleteOperation(nullptr);
						return;
					}

					CreateInstanceWhenVariationCompiles(*contextPtr, variationIndex, resultOp);
				});

			return resultOp;
		}

		template <class T>
		void RendererMaterial<T>::CreateInstanceWhenVariationCompiles(GpuWorkContext& gpuContext, u32 variationIndex, TAsyncOp<T*> resultOp)
		{
			GpuWorkContext* const contextPtr = &gpuContext;
			TAsyncOp<bool> compileOp = CompileRendererMaterialVariation(variationIndex);

			// The continuation runs on this thread
			compileOp.DoWhenComplete([contextPtr, variationIndex, resultOp, compileOp]() mutable
			{
				T* instance = nullptr;
				if(compileOp.GetReturnValue())
					instance = CreateInstanceForCompiledVariation(*contextPtr, variationIndex);

				// Drop the cache's reference to the now-settled in-flight operation. Once the instance exists the
				// fast paths return it directly and never consult PendingOps, so this is purely housekeeping.
				if(IGpuWorkContextLocal* const local = contextPtr->GetLocal(&GetMetaData()))
				{
					RendererMaterialContextCache<T>& cache = static_cast<RendererMaterialContextCache<T>&>(*local);
					if(variationIndex < cache.PendingOps.size())
						cache.PendingOps[variationIndex] = TAsyncOp<T*>(AsyncOpEmpty());
				}

				resultOp.CompleteOperation(instance);
			});
		}

		template <class T>
		T* RendererMaterial<T>::CreateInstanceForCompiledVariation(GpuWorkContext& gpuContext, u32 variationIndex)
		{
			RendererMaterialContextCache<T>& cache = gpuContext.GetOrCreateLocal<RendererMaterialContextCache<T>>(&GetMetaData(), []() { return B3DMakeUnique<RendererMaterialContextCache<T>>(); });

			if(variationIndex >= cache.Instances.size())
				cache.Instances.resize(variationIndex + 1, nullptr);

			if(cache.Instances[variationIndex] == nullptr)
			{
				T* const instance = new(B3DAllocate<T>()) T();
				instance->InitializeInternal(gpuContext, variationIndex);
				cache.Instances[variationIndex] = instance;
			}

			return static_cast<T*>(cache.Instances[variationIndex]);
		}

		template <class T>
		T* RendererMaterial<T>::GetOrCreateInstance(GpuWorkContext& gpuContext, u32 variationIndex)
		{
			if(!EnsureVariationCompiled(variationIndex))
				return nullptr;

			return CreateInstanceForCompiledVariation(gpuContext, variationIndex);
		}

		template <class T>
		bool RendererMaterial<T>::EnsureShaderInitialized()
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			TAsyncOp<TShared<Shader>> opToWait = InitializeShader();
			opToWait.BlockUntilComplete();

			Lock lock(metaData.StateMutex);
			return metaData.ShaderState == RendererMaterialShaderState::Initialized && metaData.Shader != nullptr;
		}

		template <class T>
		TAsyncOp<TShared<Shader>> RendererMaterial<T>::InitializeShader()
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			TAsyncOp<TShared<Shader>> op{ AsyncOpEmpty() };
			bool shouldStartCompile = false;
			{
				Lock lock(metaData.StateMutex);

				switch(metaData.ShaderState)
				{
				case RendererMaterialShaderState::Initialized:
				case RendererMaterialShaderState::InitializeInProgress:
					op = metaData.ShaderInitializeOperation;
					break;
				case RendererMaterialShaderState::NotInitialized:
					metaData.ShaderState = RendererMaterialShaderState::InitializeInProgress;
					metaData.ShaderInitializeOperation = TAsyncOp<TShared<Shader>>();
					op = metaData.ShaderInitializeOperation;
					shouldStartCompile = true;
					break;
				}
			}

			// Kick the compile outside the lock so the work and any wait stay off the StateMutex. The CPU shader
			// compile runs on a task-scheduler worker; the result is finalized on the render thread (variation
			// enumeration + pass compile must run there). The finalize is posted as a scheduler task so it runs
			// even while the render thread is blocked waiting on this operation.
			if(shouldStartCompile)
			{
				auto fnCompileShader = []()
				{
					RendererMaterialMetaData& metaData = GetMetaData();
					TShared<Shader> compiledShader = ShaderRegistry::Instance().GetOrCompileShader<true>(
						metaData.ShaderPath, RendererMaterialBase::kRendererMaterialShaderCachePrefix, metaData.Defines);

					GetRenderThread().PostTask(SchedulerTask([compiledShader]() mutable
					{
						FinishShaderInitializationOnRenderThread(std::move(compiledShader));
					}, "RendererMaterial shader finalize"));
				};

				GetApplication().GetTaskScheduler().Post(SchedulerTask(std::move(fnCompileShader), "Compile shader meta-data"));
			}

			return op;
		}

		template <class T>
		void RendererMaterial<T>::FinishShaderInitializationOnRenderThread(TShared<Shader> compiledShader)
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			TAsyncOp<TShared<Shader>> completedOp{ AsyncOpEmpty() };

			if(compiledShader != nullptr)
			{
				// Enumerate variations outside the lock (touches only the freshly compiled shader).
				const Vector<TShared<Variation>> variations = compiledShader->GetCompatibleVariations();

				TInlineArray<RendererMaterialVariationInformation, 4> newVariationInformation;
				ShaderVariations newVariationParameterSet;

				newVariationInformation.resize((u32)variations.size());
				for(u32 i = 0; i < (u32)variations.size(); ++i)
				{
					newVariationInformation[i].ShaderVariation = variations[i];
					newVariationParameterSet.Add(variations[i]->GetVariationParameters());
				}

				{
					Lock lock(metaData.StateMutex);
					metaData.Shader = compiledShader;
					metaData.VariationInformation = std::move(newVariationInformation);
					metaData.VariationParameterSet = std::move(newVariationParameterSet);
					metaData.ShaderState = RendererMaterialShaderState::Initialized;

					// Retain the operation in completed form so later Initialized-state callers reuse it; take a
					// copy (it shares the same heap data) to complete outside the lock.
					completedOp = metaData.ShaderInitializeOperation;
				}
			}
			else
			{
				B3D_LOG(Error, LogRendererMaterial, "Cannot initialize renderer material. Failed to compile shader {0}.", metaData.ShaderPath);

				Lock lock(metaData.StateMutex);
				metaData.ShaderState = RendererMaterialShaderState::NotInitialized;

				completedOp = std::move(metaData.ShaderInitializeOperation);
				metaData.ShaderInitializeOperation = TAsyncOp<TShared<Shader>>(AsyncOpEmpty());
			}

			// Complete outside the lock so waiters resume without contending on StateMutex.
			if(completedOp != nullptr)
				completedOp.CompleteOperation(compiledShader);
		}

		template <class T>
		u32 RendererMaterial<T>::ResolveVariationIndex(const ShaderVariationParameters& variationParameters)
		{
			RendererMaterialMetaData& metaData = GetMetaData();
			Lock lock(metaData.StateMutex);

			u32 variationIndex = variationParameters.GetIndex();
			if(variationIndex == ~0u || variationIndex >= metaData.VariationInformation.size())
			{
				variationIndex = metaData.VariationParameterSet.Find(variationParameters);
				variationParameters.SetIndex(variationIndex);
			}

			if(!B3D_ENSURE(variationIndex != ~0u && variationIndex < metaData.VariationInformation.size()))
			{
				B3D_LOG(Error, LogRendererMaterial, "Cannot find renderer material variation for {0}.", metaData.ShaderPath);
				return ~0u;
			}

			return variationIndex;
		}

		template <class T>
		TAsyncOp<bool> RendererMaterial<T>::CompileRendererMaterialVariation(u32 variationIndex)
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			TAsyncOp<bool> op{ AsyncOpEmpty() };
			bool invalidIndex = false;
			bool shouldCompile = false;
			{
				Lock lock(metaData.StateMutex);

				if(variationIndex >= metaData.VariationInformation.size())
				{
					invalidIndex = true;
				}
				else
				{
					RendererMaterialVariationInformation& variationInformation = metaData.VariationInformation[variationIndex];
					switch(variationInformation.State)
					{
					case RendererMaterialVariationState::Compiled:
					case RendererMaterialVariationState::CompilationInProgress:
						op = variationInformation.CompileOperation;
						break;
					case RendererMaterialVariationState::NotCompiled:
						variationInformation.State = RendererMaterialVariationState::CompilationInProgress;
						variationInformation.CompileOperation = TAsyncOp<bool>();
						op = variationInformation.CompileOperation;
						shouldCompile = true;
						break;
					}
				}
			}

			// An out-of-range index has no stored operation to reuse; report it and hand back a completed failure.
			if(invalidIndex)
			{
				B3D_ENSURE(false && "RendererMaterial variation index out of range.");
				TAsyncOp<bool> completed;
				completed.CompleteOperation(false);
				return completed;
			}

			if(shouldCompile)
			{
				// The compile (ShaderVariation/Pass compile) is not thread safe, so pin it to the render thread.
				if(B3D_CURRENT_THREAD_ID == RenderThread::Instance().GetThreadId())
					CompileRendererMaterialVariationOnRenderThread(variationIndex);
				else
					GetRenderThread().PostTask(SchedulerTask([variationIndex]() { CompileRendererMaterialVariationOnRenderThread(variationIndex); }, "RendererMaterial variation compile"));
			}

			return op;
		}

		template <class T>
		void RendererMaterial<T>::CompileRendererMaterialVariationOnRenderThread(u32 variationIndex)
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			// Read the variation handle under the lock; the compile itself runs without the lock held.
			TShared<Variation> shaderVariation;
			{
				Lock lock(metaData.StateMutex);
				B3D_ASSERT(metaData.ShaderState == RendererMaterialShaderState::Initialized);
				B3D_ASSERT(variationIndex < metaData.VariationInformation.size());
				B3D_ASSERT(metaData.VariationInformation[variationIndex].State == RendererMaterialVariationState::CompilationInProgress);

				shaderVariation = metaData.VariationInformation[variationIndex].ShaderVariation;
			}

			bool success = false;
			TShared<GpuGraphicsPipelineState> graphicsPipeline;
			TShared<GpuComputePipelineState> computePipeline;
			u32 stencilReferenceValue = 0;

			if(B3D_ENSURE(shaderVariation != nullptr) && shaderVariation->IsSupported())
			{
				// Variation::Compile() is currently synchronous (see B3DVariation.cpp) and compiles the passes
				// inline, so this wait returns immediately - it does not block the render thread's command pump.
				TAsyncOp<bool> backendCompileOperation = shaderVariation->Compile();
				backendCompileOperation.BlockUntilComplete();

				if(backendCompileOperation.GetReturnValue() && shaderVariation->GetPassCount() > 0)
				{
					const TShared<Pass> pass = shaderVariation->GetPass(0);
					pass->Compile(); // idempotent; Compile() above already compiled the passes

					graphicsPipeline = pass->GetGraphicsPipelineState();
					if(graphicsPipeline == nullptr)
						computePipeline = pass->GetComputePipelineState();

					stencilReferenceValue = pass->GetStencilRefValue();
					success = graphicsPipeline != nullptr || computePipeline != nullptr;
				}
			}

			if(!success)
				B3D_LOG(Error, LogRendererMaterial, "Failed to compile renderer material variation {0} for {1}.", variationIndex, metaData.ShaderPath);

			TAsyncOp<bool> completedOp{ AsyncOpEmpty() };
			{
				Lock lock(metaData.StateMutex);
				RendererMaterialVariationInformation& variationInformation = metaData.VariationInformation[variationIndex];

				if(success)
				{
					variationInformation.GraphicsPipeline = graphicsPipeline;
					variationInformation.ComputePipeline = computePipeline;
					variationInformation.StencilReferenceValue = stencilReferenceValue;
					variationInformation.State = RendererMaterialVariationState::Compiled;
					metaData.VariationCompileCount.fetch_add(1, std::memory_order_relaxed);

					completedOp = variationInformation.CompileOperation;
				}
				else
				{
					variationInformation.State = RendererMaterialVariationState::NotCompiled;
					completedOp = std::move(variationInformation.CompileOperation);
					variationInformation.CompileOperation = TAsyncOp<bool>(AsyncOpEmpty());
				}
			}

			// Complete outside the lock so waiters resume without contending on StateMutex.
			if(completedOp != nullptr)
				completedOp.CompleteOperation(success);
		}

		template <class T>
		bool RendererMaterial<T>::EnsureVariationCompiled(u32 variationIndex)
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			TAsyncOp<bool> opToWait = CompileRendererMaterialVariation(variationIndex);
			opToWait.BlockUntilComplete();

			Lock lock(metaData.StateMutex);
			return variationIndex < metaData.VariationInformation.size() && metaData.VariationInformation[variationIndex].State == RendererMaterialVariationState::Compiled;
		}

		template<class T>
		RendererMaterial<T>::RendererMaterial()
		{
			mInitOnStart.Instantiate();
		}

		template<class T>
		void RendererMaterial<T>::InitializeInternal(GpuWorkContext& gpuContext, u32 variationIndex)
		{
			RendererMaterialMetaData& metaData = GetMetaData();

			mOwnerContext = &gpuContext;
			mGpuDevice = GetApplication().GetPrimaryGpuDevice();

			// Copy the shared, immutable-after-compile state for this variation. No pass compile here - the
			// pipeline products were already produced once, on the render thread, by CompileShaderVariation.
			{
				Lock lock(metaData.StateMutex);
				B3D_ASSERT(variationIndex < metaData.VariationInformation.size());
				const RendererMaterialVariationInformation& variationInformation = metaData.VariationInformation[variationIndex];
				B3D_ASSERT(variationInformation.State == RendererMaterialVariationState::Compiled);

				mShader = metaData.Shader;
				mVariationParameters = metaData.VariationParameterSet.Get(variationIndex);
				mGraphicsPipeline = variationInformation.GraphicsPipeline;
				mComputePipeline = variationInformation.ComputePipeline;
				mStencilReferenceValue = variationInformation.StencilReferenceValue;
			}

			B3D_ASSERT(mGraphicsPipeline != nullptr || mComputePipeline != nullptr);

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

			Initialize();
		}

		template <class T>
		InitRendererMaterialStart<T> RendererMaterial<T>::mInitOnStart;

		template <class T>
		TShared<GpuParameterSet> RendererMaterial<T>::CreateGpuParameterSet(u32 set) const
		{
			B3D_ASSERT(mOwnerContext != nullptr);
			GpuParameterSetPool& pool = mOwnerContext->GetParameterSetPool();

			if(mGraphicsPipeline != nullptr)
				return pool.Create(mGraphicsPipeline->GetParameterLayout()->GetSet(set), set);
			else if(mComputePipeline != nullptr)
				return pool.Create(mComputePipeline->GetParameterLayout()->GetSet(set), set);

			return nullptr;
		}

		/** @} */
	} // namespace render
} // namespace b3d
