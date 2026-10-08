//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuSubmissionTimeline.h"

using namespace b3d;

GpuSubmissionTimeline::Submission::Submission(GpuQueueId queue)
	: Queue(queue)
{
	for(i32& covered : Covered)
		covered = kNotCovered;
}

GpuSubmissionTimeline::GpuSubmissionTimeline()
{
	Clear();
}

u32 GpuSubmissionTimeline::AddSubmission(GpuQueueId queue, GpuQueueMask waitQueues)
{
	const u32 submissionIndex = (u32)mSubmissions.Size();
	Submission submission(queue);

	auto fnCover = [&submission](const Submission& other)
	{
		for(u32 queueId = 0; queueId < B3D_MAX_UNIQUE_QUEUES; queueId++)
			submission.Covered[queueId] = std::max(submission.Covered[queueId], other.Covered[queueId]);
	};

	// Everything that was ordered on the last submission on the same queue is also ordered against this one
	if(mLatestSubmissions[queue.Id] >= 0)
		fnCover(mSubmissions[mLatestSubmissions[queue.Id]]);

	for(u32 queueId = 0; queueId < B3D_MAX_UNIQUE_QUEUES; queueId++)
	{
		if(queueId == queue.Id || !waitQueues.IsSet(GpuQueueId(queueId)))
			continue;

		const i32 latestSubmission = mLatestSubmissions[queueId];
		if(latestSubmission >= 0)
			fnCover(mSubmissions[latestSubmission]);
	}

	submission.Covered[queue.Id] = (i32)submissionIndex;

	mSubmissions.Add(submission);
	mLatestSubmissions[queue.Id] = (i32)submissionIndex;

	return submissionIndex;
}

bool GpuSubmissionTimeline::IsOrderedBefore(u32 earlier, u32 later) const
{
	B3D_ASSERT(earlier < mSubmissions.Size() && later < mSubmissions.Size());

	const GpuQueueId earlierQueue = mSubmissions[earlier].Queue;
	return mSubmissions[later].Covered[earlierQueue.Id] >= (i32)earlier;
}

void GpuSubmissionTimeline::Clear()
{
	mSubmissions.Clear();

	for(i32& latestSubmission : mLatestSubmissions)
		latestSubmission = -1;
}
