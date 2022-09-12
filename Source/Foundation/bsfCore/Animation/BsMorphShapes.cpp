//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsMorphShapes.h"
#include "Private/RTTI/BsMorphShapesRTTI.h"

namespace bs
{
	MorphShape::MorphShape(const String& name, float weight, const Vector<MorphVertex>& vertices)
		:mName(name), mWeight(weight), mVertices(vertices)
	{ }

	/** Creates a new morph shape from the provided set of vertices. */
	SPtr<MorphShape> MorphShape::Create(const String& name, float weight, const Vector<MorphVertex>& vertices)
	{
		return bs_shared_ptr_new<MorphShape>(name, weight, vertices);
	}

	RTTITypeBase* MorphShape::getRTTIStatic()
	{
		return MorphShapeRTTI::Instance();
	}

	RTTITypeBase* MorphShape::getRTTI() const
	{
		return GetRTTIStatic();
	}

	MorphChannel::MorphChannel(const String& name, const Vector<SPtr<MorphShape>>& shapes)
		:mName(name), mShapes(shapes)
	{
		std::sort(mShapes.begin(), mShapes.end(),
			[](auto& x, auto& y)
		{
			return x->GetWeight() < y->getWeight();
		});
	}

	SPtr<MorphChannel> MorphChannel::Create(const String& name, const Vector<SPtr<MorphShape>>& shapes)
	{
		MorphChannel* raw = new (bs_alloc<MorphChannel>()) MorphChannel(name, shapes);
		return bs_shared_ptr(raw);
	}

	SPtr<MorphChannel> MorphChannel::CreateEmpty()
	{
		MorphChannel* raw = new (bs_alloc<MorphChannel>()) MorphChannel();
		return bs_shared_ptr(raw);
	}

	RTTITypeBase* MorphChannel::getRTTIStatic()
	{
		return MorphChannelRTTI::Instance();
	}

	RTTITypeBase* MorphChannel::getRTTI() const
	{
		return GetRTTIStatic();
	}

	MorphShapes::MorphShapes(const Vector<SPtr<MorphChannel>>& channels, UINT32 numVertices)
		:mChannels(channels), mNumVertices(numVertices)
	{

	}

	SPtr<MorphShapes> MorphShapes::Create(const Vector<SPtr<MorphChannel>>& channels, UINT32 numVertices)
	{
		MorphShapes* raw = new (bs_alloc<MorphShapes>()) MorphShapes(channels, numVertices);
		return bs_shared_ptr(raw);
	}

	SPtr<MorphShapes> MorphShapes::CreateEmpty()
	{
		MorphShapes* raw = new (bs_alloc<MorphShapes>()) MorphShapes();
		return bs_shared_ptr(raw);
	}

	RTTITypeBase* MorphShapes::getRTTIStatic()
	{
		return MorphShapesRTTI::Instance();
	}

	RTTITypeBase* MorphShapes::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
