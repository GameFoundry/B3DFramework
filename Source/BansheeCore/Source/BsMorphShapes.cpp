//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMorphShapes.h"
#include "BsMorphShapesRTTI.h"

namespace BansheeEngine
{
	MorphShape::MorphShape()
	{ }

	MorphShape::MorphShape(const String& name, float weight, const Vector<MorphVertex>& vertices)
		:mName(name), mWeight(weight), mVertices(vertices)
	{ }

	/** Creates a new morph shape from the provided set of vertices. */
	SPtr<MorphShape> MorphShape::create(const String& name, float weight, const Vector<MorphVertex>& vertices)
	{
		return bs_shared_ptr_new<MorphShape>(name, weight, vertices);
	}

	RTTITypeBase* MorphShape::getRTTIStatic()
	{
		return MorphShapeRTTI::instance();
	}

	RTTITypeBase* MorphShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	MorphChannel::MorphChannel()
	{ }

	MorphChannel::MorphChannel(const String& name, const Vector<SPtr<MorphShape>>& shapes)
		:mName(name), mShapes(shapes)
	{
		std::sort(mShapes.begin(), mShapes.end(), 
			[](auto& x, auto& y)
		{
			return x->getWeight() < y->getWeight();
		});
	}

	SPtr<MorphChannel> MorphChannel::create(const String& name, const Vector<SPtr<MorphShape>>& shapes)
	{
		MorphChannel* raw = new (bs_alloc<MorphChannel>()) MorphChannel(name, shapes);
		return bs_shared_ptr(raw);
	}

	SPtr<MorphChannel> MorphChannel::createEmpty()
	{
		MorphChannel* raw = new (bs_alloc<MorphChannel>()) MorphChannel();
		return bs_shared_ptr(raw);
	}

	RTTITypeBase* MorphChannel::getRTTIStatic()
	{
		return MorphChannelRTTI::instance();
	}

	RTTITypeBase* MorphChannel::getRTTI() const
	{
		return getRTTIStatic();
	}

	MorphShapes::MorphShapes()
	{ }

	MorphShapes::MorphShapes(const Vector<SPtr<MorphChannel>>& channels, UINT32 numVertices)
		:mChannels(channels), mNumVertices(numVertices)
	{

	}

	SPtr<MorphShapes> MorphShapes::create(const Vector<SPtr<MorphChannel>>& channels, UINT32 numVertices)
	{
		MorphShapes* raw = new (bs_alloc<MorphShapes>()) MorphShapes(channels, numVertices);
		return bs_shared_ptr(raw);
	}

	SPtr<MorphShapes> MorphShapes::createEmpty()
	{
		MorphShapes* raw = new (bs_alloc<MorphShapes>()) MorphShapes();
		return bs_shared_ptr(raw);
	}

	RTTITypeBase* MorphShapes::getRTTIStatic()
	{
		return MorphShapesRTTI::instance();
	}

	RTTITypeBase* MorphShapes::getRTTI() const
	{
		return getRTTIStatic();
	}
}