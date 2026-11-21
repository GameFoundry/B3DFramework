//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DMorphShapes.h"
#include "RTTI/B3DMorphShapesRTTI.h"

using namespace b3d;

MorphShape::MorphShape(const String& name, float weight)
	: mName(name), mWeight(weight)
{}

TShared<MorphShape> MorphShape::Create(const String& name, float weight)
{
	return B3DMakeShared<MorphShape>(name, weight);
}

RTTIType* MorphShape::GetRttiStatic()
{
	return MorphShapeRTTI::Instance();
}

RTTIType* MorphShape::GetRtti() const
{
	return GetRttiStatic();
}

MorphChannel::MorphChannel(const String& name, const Vector<TShared<MorphShape>>& shapes)
	: mName(name), mShapes(shapes)
{
	std::sort(mShapes.begin(), mShapes.end(), [](auto& x, auto& y)
			  { return x->GetWeight() < y->GetWeight(); });
}

TShared<MorphChannel> MorphChannel::Create(const String& name, const Vector<TShared<MorphShape>>& shapes)
{
	MorphChannel* raw = new(B3DAllocate<MorphChannel>()) MorphChannel(name, shapes);
	return B3DMakeSharedFromExisting(raw);
}

TShared<MorphChannel> MorphChannel::CreateEmpty()
{
	MorphChannel* raw = new(B3DAllocate<MorphChannel>()) MorphChannel();
	return B3DMakeSharedFromExisting(raw);
}

RTTIType* MorphChannel::GetRttiStatic()
{
	return MorphChannelRTTI::Instance();
}

RTTIType* MorphChannel::GetRtti() const
{
	return GetRttiStatic();
}

MorphShapes::MorphShapes(const Vector<TShared<MorphChannel>>& channels, u32 numVertices)
	: mChannels(channels), mVertexCount(numVertices)
{
}

u32 MorphShapes::GetShapeCount() const
{
	u32 shapeCount = 0;
	for(const TShared<MorphChannel>& channel : mChannels)
		shapeCount += channel->GetShapeCount();

	return shapeCount;
}

TShared<MorphShapes> MorphShapes::Create(const Vector<TShared<MorphChannel>>& channels, u32 numVertices)
{
	MorphShapes* raw = new(B3DAllocate<MorphShapes>()) MorphShapes(channels, numVertices);
	return B3DMakeSharedFromExisting(raw);
}

TShared<MorphShapes> MorphShapes::CreateEmpty()
{
	MorphShapes* raw = new(B3DAllocate<MorphShapes>()) MorphShapes();
	return B3DMakeSharedFromExisting(raw);
}

RTTIType* MorphShapes::GetRttiStatic()
{
	return MorphShapesRTTI::Instance();
}

RTTIType* MorphShapes::GetRtti() const
{
	return GetRttiStatic();
}

MorphShapeDeltas::MorphShapeDeltas(u32 vertexCount, u32 deltaCount, u32 shapeCount)
	: mVertexRanges(vertexCount + 1, 0), mDeltas(deltaCount * kDeltaSize, 0), mShapeCount(shapeCount)
{ }

TShared<MorphShapeDeltas> MorphShapeDeltas::Create(u32 vertexCount, TArrayView<const Vector<MorphVertex>> shapeVertices)
{
	// Count the deltas of each vertex, then turn the counts into the index of each vertex's first delta
	Vector<u32> vertexRanges(vertexCount + 1, 0);
	for(u32 shapeIndex = 0; shapeIndex < (u32)shapeVertices.Size(); shapeIndex++)
	{
		for(const MorphVertex& vertex : shapeVertices[shapeIndex])
		{
			if(B3D_ENSURE_LOG(vertex.SourceIdx < vertexCount, "Morph shape {0} moves vertex {1} of a mesh with {2} vertices.", shapeIndex, vertex.SourceIdx, vertexCount))
				vertexRanges[vertex.SourceIdx + 1]++;
		}
	}

	for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
		vertexRanges[vertexIndex + 1] += vertexRanges[vertexIndex];

	const TShared<MorphShapeDeltas> deltas = Create(vertexCount, vertexRanges[vertexCount], (u32)shapeVertices.Size());
	std::memcpy(deltas->GetVertexRanges().Data(), vertexRanges.data(), vertexRanges.size() * sizeof(u32));

	// Each delta goes to the next free slot of its vertex. Shapes are visited in order, so a vertex's deltas are sorted by shape.
	vertexRanges.pop_back();
	Vector<u32>& nextDeltaIndices = vertexRanges;
	for(u32 shapeIndex = 0; shapeIndex < (u32)shapeVertices.Size(); shapeIndex++)
	{
		for(const MorphVertex& vertex : shapeVertices[shapeIndex])
		{
			if(vertex.SourceIdx >= vertexCount)
				continue;

			u8* const delta = deltas->GetDeltas().Data() + nextDeltaIndices[vertex.SourceIdx]++ * kDeltaSize;
			std::memcpy(delta, &vertex.DeltaPosition, sizeof(Vector3));
			std::memcpy(delta + 12, &vertex.DeltaNormal, sizeof(Vector3));
			std::memcpy(delta + 24, &shapeIndex, sizeof(u32));
		}
	}

	return deltas;
}

TShared<MorphShapeDeltas> MorphShapeDeltas::Create(u32 vertexCount, u32 deltaCount, u32 shapeCount)
{
	return B3DMakeSharedFromExisting(new(B3DAllocate<MorphShapeDeltas>()) MorphShapeDeltas(vertexCount, deltaCount, shapeCount));
}

RTTIType* MorphShapeDeltas::GetRttiStatic()
{
	return MorphShapeDeltasRTTI::Instance();
}

RTTIType* MorphShapeDeltas::GetRtti() const
{
	return GetRttiStatic();
}
