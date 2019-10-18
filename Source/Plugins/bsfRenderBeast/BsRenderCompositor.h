//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"

namespace bs
{
	class RendererExtension;

namespace ct
{
	struct SceneInfo;
	class RendererViewGroup;
	class RenderCompositorNode;
	struct PooledStorageBuffer;
	struct FrameInfo;
	class RCNodeLightAccumulation;

	/** @addtogroup RenderBeast
	 *  @{
	 */
	
	/** Inputs provided to each node in the render compositor hierarchy */
	struct RenderCompositorNodeInputs
	{
		RenderCompositorNodeInputs(const RendererViewGroup& viewGroup, const RendererView& view, const SceneInfo& scene,
			const RenderBeastOptions& options, const FrameInfo& frameInfo, RenderBeastFeatureSet featureSet)
			: viewGroup(viewGroup), view(view), scene(scene), options(options), frameInfo(frameInfo), featureSet(featureSet)
		{ }

		const RendererViewGroup& viewGroup;
		const RendererView& view;
		const SceneInfo& scene;
		const RenderBeastOptions& options;
		const FrameInfo& frameInfo;
		const RenderBeastFeatureSet featureSet;

		// Callbacks to external systems can hook into the compositor
		SmallVector<RendererExtension*, 4> extPrepare;
		SmallVector<RendererExtension*, 4> extPreBasePass;
		SmallVector<RendererExtension*, 4> extPostBasePass;
		SmallVector<RendererExtension*, 4> extPostLighting;
		SmallVector<RendererExtension*, 4> extOverlay;

		SmallVector<RenderCompositorNode*, 4> inputNodes;
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
		virtual void render(const RenderCompositorNodeInputs& inputs) = 0;

		/**
		 * Cleans up any temporary resources allocated in a render() call. Any resources lasting longer than one frame
		 * should be kept alive and released in some other manner.
		 */
		virtual void clear() = 0;
	};

	/**
	 * Performs rendering by iterating over a hierarchy of render nodes. Each node in the hierarchy performs a specific
	 * rendering tasks and passes its output to the dependant node. The system takes care of initializing, rendering and
	 * cleaning up nodes automatically depending on their dependencies.
	 */
	class RenderCompositor
	{
	public:
		struct NodeType;

	private:
		/** Contains internal information about a single render node. */
		struct NodeInfo
		{
			RenderCompositorNode* node;
			NodeType* nodeType;
			UINT32 lastUseIdx;
			SmallVector<RenderCompositorNode*, 4> inputs;
		};

	public:
		~RenderCompositor();

		/**
		 * Rebuilds the render node hierarchy. Call this whenever some setting that may influence the render node
		 * dependencies changes.
		 *
		 * @param[in]	view		Parent view to which this compositor belongs to.
		 * @param[in]	finalNode	Identifier of the final node in the node hierarchy. This node is expected to write
		 *							to the views render target. All other nodes will be deduced from this node's
		 *							dependencies.
		 */
		void build(const RendererView& view, const StringID& finalNode);

		/** Performs rendering using the current render node hierarchy. This is expected to be called once per frame. */
		void execute(RenderCompositorNodeInputs& inputs) const;

	private:
		/** Clears the render node hierarchy. */
		void clear();

		Vector<NodeInfo> mNodeInfos;
		bool mIsValid = false;

		/************************************************************************/
		/* 							NODE TYPES	                     			*/
		/************************************************************************/
	public:
		/** Contains information about a specific node type. */
		struct NodeType
		{
			virtual ~NodeType() = default;

			/** Creates a new node of this type. */
			virtual RenderCompositorNode* create() const = 0;

			/** Returns identifier for all the dependencies of a node of this type. */
			virtual SmallVector<StringID, 4> getDependencies(const RendererView& view) const = 0;

			StringID id;
		};
		
		/** Templated implementation of NodeType. */
		template<class T>
		struct TNodeType : NodeType
		{
			TNodeType()
			{
				id = T::getNodeId();
			}

			/** @copydoc NodeType::create() */
			RenderCompositorNode* create() const override { return bs_new<T>(); }

			/** @copydoc NodeType::getDependencies() */
			SmallVector<StringID, 4> getDependencies(const RendererView& view) const override
			{
				return T::getDependencies(view);
			}
		};

		/**
		 * Registers a new type of node with the system. Each node type must first be registered before it can be used
		 * in the node hierarchy.
		 */
		template<class T>
		static void registerNodeType()
		{
			auto findIter = mNodeTypes.find(T::getNodeId());
			if (findIter != mNodeTypes.end())
				BS_LOG(Error, Renderer, "Found two render compositor nodes with the same name \"{0}\".", String(T::getNodeId().c_str()));

			mNodeTypes[T::getNodeId()] = bs_new<TNodeType<T>>();
		}

