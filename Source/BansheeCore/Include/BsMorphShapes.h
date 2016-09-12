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
		MorphShape(const String& name, const Vector<MorphVertex>& vertices);

		/** Returns the name of the shape. */
		const String& getName() const { return mName; }

		/** Returns a reference to all of the shape's vertices. Contains only vertices that differ from the base. */
		const Vector<MorphVertex>& getVertices() const { return mVertices; }

		/** Creates a new morph shape from the provided set of vertices. */
		static SPtr<MorphShape> create(const String& name, const Vector<MorphVertex>& vertices);

	private:
		String mName;
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
	 * Contains a set of morph shapes, used for morph target animation. Each morph shape contains a single possible shape
	 * that can be added on top of the base shape in order to create the animation.
	 */
	class BS_CORE_EXPORT MorphShapes : public IReflectable // Note: Must be immutable in order to be usable on multiple threads
	{
	public:
		/** Returns the number of available morph shapes. */
		UINT32 getNumShapes() const { return (UINT32)mShapes.size(); }

		/** Returns the morph shape at the specified index. */
		SPtr<MorphShape> getShape(UINT32 idx) const { return mShapes[idx]; }

		/** Creates a new set of morph shapes. */
		static SPtr<MorphShapes> create(const Vector<SPtr<MorphShape>>& shapes);

	private:
		MorphShapes();
		MorphShapes(const Vector<SPtr<MorphShape>>& shapes);

		Vector<SPtr<MorphShape>> mShapes;

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