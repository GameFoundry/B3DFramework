//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DTypeList.h"

namespace b3d
{
	class GpuCommandBufferProfiler;
}

namespace b3d
{
	class RendererExtension;

	namespace render
	{
		class RenderBeastScene;
		class RendererViewGroup;
		class RenderCompositorNode;
		struct PooledStorageBuffer;
		struct FrameInfo;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Inputs provided to each node in the render compositor hierarchy */
		struct RenderCompositorNodeInputs
		{
			RenderCompositorNodeInputs(RenderBeastScene& scene, const RendererViewGroup& viewGroup, const RendererView& view, const RenderBeastOptions& options, const FrameInfo& frameInfo, RenderBeastFeatureSet featureSet)
				: Scene(scene), ViewGroup(viewGroup), View(view), Options(options), FrameInfo(frameInfo), FeatureSet(featureSet)
			{}

			SPtr<GpuCommandBuffer> ActiveCommandBuffer;
#if B3D_PROFILING_ENABLED
			SPtr<GpuCommandBufferProfiler> CommandBufferProfiler;
#endif

			RenderBeastScene& Scene;
			const RendererViewGroup& ViewGroup;
			const RendererView& View;
			const RenderBeastOptions& Options;
			const FrameInfo& FrameInfo;
			const RenderBeastFeatureSet FeatureSet;

			// Callbacks to external systems can hook into the compositor
			TInlineArray<RendererExtension*, 4> ExtPrepare;
			TInlineArray<RendererExtension*, 4> ExtPreBasePass;
			TInlineArray<RendererExtension*, 4> ExtPostBasePass;
			TInlineArray<RendererExtension*, 4> ExtPostLighting;
			TInlineArray<RendererExtension*, 4> ExtOverlay;

			TInlineArray<RenderCompositorNode*, 4> InputNodes;
		};

		/** Forward declaration */
		template<typename... NodeTypes>
		struct RCNodeDependencies;

		/** Defines render compositor node dependencies. Each node can be conditionally enabled or disabled via a predicate. */
		template<typename... NodeTypes>
		struct RCNodeDependencyDefinition
		{
			using TypeList = TTypeList<NodeTypes...>;
			using ConditionalEnablePredicate = std::function<bool(const RendererView&)>;

			RCNodeDependencyDefinition()
			{
				mConditionalEnablePredicates.fill(nullptr);
			}

			/** Sets a predicate that allows nodes to be conditionally enabled or disabled. Returns *this for chaining. */
			template<typename T>
			RCNodeDependencyDefinition& ConditionalEnable(ConditionalEnablePredicate predicate)
			{
				mConditionalEnablePredicates[TTypeListIndexOf<T, TypeList>] = predicate;
				return *this;
			}

			/** Returns StringIDs for enabled dependencies only. */
			TInlineArray<StringID, 4> GetEnabledDependencyIds(const RendererView& view) const
			{
				TInlineArray<StringID, 4> ids;
				u32 index = 0;

				([this, &ids, &index, &view]()
				{
					const auto& predicate = mConditionalEnablePredicates[index++];
					if(!predicate || predicate(view))
						ids.Add(NodeTypes::GetNodeId());
				}(), ...);

				return ids;
			}

			/** Returns StringIDs for all dependencies (regardless of enable state). */
			static TInlineArray<StringID, 4> GetAllDependencyIds()
			{
				TInlineArray<StringID, 4> ids;
				(ids.Add(NodeTypes::GetNodeId()), ...);
				return ids;
			}

			/** Returns a structure that may be used for retrieving node dependencies in a type-safe way. */
			RCNodeDependencies<NodeTypes...> ResolveDependencies(const RenderCompositorNodeInputs& inputs) const;

		private:
			std::array<ConditionalEnablePredicate, sizeof...(NodeTypes)> mConditionalEnablePredicates;
		};

		/** Helper that allows you to retrieve render compositor node dependencies based on their type. */
		template<typename... NodeTypes>
		struct RCNodeDependencies
		{
			using TypeList = TTypeList<NodeTypes...>;
			const RenderCompositorNodeInputs& Inputs;

			RCNodeDependencies(const RenderCompositorNodeInputs& inputs)
				: Inputs(inputs)
			{ }

