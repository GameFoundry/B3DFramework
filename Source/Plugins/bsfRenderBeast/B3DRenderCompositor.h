//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DTypeList.h"
#include "RenderState/B3DSurfaceShaderParameterSets.h"
#include "Shading/B3DGpuParticleSimulation.h"

namespace b3d
{
	class RendererExtension;

	namespace render
	{
		class FrameGraph;
		class RenderBeastScene;
		class RendererViewGroup;
		class RenderCompositorNode;
		struct FrameInfo;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Inputs provided to each node in the render compositor hierarchy */
		struct RenderCompositorNodeInputs
		{
			RenderCompositorNodeInputs(FrameGraph& graph, RenderBeastScene& scene, const RendererViewGroup& viewGroup, const RendererView& view, const RenderBeastOptions& options, const FrameInfo& frameInfo)
				: Graph(graph), Scene(scene), ViewGroup(viewGroup), View(view), Options(options), FrameInfo(frameInfo)
			{}

			/** Graph of the view, which nodes add their passes and textures to. */
			FrameGraph& Graph;

			RenderBeastScene& Scene;
			const RendererViewGroup& ViewGroup;
			const RendererView& View;
			const RenderBeastOptions& Options;
			const FrameInfo& FrameInfo;

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
			 * Called at the end of every frame, after every node rendered. Drops the node's references to the resources of the frame, which
			 * the frame graph owns, and moves resources kept for the next frame into the node's history.
			 */
			virtual void Clear() = 0;
		};

		/**
		 * Performs rendering by iterating over a hierarchy of render nodes. Each node in the hierarchy performs a specific
		 * rendering tasks and passes its output to the dependant node. The system takes care of initializing nodes and rendering
		 * them in the order of their dependencies, and clears every node at the end of the frame.
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

			/**
			 * Adds the passes of the current render node hierarchy to the graph of @p inputs. This is expected to be called once per
			 * frame. Allocates from the frame allocator, so the caller marks it, and frees it after the graph executes.
			 */
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
			TShared<Texture> DepthTex;

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
			TShared<Texture> AlbedoTex;
			TShared<Texture> NormalTex;
			TShared<Texture> RoughMetalTex;
			TShared<Texture> IdTex;
			TShared<Texture> VelocityTex;

			/** Outputs of the GPU particle simulation that the draws of the view read. */
			GpuParticleDrawInputs GpuParticleInputs;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneDepth, RCNodeSceneColor, RCNodeParticleSort, RCNodeMSAACoverage>;

			static StringID GetNodeId() { return "BasePass"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SurfaceShaderParameterSets mSurfaceShaderParameterSets;
		};

		/** Initializes the scene color texture and/or buffer. Does not perform any rendering. */
		class RCNodeSceneColor : public RenderCompositorNode
		{
		public:
			// Outputs
			/** Contains scene color. */
			TShared<Texture> SceneColorTex;

			/**
			 * Texture array version of SceneColorTex, with one slice per sample. Only available when MSAA is used, since random writes
			 * to multisampled textures aren't supported on all render backends, and only until AddArrayToMSAAPass().
			 */
			TShared<Texture> SceneColorTexArray;

			/** Adds a pass that copies the texture array into the multisampled scene color texture, and drops the array. */
			void AddArrayToMSAAPass(FrameGraph& graph);

			/**
			 * Updates the internal scene color texture with the provided texture. With MSAA, AddArrayToMSAAPass() must have been called
			 * before this call.
			 */
			void SetExternalTexture(const TShared<Texture>& texture);

			/** Swaps the textures between this node and the light accumulation nodes. */
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
			TShared<Texture> Output;

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
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeBasePass, RCNodeSceneDepth, RCNodeResolvedSceneDepth>;

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
			// Outputs
			/**
			 * Indices of the particles of the CPU simulated systems the view sees, as texel coordinates packed in two 16-bit values. Each
			 * system's are at its ParticleRenderState::ParticleIndicesOffset, distance sorted for the view if the system sorts by distance.
			 * Kept across frames and rewritten every frame. Holds at least one element.
			 */
			TShared<GpuBuffer> ParticleIndices;

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
			/** Contains lighting information accumulated from multiple lights. */
			TShared<Texture> LightAccumulationTex;

			/**
			 * Texture array version of LightAccumulationTex, with one slice per sample, cleared to zero. Only available when MSAA is
			 * used, since random writes to multisampled textures aren't supported on all render backends, and only until
			 * AddArrayToMSAAPass().
			 */
			TShared<Texture> LightAccumulationTexArray;

