
namespace
{
	/** Determines transitions required in-between ExecuteCommandList calls, as well as waits required between queues. */
	class D3D12SubmissionTransitionVisitor : public GpuSubmissionTransitionVisitor, public D3D12BufferPageSubmissionTransitionVisitor
	{
	public:
		D3D12SubmissionTransitionVisitor(D3D12GpuDevice& device, GpuQueueId destinationQueueId, D3D12GpuCommandBufferSubmitInformation& submitInformation) : mDevice(device), mDestinationQueueId(destinationQueueId), mSubmitInformation(submitInformation)
		{ }

		void VisitBuffer(const GpuSubmissionBufferTransition& transition) override
		{
			AddParallelAccessWait(transition);
		}

		void VisitBufferPage(D3D12BufferPage&, const GpuSubmissionTransition& transition) override
		{
			AddParallelAccessWait(transition);
		}

		void VisitImage(const GpuSubmissionImageTransition& transition) override
		{
			// Resolve the native layouts at the command buffer boundaries.
			D3D12Image* const image = static_cast<D3D12Image*>(transition.Image);
			D3D12ImageSubresource* const subresource = static_cast<D3D12ImageSubresource*>(transition.StateResource);

			const GpuQueueType destinationQueueType = mDestinationQueueId.GetType();
			const D3D12TextureLayout committedLayout = subresource->GetLayout();
			const bool allowConcurrentQueueReads = image->AllowsConcurrentQueueReads();

			const D3D12TextureLayout initialLayout = transition.InitialLayout != GpuImageLayout::Undefined ? D3D12BarrierUtility::GetTextureLayout(transition.InitialLayout, destinationQueueType, transition.ImageRange.AspectMask, allowConcurrentQueueReads) : committedLayout;
			const D3D12TextureLayout finalLayout = transition.FinalLayout != GpuImageLayout::Undefined ? D3D12BarrierUtility::GetTextureLayout(transition.FinalLayout, destinationQueueType, transition.ImageRange.AspectMask, allowConcurrentQueueReads) : initialLayout;

			if(!B3D_ENSURE_LOG(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(initialLayout, transition.ImageRange.AspectMask, destinationQueueType), "D3D12 image layout is not supported on destination queue type {0}.", (u32)destinationQueueType))
			{
				mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;
				return;
			}

			// Select a queue capable of establishing the initial layout.
			D3D12TextureLayout transitionSourceLayout = committedLayout;
			GpuQueueId layoutTransitionQueueId;
			const bool hasLayoutTransitionQueue = subresource->GetLayoutTransitionQueueId(layoutTransitionQueueId);
			bool layoutTransitionQueueChanged = false;

			const bool needsLayoutTransition = committedLayout != initialLayout;
			const bool destinationCanTransitionLayout =
				D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(committedLayout, transition.ImageRange.AspectMask, destinationQueueType) &&
				D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(initialLayout, transition.ImageRange.AspectMask, destinationQueueType);

			// Transition layout on the source queue if the destination queue cannot perform the transform itself
			const bool needsSourceRelease = hasLayoutTransitionQueue && layoutTransitionQueueId.Id != mDestinationQueueId.Id && needsLayoutTransition && !destinationCanTransitionLayout;
			if(needsSourceRelease)
			{
				const GpuQueueType sourceQueueType = layoutTransitionQueueId.GetType();
				const D3D12TextureLayout releaseLayout = D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(initialLayout, transition.ImageRange.AspectMask, sourceQueueType) ? initialLayout : D3D12TextureLayout::Common();

				SourceTransitionBuildInformation& sourceTransition = GetSourceTransition(layoutTransitionQueueId, transition.ExclusiveAccessWaitMask);
				sourceTransition.Barriers.AddTextureBarrier(image->GetD3D12Resource(), transition.ImageRange, GpuBarrierScope(), GpuImageLayout::Undefined, GpuImageLayout::Undefined, committedLayout, releaseLayout);

				mSubmitInformation.RequiredWaitMask |= layoutTransitionQueueId;
				transitionSourceLayout = releaseLayout;
			}
			// Destination queue cannot perform the layout transition, and no layout transition queue is associated with the resource, use the graphics queue for layout transition
			else if(!hasLayoutTransitionQueue && needsLayoutTransition && !destinationCanTransitionLayout)
			{
				if(!B3D_ENSURE_LOG(mDevice.GetQueueCount(GQT_GRAPHICS) > 0, "D3D12 texture activation requires a graphics queue."))
					return;

				const GpuQueueId activationQueueId(GQT_GRAPHICS, 0);
				if(!B3D_ENSURE_LOG(
					D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(committedLayout, transition.ImageRange.AspectMask, GQT_GRAPHICS) &&
					D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(initialLayout, transition.ImageRange.AspectMask, GQT_GRAPHICS),
					"D3D12 texture layouts cannot be activated on a graphics queue."))
				{
					return;
				}

				SourceTransitionBuildInformation& sourceTransition = GetSourceTransition(activationQueueId, GpuQueueMask::kNone);
				sourceTransition.Barriers.AddTextureBarrier(image->GetD3D12Resource(), transition.ImageRange, GpuBarrierScope(), GpuImageLayout::Undefined, GpuImageLayout::Undefined, committedLayout, initialLayout);

				mSubmitInformation.RequiredWaitMask |= activationQueueId;
				transitionSourceLayout = initialLayout;

				// Set new transition queue
				layoutTransitionQueueId = activationQueueId;
				layoutTransitionQueueChanged = true;
			}
			else if(committedLayout != initialLayout)
				mSubmitInformation.RequiredWaitMask |= transition.ExclusiveAccessWaitMask;
			else
				mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;

			// Record any remaining transition in the destination submission prologue.
			if(transitionSourceLayout != initialLayout)
			{
				const GpuBarrierScope barrier(GpuStageFlag::None, GpuAccessFlag::None, transition.DestinationFirstAccessScope.GetStages(), transition.DestinationFirstAccessScope.GetAccess());
				mDestinationBarriers.AddTextureBarrier(image->GetD3D12Resource(), transition.ImageRange, barrier, GpuImageLayout::Undefined, transition.InitialLayout, transitionSourceLayout, initialLayout);

				layoutTransitionQueueId = mDestinationQueueId;
				layoutTransitionQueueChanged = true;
			}

			if(initialLayout != finalLayout)
			{
				layoutTransitionQueueId = mDestinationQueueId;
				layoutTransitionQueueChanged = true;
			}

			// Publish the native state used by later submissions.
			subresource->SetLayout(finalLayout);

			if(layoutTransitionQueueChanged)
				subresource->SetLayoutTransitionQueueId(layoutTransitionQueueId);
		}