			/** Retrieves a dependency node via direct array lookup. Returns nullptr if not enabled. */
			template<typename T>
			T* Get() const
			{
				constexpr u32 index = TTypeListIndexOf<T, TypeList>;
				return static_cast<T*>(Inputs.InputNodes[index]);
			}
		};

		template<typename... NodeTypes>
		RCNodeDependencies<NodeTypes...> RCNodeDependencyDefinition<NodeTypes...>::ResolveDependencies(const RenderCompositorNodeInputs& inputs) const
		{
			return RCNodeDependencies<NodeTypes...>(inputs);
		}

		/** Types of render compositor nodes. */
		enum class RenderCompositorNodeCategory
		{
			Primary, /**< Primary rendering pipeline node. */
			Utility /**< Utility node pulled in on-demand. */
		};

		/**
		 * Node in the render compositor hierarchy. Nodes can be implemented to perform specific rendering tasks. Each node
		 * can depend on other nodes in the hierarchy.
		 *
		 * @note	Implementations must provide a getNodeId() and getDependencies() static method, which are expected to
		 *			return a unique name for the implemented node, as well as a set of nodes it depends on.
		 */
		class RenderCompositorNode
		{
		public:
			virtual ~RenderCompositorNode() = default;

		protected:
			friend class RenderCompositor;

			/** Executes the task implemented in the node. */
			virtual void Render(const RenderCompositorNodeInputs& inputs) = 0;

			/**
			 * Cleans up any temporary resources allocated in a render() call. Any resources lasting longer than one frame
			 * should be kept alive and released in some other manner.
			 */
			virtual void Clear() = 0;
		};

		/**
		 * Performs rendering by iterating over a hierarchy of render nodes. Each node in the hierarchy performs a specific
		 * rendering tasks and passes its output to the dependant node. The system takes care of initializing, rendering and
		 * cleaning up nodes automatically depending on their dependencies.
		 */
		class RenderCompositor
		{
		public:
			struct NodeDescriptor;

		private:
			/** Contains internal information about a single render node. */
			struct NodeInfo
			{
				RenderCompositorNode* Node;
				const NodeDescriptor* NodeDescriptor;
				u32 LastUsedByNodeIndex;
				TInlineArray<RenderCompositorNode*, 4> Inputs;
			};

		public:
			~RenderCompositor();

			/**
			 * Rebuilds the render node hierarchy. Call this whenever some setting that may influence the render node
			 * dependencies changes.
			 *
			 * @param	view			Parent view to which this compositor belongs to.
			 * @param	primaryNodes	List of nodes to execute, in the order listed. Primary nodes might pull in other helper
			 *							nodes as needed by their dependencies.
			 */
			void Build(const RendererView& view, const TArray<StringID>& primaryNodes);

			/** Performs rendering using the current render node hierarchy. This is expected to be called once per frame. */
			void Execute(RenderCompositorNodeInputs& inputs) const;

		private:
			/** Clears the render node hierarchy. */
			void Clear();

			Vector<NodeInfo> mRegisteredNodes;
			bool mIsValid = false;

			/************************************************************************/
			/* 							NODE TYPES	                     			*/
			/************************************************************************/
		public:
			/** Contains information about a specific node type. */
			struct NodeDescriptor
			{
				virtual ~NodeDescriptor() = default;

				/** Creates a new node of this type. */
				virtual RenderCompositorNode* Create() const = 0;

				/** Returns identifier for all the dependencies of a node of this type. */
				virtual TInlineArray<StringID, 4> GetDependencies(const RendererView& view) const = 0;

				/** Returns identifier for all possible dependencies, regardless of enable state. */
				virtual TInlineArray<StringID, 4> GetAllDependencyIds() const = 0;

				/**
				 * Returns the category of this node. Primary nodes represent the main path through the pipeline, and must be explicitly
				 * provided when calling the RenderCompositor::Build() method. Utility nodes are pulled in on demand based on dependencies
				 * of the primary nodes (or other utility nodes).
				 */
				virtual RenderCompositorNodeCategory GetCategory() const = 0;

				StringID Id;
			};

			/** Templated implementation of NodeType. */
			template <class T>
			struct TNodeDescriptor : NodeDescriptor
			{
				TNodeDescriptor()
				{
					Id = T::GetNodeId();
				}

