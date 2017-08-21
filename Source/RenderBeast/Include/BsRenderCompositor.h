//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	/** @addtogroup RenderBeast
	 *  @{
	 */
	
	/** Inputs provided to each node in the render compositor hierarchy */
	struct RenderCompositorNodeInputs
	{
		RenderCompositorNodeInputs(const RendererViewGroup& viewGroup, const RendererView& view, const SceneInfo& scene, 
			const RenderBeastOptions& options, const FrameInfo& frameInfo)
			: viewGroup(viewGroup), view(view), scene(scene), options(options), frameInfo(frameInfo)
		{ }

		const RendererViewGroup& viewGroup;
		const RendererView& view;
		const SceneInfo& scene;
		const RenderBeastOptions& options;
		const FrameInfo& frameInfo;

		// Callbacks to external systems can hook into the compositor
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
		virtual ~RenderCompositorNode() { }

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
		/** Contains internal information about a single render node. */
		struct NodeInfo
		{
			RenderCompositorNode* node;
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
			virtual ~NodeType() {};

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
				LOGERR("Found two render compositor nodes with the same name \"" + String(T::getNodeId().cstr()) + "\".");

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
	class RCNodeGBuffer : public RenderCompositorNode
	{
	public:
		// Outputs
		SPtr<PooledRenderTexture> albedoTex;
		SPtr<PooledRenderTexture> normalTex;
		SPtr<PooledRenderTexture> roughMetalTex;

		SPtr<RenderTexture> renderTarget;

		static StringID getNodeId() { return "GBuffer"; }
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
		 * Contains scene color. If MSAA is used this texture will be null until the flattened data from the buffer is
		 * first resolved into this texture.
		 */
		SPtr<PooledRenderTexture> sceneColorTex;

		/** 
		 * Flattened, buffer version of sceneColorTex. Only available when MSAA is used, since random writes to multisampled
		 * textures aren't supported on all render backends.
		 */
		SPtr<PooledStorageBuffer> flattenedSceneColorBuffer;

		SPtr<RenderTexture> renderTarget;

		static StringID getNodeId() { return "SceneColor"; }
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
		 * the flattened data from the buffer is first resolved into this texture.
		 */
		SPtr<PooledRenderTexture> lightAccumulationTex;

		/** 
		 * Flattened, buffer version of lightAccumulationTex. Only available when MSAA is used, since random writes to
		 * multisampled textures aren't supported on all render backends.
		 */
		SPtr<PooledStorageBuffer> flattenedLightAccumBuffer;

		SPtr<RenderTexture> renderTarget;

		static StringID getNodeId() { return "LightAccumulation"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** 
	 * Performs tiled deferred lighting, outputing any lighting information in the light accumulation buffer. 
	 * By default only non-shadowed lights are rendered, as shadowed ones are handled using standard deferred.
	 */
	class RCNodeTiledDeferredLighting : public RenderCompositorNode
	{
	public:
		// Outputs
		RCNodeLightAccumulation* output;

		static StringID getNodeId() { return "TiledDeferredLighting"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Performs standard deferred lighting, outputting any lighting information in the light accumulation buffer.
	 * Only renders shadowed lights.
	 */
	class RCNodeStandardDeferredLighting : public RenderCompositorNode
	{
	public:
		// Outputs
		RCNodeLightAccumulation* output;

		static StringID getNodeId() { return "StandardDeferredLighting"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;

		SPtr<RenderTexture> mRenderTarget;
	};

	/**
	 * In case light accumulation was rendered into a buffer instead of a texture (if MSAA is used), this node will
	 * unflatten the buffer and write its contents into the light accumulation texture.
	 */
	class RCNodeUnflattenLightAccum : public RenderCompositorNode
	{
	public:
		// Outputs
		RCNodeLightAccumulation* output;

		static StringID getNodeId() { return "UnflattenLightAccum"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Evaluates indirect lighting from the light probe volume, if available, or the sky irradiance otherwise. */
	class RCNodeIndirectLighting : public RenderCompositorNode
	{
	public:
		// Outputs to the unflattened RCNodeLightAccumulation

		static StringID getNodeId() { return "IndirectLighting"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/**
	 * Performs tiled deferred image based lighting, combines it with direct lighting present in the light accumulation
	 * buffer and outputs the results to the scene color texture or buffer.
	 */
	class RCNodeTiledDeferredIBL : public RenderCompositorNode
	{
	public:
		// Outputs
		RCNodeLightAccumulation* output;

		static StringID getNodeId() { return "TiledDeferredIBL"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders transparent objects using clustered forward rendering. */
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
	};

	/**
	 * In case scene color was rendered into a buffer instead of a texture (if MSAA is used), this node will
	 * unflatten the buffer and write its contents into the scene color texture.
	 */
	class RCNodeUnflattenSceneColor : public RenderCompositorNode
	{
	public:
		// Outputs
		RCNodeSceneColor* output;

		static StringID getNodeId() { return "UnflattenSceneColor"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
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
		RCNodePostProcess();

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
		mutable bool mAllocated[2];
		mutable UINT32 mCurrentIdx = 0;
	};

	/**
	 * Performs tone mapping on the contents of the scene color texture. At the same time resolves MSAA into a non-MSAA
	 * scene color texture.
	 */
	class RCNodeTonemapping : public RenderCompositorNode
	{
	public:
		SPtr<PooledRenderTexture> eyeAdaptation;
		SPtr<PooledRenderTexture> prevEyeAdaptation;

		~RCNodeTonemapping();

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

		bool mPassThrough = false;
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
		SPtr<PooledRenderTexture> output;

		static StringID getNodeId() { return "SSAO"; }
		static SmallVector<StringID, 4> getDependencies(const RendererView& view);
	protected:
		/** @copydoc RenderCompositorNode::render */
		void render(const RenderCompositorNodeInputs& inputs) override;

		/** @copydoc RenderCompositorNode::clear */
		void clear() override;
	};

	/** Renders screen space reflections. */
	class RCNodeSSR : public RenderCompositorNode
	{
	public:
		SPtr<PooledRenderTexture> output;

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

		SPtr<PooledRenderTexture> mPrevFrame;
	};

	/** @} */
}}
