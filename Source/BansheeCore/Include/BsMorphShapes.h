//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsVector3.h"

namespace BansheeEngine
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** A single vertex used for morph target animation. Contains a difference between base and target shape. */
	struct BS_CORE_EXPORT MorphVertex
	{
		MorphVertex() { }
		MorphVertex(const Vector3& deltaPosition, const Vector3& deltaNormal, UINT32 sourceIdx)
			:deltaPosition(deltaPosition), deltaNormal(deltaNormal), sourceIdx(sourceIdx)
		{ }

		Vector3 deltaPosition;
		Vector3 deltaNormal;
		UINT32 sourceIdx;
	};

	/** 
	 * A set of vertices representing a single shape in a morph target animation. Vertices are represented as a difference
	 * between base and target shape.
	 */
	class BS_CORE_EXPORT MorphShape : public IReflectable
	{
	public:
		MorphShape(const String& name, float weight, const Vector<MorphVertex>& vertices);

		/** Returns the name of the shape. */
		const String& getName() const { return mName; }

		/** Returns the weight of the shape, determining how are different shapes within a channel blended. */
		float getWeight() const { return mWeight; }

		/** Returns a reference to all of the shape's vertices. Contains only vertices that differ from the base. */
		const Vector<MorphVertex>& getVertices() const { return mVertices; }

		/** 
		 * Creates a new morph shape from the provided set of vertices. 
		 * 
		 * @param[in]	name		Name of the frame. Must be unique within a morph channel.
		 * @param[in]	weight		Weight in range [0, 1]. Determines how are sequential shapes animated between within a
		 *							morph channel. e.g. if there is a shape A with weight 0.3 and shape B with weight 0.8
		 *							then shape A will be displayed first and then be blended towards shape B as time passes.
		 * @param[in]	vertices	Vertices of the base mesh modified by the shape.
		 */
		static SPtr<MorphShape> create(const String& name, float weight, const Vector<MorphVertex>& vertices);

	private:
		String mName;
		float mWeight;
		Vector<MorphVertex> mVertices;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphShapeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		MorphShape(); // Serialization only
	};

	/**
	 * A collection of morph shapes that are sequentially blended together. Each shape has a weight in range [0, 1] which
	 * determines at what point is that shape blended. As the channel percent moves from 0 to 1, different shapes will be
	 * blended with those before or after them, depending on their weight.
	 */
	class BS_CORE_EXPORT MorphChannel : public IReflectable
	{
	public:
		/** Returns the name of the channel. */
		const String& getName() const { return mName; }

		/** Returns the number of available morph shapes. */
		UINT32 getNumShapes() const { return (UINT32)mShapes.size(); }

		/** Returns the morph shape at the specified index. */
		SPtr<MorphShape> getShape(UINT32 idx) const { return mShapes[idx]; }

		/** Creates a new channel from a set of morph shapes. */
		static SPtr<MorphChannel> create(const String& name, const Vector<SPtr<MorphShape>>& shapes);

	private:
		MorphChannel();
		MorphChannel(const String& name, const Vector<SPtr<MorphShape>>& shapes);

		String mName;
		Vector<SPtr<MorphShape>> mShapes;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphChannelRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		/** 
		 * Creates MorphShapes with no data. You must populate its data manually.
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<MorphChannel> createEmpty();
	};

	/** 
	 * Contains a set of morph channels used for morph target animation. Each morph channel contains one or multiple shapes
	 * which are blended together depending on frame animation. Each channel is then additively blended together depending
	 * on some weight.
	 */
	class BS_CORE_EXPORT MorphShapes : public IReflectable // Note: Must be immutable in order to be usable on multiple threads
	{
	public:
		/** Returns the number of available morph channels. */
		UINT32 getNumChannels() const { return (UINT32)mChannels.size(); }

		/** Returns the morph channel at the specified index. */
		SPtr<MorphChannel> getChannel(UINT32 idx) const { return mChannels[idx]; }

		/** Returns the number of vertices per morph shape. */
		UINT32 getNumVertices() const { return mNumVertices; }

		/** Creates a new set of morph shapes. */
		static SPtr<MorphShapes> create(const Vector<SPtr<MorphChannel>>& channels, UINT32 numVertices);

	private:
		MorphShapes();
		MorphShapes(const Vector<SPtr<MorphChannel>>& channels, UINT32 numVertices);

		Vector<SPtr<MorphChannel>> mChannels;
		UINT32 mNumVertices;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphShapesRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		/** 
		 * Creates MorphShapes with no data. You must populate its data manually.
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<MorphShapes> createEmpty();
	};

	/** @} */
}