		/**
		 * Records all barriers accumulated by the visits into freshly created command buffers and stores them on the
		 * submit information. Must be called once every transition has been visited.
		 */
		void Finalize()
		{
			auto fnRecordBarriers = [this](GpuQueueId queueId, const D3D12BarrierBatch& barriers, const StringView& name)
			{
				GpuCommandBufferPool& commandBufferPool = mDevice.GetSubmitThread().GetCommandBufferPool(queueId.GetType());
				const TShared<D3D12GpuCommandBuffer> commandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(commandBufferPool.Create(GpuCommandBufferCreateInformation::Create(name)));

				barriers.Record(*commandBuffer->GetD3D12Handle());
				commandBuffer->End();

				return commandBuffer;
			};

			for(SourceTransitionBuildInformation& transition : mSourceTransitions)
			{
				if(transition.Barriers.IsEmpty())
					continue;

				D3D12SourceQueueTransition sourceTransition;
				sourceTransition.QueueId = transition.QueueId;
				sourceTransition.WaitMask = transition.WaitMask;
				sourceTransition.CommandBuffer = fnRecordBarriers(transition.QueueId, transition.Barriers, "Source queue resource transitions");

				mSubmitInformation.SourceQueueTransitions.Add(std::move(sourceTransition));
			}

			if(!mDestinationBarriers.IsEmpty())
				mSubmitInformation.TransitionCommandBuffer = fnRecordBarriers(mDestinationQueueId, mDestinationBarriers, "Submission resource transitions");
		}

