//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DMorphShapes.h"
#include "RTTI/B3DMorphShapesRTTI.h"

using namespace b3d;

MorphShape::MorphShape(const String& name, float weight, const Vector<MorphVertex>& vertices)
	: mName(name), mWeight(weight), mVertices(vertices)
{}

/** Creates a new morph shape from the provided set of vertices. */
SPtr<MorphShape> MorphShape::Create(const String& name, float weight, const Vector<MorphVertex>& vertices)
{
	return B3DMakeShared<MorphShape>(name, weight, vertices);
}

RTTIType* MorphShape::GetRttiStatic()
{
	return MorphShapeRTTI::Instance();
}

RTTIType* MorphShape::GetRtti() const
{
	return GetRttiStatic();
}

MorphChannel::MorphChannel(const String& name, const Vector<SPtr<MorphShape>>& shapes)
	: mName(name), mShapes(shapes)
{
	std::sort(mShapes.begin(), mShapes.end(), [](auto& x, auto& y)
			  { return x->GetWeight() < y->GetWeight(); });
}

SPtr<MorphChannel> MorphChannel::Create(const String& name, const Vector<SPtr<MorphShape>>& shapes)
{
	MorphChannel* raw = new(B3DAllocate<MorphChannel>()) MorphChannel(name, shapes);
	return B3DMakeSharedFromExisting(raw);
}

SPtr<MorphChannel> MorphChannel::CreateEmpty()
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

MorphShapes::MorphShapes(const Vector<SPtr<MorphChannel>>& channels, u32 numVertices)
	: mChannels(channels), mVertexCount(numVertices)
{
}

SPtr<MorphShapes> MorphShapes::Create(const Vector<SPtr<MorphChannel>>& channels, u32 numVertices)
{
	MorphShapes* raw = new(B3DAllocate<MorphShapes>()) MorphShapes(channels, numVertices);
	return B3DMakeSharedFromExisting(raw);
}

SPtr<MorphShapes> MorphShapes::CreateEmpty()
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
