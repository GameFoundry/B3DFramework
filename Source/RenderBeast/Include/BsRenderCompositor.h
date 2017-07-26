//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"

namespace bs { namespace ct
{
	struct SceneInfo;
	class RendererViewGroup;
	class RenderCompositorNode;
	struct PooledStorageBuffer;

	/** @addtogroup RenderBeast
	 *  @{
	 */
	
	// TODO - Doc
	struct RenderCompositorNodeInputs
	{
		RenderCompositorNodeInputs(const RendererViewGroup& viewGroup, const RendererView& view, const SceneInfo& scene, 
			const RenderBeastOptions& options, const SmallVector<RenderCompositorNode*, 4>& inputNodes)
			: viewGroup(viewGroup), view(view), scene(scene), options(options), inputNodes(inputNodes)
		{ }

		const RendererViewGroup& viewGroup;
		const RendererView& view;
		const SceneInfo& scene;
		const RenderBeastOptions& options;

		SmallVector<RenderCompositorNode*, 4> inputNodes;
	};

	// TODO - Doc
	class RenderCompositorNode
	{
	public:
		virtual ~RenderCompositorNode() { }

	protected:
		friend class RenderCompositor;

		// TODO - Doc
		virtual void render(const RenderCompositorNodeInputs& inputs) = 0;

		// TODO - Doc. Note this is meant to clean up data within a single render pass, if there is permanent data lasting
		// multiple frames, only clear that when the node is destroyed.
		virtual void clear() = 0;
	};

	// TODO - Doc
	class RenderCompositor
	{
		// TODO - Doc
		struct NodeInfo
		{
			RenderCompositorNode* node;
			UINT32 lastUseIdx;
			SmallVector<RenderCompositorNode*, 4> inputs;
		};
	public:
		~RenderCompositor()
		{
			clear();
		}

		// TODO - Doc
		void build(const RendererView& view, const StringID& finalNode);

		// TODO - Doc
		void execute(const RendererViewGroup& viewGroup, const RendererView& view, const SceneInfo& scene,
			const RenderBeastOptions& options) const;

	private:
		// TODO - Doc
		void clear()
		{
			for (auto& entry : mNodeInfos)
				bs_delete(entry.node);

			mNodeInfos.clear();
			mIsValid = false;
		}

		Vector<NodeInfo> mNodeInfos;
		bool mIsValid = false;

		/************************************************************************/
		/* 							NODE TYPES	                     			*/
		/************************************************************************/
	public:
		// TODO - Doc
		struct NodeType
		{
			virtual ~NodeType() {};

			// TODO - Doc
			virtual RenderCompositorNode* create() const = 0;

			// TODO - Doc
			virtual SmallVector<StringID, 4> getDependencies(const RendererView& view) const = 0;

			StringID id;
		};

		// TODO - Doc
		template<class T>
		struct TNodeType : NodeType
		{
			// TODO - Doc
			RenderCompositorNode* create() const override { return bs_new<T>(); }

			// TODO - Doc
			SmallVector<StringID, 4> getDependencies(const RendererView& view) const override
			{
				return T::getDependencies(view);
			}
		};

		// TODO - Doc
		template<class T>
		static void registerNodeType()
		{
			auto findIter = mNodeTypes.find(T::getNodeId());
			if (findIter != mNodeTypes.end())
				LOGERR("Found two render compositor nodes with the same name \"" + String(T::getNodeId().cstr()) + "\".");

			mNodeTypes[T::getNodeId()] = bs_new<TNodeType<T>>();
		}

		// TODO - Doc
		static void cleanUp()
		{
			for (auto& entry : mNodeTypes)
				bs_delete(entry.second);

			mNodeTypes.clear();
		}

		static UnorderedMap<StringID, NodeType*> mNodeTypes;
	};

	/************************************************************************/
	/* 							BASE PASS NODES	                     		*/
	/************************************************************************/

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

		SPtr<PooledRenderTexture> mLightOcclusionTex;
		SPtr<RenderTexture> mRenderTarget;
	};

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

	// TODO - Doc
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

	/** @} */
}}