	private:
		/** Adds waits for earlier submissions on other queues. Same-queue ECL boundaries require no access barrier. */
		void AddParallelAccessWait(const GpuSubmissionTransition& transition)
		{
			mSubmitInformation.RequiredWaitMask |= transition.ParallelAccessWaitMask;
		}

		struct SourceTransitionBuildInformation
		{
			GpuQueueId QueueId;                            /**< Queue recording the release barriers. */
			GpuQueueMask WaitMask = GpuQueueMask::kNone;   /**< Queues the release must wait for. */
			D3D12BarrierBatch Barriers;                    /**< Native release barriers to record. */
		};

		/**
		 * Returns the entry accumulating the barriers to record on the provided queue, creating it if this is the
		 * first transition released from that queue. @p waitMask is merged into the entry's own wait mask.
		 */
		SourceTransitionBuildInformation& GetSourceTransition(GpuQueueId queueId, GpuQueueMask waitMask)
		{
			auto sourceTransitionIterator = std::find_if(mSourceTransitions.begin(), mSourceTransitions.end(), [queueId](const SourceTransitionBuildInformation& entry) { return entry.QueueId.Id == queueId.Id; });
			if(sourceTransitionIterator == mSourceTransitions.end())
			{
				SourceTransitionBuildInformation transition;
				transition.QueueId = queueId;
				transition.WaitMask = waitMask & ~GpuQueueMask(queueId);

				mSourceTransitions.Add(std::move(transition));
				return mSourceTransitions.back();
			}

			sourceTransitionIterator->WaitMask |= waitMask & ~GpuQueueMask(queueId);
			return *sourceTransitionIterator;
		}

		D3D12GpuDevice& mDevice;
		GpuQueueId mDestinationQueueId;
		D3D12GpuCommandBufferSubmitInformation& mSubmitInformation;
		D3D12BarrierBatch mDestinationBarriers;
		TInlineArray<SourceTransitionBuildInformation, 4> mSourceTransitions;
	};
}
D3D12GpuCommandBufferSubmitInformation D3D12GpuCommandBuffer::PrepareForSubmitOnSubmitThread(GpuQueueType queueType, u32 queueIndex)
{
	AssertIfNotSubmitThread();
	B3D_ASSERT(IsSubmitted());

	D3D12GpuCommandBufferSubmitInformation submitInformation;
	D3D12GpuDevice& device = GetD3D12GpuDevice();
	const GpuQueueId destinationQueueId(queueType, queueIndex);

	D3D12SubmissionTransitionVisitor visitor(device, destinationQueueId, submitInformation);
	mResourceTracker.ResolveSubmissionTransitions(destinationQueueId, visitor);
	mResourceTracker.ResolveBufferPageSubmissionTransitions(destinationQueueId, visitor); // TODO - Merge this into ResolveSubmissionTransitions?
	visitor.Finalize();

	submitInformation.PrimaryCommandBuffer = std::static_pointer_cast<D3D12GpuCommandBuffer>(GetShared());
	return submitInformation;
}