			/** Adds a pass that copies the texture array into the multisampled light accumulation texture, and drops the array. */
			void AddArrayToMSAAPass(FrameGraph& graph);

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneDepth>;

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
		};

		/**
		 * Calculates specular lighting (specular reflections) using image based methods and deferred rendering. Uses either
		 * tiled deferred or standard deferred depending on the active feature set. The resulting lighting data is combined
		 * with direct lighting present in the light accumulation buffer and output to scene color texture/buffer.
		 */
		class RCNodeDeferredIndirectSpecularLighting : public RenderCompositorNode
		{
		public:
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
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeSkybox, RCNodeSceneDepth, RCNodeParticleSimulate, RCNodeParticleSort, RCNodeResolvedSceneDepth,
				RCNodeBasePass>;

			static StringID GetNodeId() { return "ClusteredForward"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			SurfaceShaderParameterSets mSurfaceShaderParameterSets;
		};

		/**
		 * Renders the skybox into the scene color texture. If skybox texture is not available, a solid color will be rendered
		 * instead.
		 */
		class RCNodeSkybox : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeSceneColor, RCNodeSceneDepth, RCNodeDeferredIndirectSpecularLighting>;

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

		/** Helper node used for post-processing. Creates the textures post-process effects write their results to. */
		class RCNodePostProcess : public RenderCompositorNode
		{
		public:
			/**
			 * Creates a transient texture of @p graph for a post-process effect to write its results to, and makes it the last output. The
			 * effect reads the previous output from GetLastOutput() before calling this.
			 */
			TShared<Texture> CreateOutput(FrameGraph& graph, const RendererView& view);

			/** Returns the texture that contains the last rendered post process output. */
			TShared<Texture> GetLastOutput() const { return mOutput; }

			using DependencyDefinition = RCNodeDependencyDefinition<>;

			static StringID GetNodeId() { return "PostProcess"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			TShared<Texture> mOutput;
		};

		/** Calculates the eye adaptation values used for automatic exposure. */
		class RCNodeEyeAdaptation : public RenderCompositorNode
		{
		public:
			TShared<Texture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeHalfSceneColor>;

			static StringID GetNodeId() { return "EyeAdaptation"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Utility; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;

			TShared<Texture> previous;
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

			TShared<Texture> mTonemapLUT;
			u64 mTonemapLastUpdateHash = -1;
		};

		/** Renders the depth of field effect with a Bokeh flare simulating camera aperture shape. */
		class RCNodeBokehDOF : public RenderCompositorNode
		{
		public:
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeSceneColor, RCNodeSceneDepth, RCNodeLightAccumulation, RCNodeResolvedSceneDepth>;

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
			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeTonemapping, RCNodeResolvedSceneDepth, RCNodePostProcess>;

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
			TShared<Texture> Output;

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

			TShared<Texture> Output[kMaxNumDownsamples];
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
			TShared<Texture> Output;

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
			TShared<Texture> Output;

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

			TShared<Texture> Output;

			static StringID GetNodeId() { return "SSAO"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
		};

		/** Renders screen space reflections. */
		class RCNodeSSR : public RenderCompositorNode
		{
		public:
			TShared<Texture> Output;

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

			/** Output of the previous frame, which the temporal filter reads at rest. Null if there is none. */
			TShared<Texture> mPreviousFrame;

			/** Output extracted from this frame's graph, which becomes the previous frame when the frame ends. Null if none is kept. */
			TShared<Texture> mHistoryOutput;

			bool mUsingTemporalAA = false;
		};

		/**
		 * Performs temporal anti-aliasing, using a special filter to accumulate multiple frames and thus
		 * improving image quality via temporal filtering.
		 */
		class RCNodeTemporalAA : public RenderCompositorNode
		{
		public:
			TShared<Texture> Output;

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

			/** Output of the previous frame, which the temporal filter reads at rest. Null if there is none. */
			TShared<Texture> mPreviousFrame;

			/** Output extracted from this frame's graph, which becomes the previous frame when the frame ends. Null if none is kept. */
			TShared<Texture> mHistoryOutput;
		};

		/** Renders the bloom effect. */
		class RCNodeBloom : public RenderCompositorNode
		{
		public:
			TShared<Texture> Output;

			using DependencyDefinition = RCNodeDependencyDefinition<RCNodeClusteredForward, RCNodeSceneColorDownsamples, RCNodeEyeAdaptation>;

			static StringID GetNodeId() { return "Bloom"; }
			static DependencyDefinition GetDependencyDefinition();
			static RenderCompositorNodeCategory GetCategory() { return RenderCompositorNodeCategory::Primary; }

		protected:
			void Render(const RenderCompositorNodeInputs& inputs) override;
			void Clear() override;
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