				RenderCompositorNode* Create() const override { return B3DNew<T>(); }
				TInlineArray<StringID, 4> GetDependencies(const RendererView& view) const override
				{
					return T::GetDependencyDefinition().GetEnabledDependencyIds(view);
				}

				TInlineArray<StringID, 4> GetAllDependencyIds() const override
				{
					return T::GetDependencyDefinition().GetAllDependencyIds();
				}

				RenderCompositorNodeCategory GetCategory() const override
				{
					return T::GetCategory();
				}
			};

			/**
			 * Registers a new type of node with the system. Each node type must first be registered before it can be used
			 * in the node hierarchy.
			 */
			template <class T>
			static void RegisterNodeType()
			{
				auto findIter = mNodeDescriptors.find(T::GetNodeId());
				if(findIter != mNodeDescriptors.end())
					B3D_LOG(Error, LogRenderer, "Found two render compositor nodes with the same name \"{0}\".", String(T::GetNodeId().CStr()));

				mNodeDescriptors[T::GetNodeId()] = B3DNew<TNodeDescriptor<T>>();
			}

			/** Releases any information about render node types. */
			static void CleanUp()
			{
				for(auto& entry : mNodeDescriptors)
					B3DDelete(entry.second);

				mNodeDescriptors.clear();
			}

		private:
			static UnorderedMap<StringID, NodeDescriptor*> mNodeDescriptors;
		};

		class RCNodeLightAccumulation;
		class RCNodeMotionBlur;
		class RCNodeHalfSceneColor;
		class RCNodeBloom;
		class RCNodeScreenSpaceLensFlare;
		class RCNodeTemporalAA;
		class RCNodeFilmGrain;
		class RCNodePostProcess;
		class RCNodeResolvedSceneDepth;
		class RCNodeSkybox;
		class RCNodeIndirectDiffuseLighting;
		class RCNodeSSAO;
		class RCNodeSSR;
		class RCNodeMSAACoverage;
		class RCNodeParticleSort;
		class RCNodeSceneColor;

		/************************************************************************/
		/* 							BASE PASS NODES	                     		*/
		/************************************************************************/

		/** Initializes the scene depth texture. Does not perform any rendering. */
		class RCNodeSceneDepth : public RenderCompositorNode
		{
		public:
			// Outputs
			SPtr<PooledRenderTexture> DepthTex;

			using DependencyDefinition = RCNodeDependencyDefinition<>;