void D3D12GpuCommandBuffer::IssueBarriers(const GpuBarriers& barriers)
{
	EnsureValidThread();

	if(!B3D_ENSURE(!IsInRenderPass()))
		return;

	for(const auto& barrier : barriers.BufferBarriers)
	{
		D3D12GpuBuffer* const gpuBuffer = static_cast<D3D12GpuBuffer*>(barrier.Object.get());
		if(gpuBuffer == nullptr || gpuBuffer->GetD3D12Buffer() == nullptr)
			continue;

		D3D12Buffer* const buffer = gpuBuffer->GetD3D12Buffer();

		if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
			mBarrierHelper.AddBufferBarrier(buffer, barrier.DestinationUsage, barrier.DestinationAccess);
		else
			mBarrierHelper.AddBufferBarrier(buffer, barrier.SourceUsage, barrier.SourceAccess, barrier.DestinationUsage, barrier.DestinationAccess);

	}

	for(const auto& barrier : barriers.TextureBarriers)
	{
		D3D12Texture* const d3d12Texture = static_cast<D3D12Texture*>(barrier.Object.get());
		if(d3d12Texture == nullptr || d3d12Texture->GetD3D12Image() == nullptr)
			continue;

		D3D12Image* const image = d3d12Texture->GetD3D12Image();

		GpuTextureSubresourceRange maskedRange = barrier.SubresourceRange;
		maskedRange.AspectMask &= image->GetRange().AspectMask;

		if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
			mBarrierHelper.AddImageBarrier(image, maskedRange, barrier.DestinationUsage, barrier.DestinationAccess, barrier.DestinationLayout);
		else
			mBarrierHelper.AddImageBarrier(image, maskedRange, barrier.SourceUsage, barrier.SourceAccess, barrier.DestinationUsage, barrier.DestinationAccess, barrier.SourceLayout, barrier.DestinationLayout);
	}

	for(const auto& barrier : barriers.RenderTargetBarriers)
	{
		if(barrier.Object == nullptr)
			continue;

		// Resolve the render target's framebuffer, which carries the per-attachment image references. This is the
		// only way to reach swap-chain back buffers, which have no standalone D3D12Texture wrapper.
		D3D12Framebuffer* framebuffer = nullptr;
		if(barrier.Object->GetProperties().IsWindow)
		{
			const RenderWindow* const renderWindow = static_cast<const RenderWindow*>(barrier.Object.get());
			const TShared<IRenderWindowSurface>& surface = renderWindow->GetRenderWindowSurface();
			if(surface != nullptr)
			{
				D3D12RenderWindowSurface* d3d12Surface = static_cast<D3D12RenderWindowSurface*>(surface.get());
				D3D12SwapChain* swapChain = d3d12Surface->GetSwapChain();
				if(swapChain != nullptr)
					framebuffer = d3d12Surface->GetFramebuffer(swapChain->GetCurrentBackBufferIndex());
			}
		}
		else
		{
			D3D12RenderTexture* const renderTexture = static_cast<D3D12RenderTexture*>(barrier.Object.get());
			framebuffer = renderTexture->GetFramebuffer();
		}

		if(framebuffer == nullptr)
			continue;

		auto fnAddAttachmentBarrier = [this, &barrier](const D3D12FramebufferAttachment& attachment)
		{
			if(attachment.Image == nullptr)
				return;

			const GpuTextureSubresourceRange range = attachment.Image->GetRange(attachment.Surface);

			if(barrier.SourceUsage == GpuResourceUseFlag::Undefined)
				mBarrierHelper.AddImageBarrier(attachment.Image, range, barrier.DestinationUsage, barrier.DestinationAccess, barrier.DestinationLayout);
			else
				mBarrierHelper.AddImageBarrier(attachment.Image, range, barrier.SourceUsage, barrier.SourceAccess, barrier.DestinationUsage, barrier.DestinationAccess, barrier.SourceLayout, barrier.DestinationLayout);
		};

		for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
		{
			const RenderSurfaceMaskBits colorMask = static_cast<RenderSurfaceMaskBits>(RT_COLOR0 << colorIndex);
			if(barrier.SurfaceMask == colorMask)
			{
				fnAddAttachmentBarrier(framebuffer->GetColorAttachment(colorIndex));
				break;
			}
		}

		if(barrier.SurfaceMask == RT_DEPTH || barrier.SurfaceMask == RT_STENCIL)
			fnAddAttachmentBarrier(framebuffer->GetDepthStencilAttachment());
	}

	mBarrierHelper.Execute(*this);
}