		/** Releases any information about render node types. */
		static void cleanUp()
		{
			for (auto& entry : mNodeTypes)
				bs_delete(entry.second);

			mNodeTypes.clear();
		}

	private:
		static UnorderedMap<StringID, NodeType*> mNodeTypes;
	};

	/************************************************************************/
	/* 							BASE PASS NODES	                     		*/
	/************************************************************************/

	/** Initializes the scene depth texture. Does not perform any rendering. */
	class RCNodeSceneDepth : public RenderCompositorNode
	{
	public:
		// Outputs
		SPtr<PooledRenderTexture> depthTex;

		static StringID getNodeId() { return "SceneDepth"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Initializes the GBuffer textures and renders the base pass into the GBuffer. The base pass includes all the opaque
	 * objects visible to the view.
	 */
	class RCNodeBasePass : public RenderCompositorNode
	{
	public:
		// Outputs
		SPtr<PooledRenderTexture> albedoTex;
		SPtr<PooledRenderTexture> normalTex;
		SPtr<PooledRenderTexture> roughMetalTex;
		SPtr<PooledRenderTexture> idTex;
		SPtr<PooledRenderTexture> velocityTex;

		SPtr<RenderTexture> renderTarget;
		SPtr<RenderTexture> renderTargetNoMask;

		static StringID getNodeId() { return "BasePass"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
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
		SPtr<PooledRenderTexture> sceneColorTex;

		/**
		 * Texture array version of sceneColorTex. Only available when MSAA is used, since random writes to
		 * multisampled texture aren't supported on all render backends.
		 */
		SPtr<PooledRenderTexture> sceneColorTexArray;

		SPtr<RenderTexture> renderTarget;

		/** Converts MSAA data from the texture array into the MSAA texture. */
		void msaaTexArrayToTexture();

		/**
		 * Updates the internal scene color texture with the provided texture. MSAA scene color texture array must have
		 * been resolved before this call. This will not update the render target.
		 */
		void setExternalTexture(const SPtr<PooledRenderTexture>& texture);

		/** Swaps the render textures between this node and the light accumulation nodes. */
		void swap(RCNodeLightAccumulation* lightAccumNode);

		static StringID getNodeId() { return "SceneColor"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
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
		SPtr<PooledRenderTexture> output;

		static StringID getNodeId() { return "MSAACoverage"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/************************************************************************/
	/* 							UTILITY NODES                     			*/
	/************************************************************************/

	/** Simulates GPU particle systems. */
	class RCNodeParticleSimulate : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "ParticleSimulate"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Performs view-based sorting on CPU simulated particle systems. */
	class RCNodeParticleSort : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "ParticleSort"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
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
		SPtr<PooledRenderTexture> lightAccumulationTex;

		/**
		 * Texture array version of lightAccumulationTex. Only available when MSAA is used, since random writes to
		 * multisampled texture aren't supported on all render backends.
		 */
		SPtr<PooledRenderTexture> lightAccumulationTexArray;

		SPtr<RenderTexture> renderTarget;

		/** Converts MSAA data from the texture array into the MSAA texture. */
		void msaaTexArrayToTexture();

		static StringID getNodeId() { return "LightAccumulation"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Handles lighting of surfaces illuminated directly, for both diffuse and specular components. Uses either tiled
	 * deferred or standard deferred rendering approach. Lighting information is output in the light accumulation buffer.
	 */
	class RCNodeDeferredDirectLighting : public RenderCompositorNode
	{
	public:
		// Outputs
		RCNodeLightAccumulation* output;

		static StringID getNodeId() { return "DeferredDirectLighting"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

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
		RCNodeLightAccumulation* output;

		static StringID getNodeId() { return "DeferredIndirectSpecularLighting"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Evaluates indirect lighting from the light probe volume, if available, or the sky irradiance otherwise.
	 * Results are written to the light accumulation buffer.
	 */
	class RCNodeIndirectDiffuseLighting : public RenderCompositorNode
	{
	public:
		// Outputs to the unflattened RCNodeLightAccumulation

		static StringID getNodeId() { return "IndirectDiffuseLighting"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Renders transparent objects using clustered forward rendering. Handles direct diffuse and specular, as well as
	 * indirect specular.
	 */
	class RCNodeClusteredForward : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "ClusteredForward"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		SPtr<RenderTexture> renderTarget;
	};

	/**
	 * Renders the skybox into the scene color texture. If skybox texture is not available, a solid color will be rendered
	 * instead.
	 */
	class RCNodeSkybox : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "Skybox"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Moves the contents of the scene color texture into the view's output target. */
	class RCNodeFinalResolve : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "FinalResolve"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
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
		void getAndSwitch(const RendererView& view, SPtr<RenderTexture>& output, SPtr<Texture>& lastFrame) const;

		/** Returns a texture that contains the last rendererd post process output. */
		SPtr<Texture> getLastOutput() const;

		static StringID getNodeId() { return "PostProcess"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		mutable SPtr<PooledRenderTexture> mOutput[2];
		mutable UINT32 mCurrentIdx = 0;
	};

	/** Calculates the eye adaptation values used for automatic exposure. */
	class RCNodeEyeAdaptation : public RenderCompositorNode
	{
	public:
		SPtr<PooledRenderTexture> output;

		static StringID getNodeId() { return "EyeAdaptation"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		/**
		 * Returns true if the more advanced (and more expensive) compute shader based method of computing eye adaptation
		 * should be used.
		 */
		bool useHistogramEyeAdapatation(const RenderCompositorNodeInputs& inputs);

		SPtr<PooledRenderTexture> previous;
	};

	/**
	 * Performs tone mapping on the contents of the scene color texture. At the same time resolves MSAA into a non-MSAA
	 * scene color texture.
	 */
	class RCNodeTonemapping : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "Tonemapping"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		SPtr<PooledRenderTexture> mTonemapLUT;
		UINT64 mTonemapLastUpdateHash = -1;
	};

	/** Renders the depth of field effect with a Bokeh flare simulating camera aperture shape. */
	class RCNodeBokehDOF : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "BokehDOF"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Renders the motion blur effect simulating light accumulation due to object and/or camera
	 * movement during a single frame. (In another words, it simulates blur due to exposure time
	 * as if on a real-world camera, i.e. depending on how long is the camera shutter open).
	 */
	class RCNodeMotionBlur : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "MotionBlur"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView & view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders the depth of field effect using Gaussian blurring. */
	class RCNodeGaussianDOF : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "GaussianDOF"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders FXAA. */
	class RCNodeFXAA : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "FXAA"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/************************************************************************/
	/* 							SCREEN SPACE								*/
	/************************************************************************/

	/** Generates a 1/2 size of the scene color texture. If MSAA only the first sample is used. */
	class RCNodeHalfSceneColor : public RenderCompositorNode
	{
	public:
		SPtr<PooledRenderTexture> output;

		static StringID getNodeId() { return "HalfSceneColor"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Generates a number of downsamples of the scene color texture. If MSAA only the first sample is used for
	 * for generating the downsampled versions.
	 */
	class RCNodeSceneColorDownsamples : public RenderCompositorNode
	{
	public:
		static constexpr UINT32 MAX_NUM_DOWNSAMPLES = 6;

		SPtr<PooledRenderTexture> output[MAX_NUM_DOWNSAMPLES];
		UINT32 availableDownsamples = 0;

		static StringID getNodeId() { return "SceneColorDownsamples"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Resolves the depth buffer (if multi-sampled). Otherwise just references the original depth buffer. */
	class RCNodeResolvedSceneDepth : public RenderCompositorNode
	{
	public:
		SPtr<PooledRenderTexture> output;

		static StringID getNodeId() { return "ResolvedSceneDepth"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Builds the hierarchical Z buffer. */
	class RCNodeHiZ : public RenderCompositorNode
	{
	public:
		SPtr<PooledRenderTexture> output;

		static StringID getNodeId() { return "HiZ"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders screen space ambient occlusion. */
	class RCNodeSSAO : public RenderCompositorNode
	{
	public:
		SPtr<Texture> output;

		static StringID getNodeId() { return "SSAO"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		SPtr<PooledRenderTexture> mPooledOutput;
	};

	/** Renders screen space reflections. */
	class RCNodeSSR : public RenderCompositorNode
	{
	public:
		SPtr<Texture> output;

		~RCNodeSSR();

		static StringID getNodeId() { return "SSR"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		/** Cleans up any outputs. */
		void deallocOutputs();

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
		SPtr<Texture> output;

		~RCNodeTemporalAA();

		static StringID getNodeId() { return "TemporalAA"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		/** Cleans up any outputs. */
		void deallocOutputs();

		SPtr<PooledRenderTexture> mPooledOutput;
		SPtr<PooledRenderTexture> mPrevFrame;
	};

	/** Renders the bloom effect. */
	class RCNodeBloom : public RenderCompositorNode
	{
	public:
		SPtr<Texture> output;

		static StringID getNodeId() { return "Bloom"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		SPtr<PooledRenderTexture> mPooledOutput;
	};

	/** Renders the screen-space lens flare effect. */
	class RCNodeScreenSpaceLensFlare : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "ScreenSpaceLensFlare"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders the chromatic aberration effect. */
	class RCNodeChromaticAberration : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "ChromaticAberration"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView & view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders the film grain effect. */
	class RCNodeFilmGrain : public RenderCompositorNode
	{
	public:
		static StringID getNodeId() { return "FilmGrain"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView & view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** @} */
}}