			static StringID GetNodeId() { return "SceneDepth"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Initializes the GBuffer textures and renders the base pass into the GBuffer. The base pass includes all the opaque
		 * objects visible to the view.
		 */
		class RCNodeBasePass : public RenderCompositorNode
		{
		public:
			// Outputs
			SPtr<PooledRenderTexture> AlbedoTex;
			SPtr<PooledRenderTexture> NormalTex;
			SPtr<PooledRenderTexture> RoughMetalTex;
			SPtr<PooledRenderTexture> IdTex;
			SPtr<PooledRenderTexture> VelocityTex;

			SPtr<RenderTexture> RenderTarget;
			SPtr<RenderTexture> RenderTargetNoMask;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneDepth, RCNodeSceneColor, RCNodeParticleSort, RCNodeMSAACoverage>;

			static StringID GetNodeId() { return "BasePass"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Initializes the scene color texture and/or buffer. Does not perform any rendering. */
		class RCNodeSceneColor : public RenderCompositorNode
		{
		public:
			// Outputs
			/**
			 * Contains scene color. If MSAA is used this texture will be null until the texture array data is first resolved
			 * into this texture.
			 */
			SPtr<PooledRenderTexture> SceneColorTex;

			/**
			 * Texture array version of sceneColorTex. Only available when MSAA is used, since random writes to
			 * multisampled texture aren't supported on all render backends.
			 */
			SPtr<PooledRenderTexture> SceneColorTexArray;

			SPtr<RenderTexture> RenderTarget;

			/** Converts MSAA data from the texture array into the MSAA texture. */
			void MsaaTexArrayToTexture(GpuCommandBuffer& commandBuffer);

			/**
			 * Updates the internal scene color texture with the provided texture. MSAA scene color texture array must have
			 * been resolved before this call. This will not update the render target.
			 */
			void SetExternalTexture(const SPtr<PooledRenderTexture>& texture);

			/** Swaps the render textures between this node and the light accumulation nodes. */
			void Swap(RCNodeLightAccumulation* lightAccumNode);

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneDepth>;

			static StringID GetNodeId() { return "SceneColor"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Determines which samples in the GBuffer require per-sample shading and outputs a texture with the coverage (for use
		 * in compute shaders) and populates the primary stencil buffer as well (for use in non-compute shaders). Only relevant
		 * when rendering with MSAA enabled.
		 */
		class RCNodeMSAACoverage : public RenderCompositorNode
		{
		public:
			// Outputs
			SPtr<PooledRenderTexture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<>;

			static StringID GetNodeId() { return "MSAACoverage"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/************************************************************************/
		/* 							PARTICLE NODES                     			*/
		/************************************************************************/

		/** Simulates GPU particle systems. */
		class RCNodeParticleSimulate : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeBasePass, RCNodeSceneDepth>;

			static StringID GetNodeId() { return "ParticleSimulate"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Performs view-based sorting on CPU simulated particle systems. */
		class RCNodeParticleSort : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<>;

			static StringID GetNodeId() { return "ParticleSort"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/************************************************************************/
		/* 							LIGHTING NODES                     			*/
		/************************************************************************/

		/** Initializes the light accumulation texture and/or buffer. Does not perform any rendering. */
		class RCNodeLightAccumulation : public RenderCompositorNode
		{
		public:
			// Outputs
			/**
			 * Contains lighting information accumulated from multiple lights. If MSAA is used this texture will be null until
			 * the texture array data is first resolved into this texture.
			 */
			SPtr<PooledRenderTexture> LightAccumulationTex;

			/**
			 * Texture array version of lightAccumulationTex. Only available when MSAA is used, since random writes to
			 * multisampled texture aren't supported on all render backends.
			 */
			SPtr<PooledRenderTexture> LightAccumulationTexArray;

			SPtr<RenderTexture> RenderTarget;

			/** Converts MSAA data from the texture array into the MSAA texture. */
			void MsaaTexArrayToTexture(GpuCommandBuffer& commandBuffer);

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeSceneDepth>;

			static StringID GetNodeId() { return "LightAccumulation"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Handles lighting of surfaces illuminated directly, for both diffuse and specular components. Uses either tiled
		 * deferred or standard deferred rendering approach. Lighting information is output in the light accumulation buffer.
		 */
		class RCNodeDeferredDirectLighting : public RenderCompositorNode
		{
		public:
			// Outputs
			RCNodeLightAccumulation* Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeLightAccumulation, RCNodeBasePass, RCNodeSceneDepth, RCNodeSceneColor, RCNodeMSAACoverage>;

			static StringID GetNodeId() { return "DeferredDirectLighting"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SPtr<RenderTexture> mLightOcclusionRT;
		};

		/**
		 * Calculates specular lighting (specular reflections) using image based methods and deferred rendering. Uses either
		 * tiled deferred or standard deferred depending on the active feature set. The resulting lighting data is combined
		 * with direct lighting present in the light accumulation buffer and output to scene color texture/buffer.
		 */
		class RCNodeDeferredIndirectSpecularLighting : public RenderCompositorNode
		{
		public:
			// Outputs
			RCNodeLightAccumulation* Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeBasePass, RCNodeSceneDepth, RCNodeLightAccumulation, RCNodeSSR, RCNodeSSAO, RCNodeMSAACoverage, RCNodeIndirectDiffuseLighting>;

			static StringID GetNodeId() { return "DeferredIndirectSpecularLighting"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Evaluates indirect lighting from the light probe volume, if available, or the sky irradiance otherwise.
		 * Results are written to the light accumulation buffer.
		 */
		class RCNodeIndirectDiffuseLighting : public RenderCompositorNode
		{
		public:
			// Outputs to the unflattened RCNodeLightAccumulation

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeBasePass, RCNodeSceneDepth, RCNodeLightAccumulation, RCNodeSSAO, RCNodeDeferredDirectLighting>;

			static StringID GetNodeId() { return "IndirectDiffuseLighting"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Renders transparent objects using clustered forward rendering. Handles direct diffuse and specular, as well as
		 * indirect specular.
		 */
		class RCNodeClusteredForward : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeSkybox, RCNodeSceneDepth, RCNodeParticleSimulate, RCNodeParticleSort, RCNodeResolvedSceneDepth>;

			static StringID GetNodeId() { return "ClusteredForward"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SPtr<RenderTexture> renderTarget;
		};

		/**
		 * Renders the skybox into the scene color texture. If skybox texture is not available, a solid color will be rendered
		 * instead.
		 */
		class RCNodeSkybox : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeDeferredIndirectSpecularLighting>;

			static StringID GetNodeId() { return "Skybox"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Moves the contents of the scene color texture into the view's output target. */
		class RCNodeFinalResolve : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodePostProcess, RCNodeFilmGrain, RCNodeSceneColor, RCNodeClusteredForward, RCNodeResolvedSceneDepth>;

			static StringID GetNodeId() { return "FinalResolve"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/************************************************************************/
		/* 							POST PROCESS NODES                			*/
		/************************************************************************/

		/**
		 * Helper node used for post-processing. Takes care of allocating and switching between textures used for post process
		 * effects.
		 */
		class RCNodePostProcess : public RenderCompositorNode
		{
		public:
			/**
			 * Returns a texture that can be used for rendering a post-process effect, and the result of the previous
			 * output. Switches these textures so the next call they are returned in the opposite parameters.
			 */
			void GetAndSwitch(const RendererView& view, SPtr<RenderTexture>& output, SPtr<Texture>& lastFrame) const;

			/** Returns a texture that contains the last rendererd post process output. */
			SPtr<Texture> GetLastOutput() const;

			using DependencyDefinition = RCNodeDependencyDefinition<>;

			static StringID GetNodeId() { return "PostProcess"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			mutable SPtr<PooledRenderTexture> mOutput[2];
			mutable u32 mCurrentIdx = 0;
		};

		/** Calculates the eye adaptation values used for automatic exposure. */
		class RCNodeEyeAdaptation : public RenderCompositorNode
		{
		public:
			SPtr<PooledRenderTexture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeHalfSceneColor>;

			static StringID GetNodeId() { return "EyeAdaptation"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			/**
			 * Returns true if the more advanced (and more expensive) compute shader based method of computing eye adaptation
			 * should be used.
			 */
			bool UseHistogramEyeAdapatation(const RenderCompositorNodeInputs& inputs);

			SPtr<PooledRenderTexture> previous;
		};

		/**
		 * Performs tone mapping on the contents of the scene color texture. At the same time resolves MSAA into a non-MSAA
		 * scene color texture.
		 */
		class RCNodeTonemapping : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeEyeAdaptation, RCNodeSceneColor, RCNodeMotionBlur, RCNodePostProcess, RCNodeHalfSceneColor, RCNodeBloom, RCNodeScreenSpaceLensFlare>;

			static StringID GetNodeId() { return "Tonemapping"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SPtr<PooledRenderTexture> mTonemapLUT;
			u64 mTonemapLastUpdateHash = -1;
		};

		/** Renders the depth of field effect with a Bokeh flare simulating camera aperture shape. */
		class RCNodeBokehDOF : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeSceneColor, RCNodeSceneDepth, RCNodeLightAccumulation>;

			static StringID GetNodeId() { return "BokehDOF"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Renders the motion blur effect simulating light accumulation due to object and/or camera
		 * movement during a single frame. (In another words, it simulates blur due to exposure time
		 * as if on a real-world camera, i.e. depending on how long is the camera shutter open).
		 */
		class RCNodeMotionBlur : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeTemporalAA, RCNodeSceneColor, RCNodeSceneDepth, RCNodeLightAccumulation>;

			static StringID GetNodeId() { return "MotionBlur"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Renders the depth of field effect using Gaussian blurring. */
		class RCNodeGaussianDOF : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeTonemapping, RCNodeSceneDepth, RCNodePostProcess>;

			static StringID GetNodeId() { return "GaussianDOF"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Renders FXAA. */
		class RCNodeFXAA : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeGaussianDOF, RCNodePostProcess>;

			static StringID GetNodeId() { return "FXAA"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/************************************************************************/
		/* 							SCREEN SPACE								*/
		/************************************************************************/

		/** Generates a 1/2 size of the scene color texture. If MSAA only the first sample is used. */
		class RCNodeHalfSceneColor : public RenderCompositorNode
		{
		public:
			SPtr<PooledRenderTexture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor>;

			static StringID GetNodeId() { return "HalfSceneColor"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/**
		 * Generates a number of downsamples of the scene color texture. If MSAA only the first sample is used for
		 * for generating the downsampled versions.
		 */
		class RCNodeSceneColorDownsamples : public RenderCompositorNode
		{
		public:
			static constexpr u32 kMaxNumDownsamples = 6;

			SPtr<PooledRenderTexture> Output[kMaxNumDownsamples];
			u32 AvailableDownsamples = 0;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeHalfSceneColor>;

			static StringID GetNodeId() { return "SceneColorDownsamples"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Resolves the depth buffer (if multi-sampled). Otherwise just references the original depth buffer. */
		class RCNodeResolvedSceneDepth : public RenderCompositorNode
		{
		public:
			SPtr<PooledRenderTexture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneDepth, RCNodeBasePass>;

			static StringID GetNodeId() { return "ResolvedSceneDepth"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Builds the hierarchical Z buffer. */
		class RCNodeHiZ : public RenderCompositorNode
		{
		public:
			SPtr<PooledRenderTexture> Output;

			// Note: This doesn't actually use any gbuffer textures, but node is a dependency because it renders to the depth
			// buffer. In order to avoid keeping gbuffer textures alive I could separate out the base pass into its own node
			// perhaps. But at the moment it doesn't matter, as anything using HiZ also needs gbuffer.
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeResolvedSceneDepth, RCNodeBasePass>;

			static StringID GetNodeId() { return "HiZ"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Renders screen space ambient occlusion. */
		class RCNodeSSAO : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeResolvedSceneDepth, RCNodeBasePass>;

			SPtr<Texture> Output;

			static StringID GetNodeId() { return "SSAO"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SPtr<PooledRenderTexture> mPooledOutput;
		};

		/** Renders screen space reflections. */
		class RCNodeSSR : public RenderCompositorNode
		{
		public:
			SPtr<Texture> Output;

			~RCNodeSSR();

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneDepth, RCNodeLightAccumulation, RCNodeBasePass, RCNodeHiZ, RCNodeResolvedSceneDepth, RCNodeIndirectDiffuseLighting>;

			static StringID GetNodeId() { return "SSR"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			/** Cleans up any outputs. */
			void DeallocOutputs();

			SPtr<PooledRenderTexture> mPooledOutput;
			SPtr<PooledRenderTexture> mPrevFrame;
			bool mUsingTemporalAA = false;
		};

		/**
		 * Performs temporal anti-aliasing, using a special filter to accumulate multiple frames and thus
		 * improving image quality via temporal filtering.
		 */
		class RCNodeTemporalAA : public RenderCompositorNode
		{
		public:
			SPtr<Texture> Output;

			~RCNodeTemporalAA();

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeSceneDepth, RCNodeBasePass>;

			static StringID GetNodeId() { return "TemporalAA"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			/** Cleans up any outputs. */
			void DeallocOutputs();

			SPtr<PooledRenderTexture> mPooledOutput;
			SPtr<PooledRenderTexture> mPrevFrame;
		};

		/** Renders the bloom effect. */
		class RCNodeBloom : public RenderCompositorNode
		{
		public:
			SPtr<Texture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeSceneColorDownsamples, RCNodeEyeAdaptation>;

			static StringID GetNodeId() { return "Bloom"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SPtr<PooledRenderTexture> mPooledOutput;
		};

		/** Renders the screen-space lens flare effect. */
		class RCNodeScreenSpaceLensFlare : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeSceneColor, RCNodeSceneColorDownsamples>;

			static StringID GetNodeId() { return "ScreenSpaceLensFlare"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Renders the chromatic aberration effect. */
		class RCNodeChromaticAberration : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeFXAA, RCNodePostProcess>;

			static StringID GetNodeId() { return "ChromaticAberration"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Renders the film grain effect. */
		class RCNodeFilmGrain : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeChromaticAberration, RCNodePostProcess>;

			static StringID GetNodeId() { return "FilmGrain"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
