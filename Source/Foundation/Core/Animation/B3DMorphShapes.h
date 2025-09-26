//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Script/B3DIScriptExportable.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** A single vertex used for morph target animation. Contains a difference between base and target shape. */
	struct B3D_EXPORT MorphVertex
	{
		MorphVertex() = default;

		MorphVertex(const Vector3& deltaPosition, const Vector3& deltaNormal, u32 sourceIdx)
			: DeltaPosition(deltaPosition), DeltaNormal(deltaNormal), SourceIdx(sourceIdx)
		{}

		Vector3 DeltaPosition;
		Vector3 DeltaNormal;
		u32 SourceIdx;
	};

	/**
	 * @native
	 * A set of vertices representing a single shape in a morph target animation. Vertices are represented as a difference
	 * between base and target shape.
	 * @endnative
	 * @script
	 * Name and weight of a single shape in a morph target animation. Each shape internally represents a set of vertices
	 * that describe the morph shape.
	 * @endscript
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) MorphShape : public IReflectable, public IScriptExportable
	{
	public:
		MorphShape(const String& name, float weight, const Vector<MorphVertex>& vertices);

		/** Returns the name of the shape. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Name))
		const String& GetName() const { return mName; }

		/** Returns the weight of the shape, determining how are different shapes within a channel blended. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Weight))
		float GetWeight() const { return mWeight; }

		/** Returns a reference to all of the shape's vertices. Contains only vertices that differ from the base. */
		const Vector<MorphVertex>& GetVertices() const { return mVertices; }

		/**
		 * Creates a new morph shape from the provided set of vertices.
		 *
		 * @param[in]	name		Name of the frame. Must be unique within a morph channel.
		 * @param[in]	weight		Weight in range [0, 1]. Determines how are sequential shapes animated between within a
		 *							morph channel. e.g. if there is a shape A with weight 0.3 and shape B with weight 0.8
		 *							then shape A will be displayed first and then be blended towards shape B as time passes.
		 * @param[in]	vertices	Vertices of the base mesh modified by the shape.
		 */
		static SPtr<MorphShape> Create(const String& name, float weight, const Vector<MorphVertex>& vertices);

	private:
		String mName;
		float mWeight;
		Vector<MorphVertex> mVertices;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

		MorphShape() = default; // Serialization only
	};

	/**
	 * A collection of morph shapes that are sequentially blended together. Each shape has a weight in range [0, 1] which
	 * determines at what point is that shape blended. As the channel percent moves from 0 to 1, different shapes will be
	 * blended with those before or after them, depending on their weight.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) MorphChannel : public IReflectable, public IScriptExportable
	{
	public:
		/** Returns the unique name of the channel. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Name))
		const String& GetName() const { return mName; }

		/** Returns the number of available morph shapes. */
		u32 GetShapeCount() const { return (u32)mShapes.size(); }

		/** Returns the morph shape at the specified index. */
		SPtr<MorphShape> GetShape(u32 index) const { return mShapes[index]; }

		/** Returns all morph shapes within this channel, in order from lowest to highest. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Shapes))
		const Vector<SPtr<MorphShape>>& GetShapes() const { return mShapes; }

		/** Creates a new channel from a set of morph shapes. */
		static SPtr<MorphChannel> Create(const String& name, const Vector<SPtr<MorphShape>>& shapes);

	private:
		MorphChannel() = default;
		MorphChannel(const String& name, const Vector<SPtr<MorphShape>>& shapes);

		String mName;
		Vector<SPtr<MorphShape>> mShapes;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphChannelRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

		/**
		 * Creates MorphShapes with no data. You must populate its data manually.
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<MorphChannel> CreateEmpty();
	};

	/**
	 * Contains a set of morph channels used for morph target animation. Each morph channel contains one or multiple shapes
	 * which are blended together depending on frame animation. Each channel is then additively blended together depending
	 * on some weight.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) MorphShapes : public IReflectable, public IScriptExportable // Note: Must be immutable in order to be usable on multiple threads
	{
	public:
		/** Returns the number of available morph channels. */
		u32 GetChannelCount() const { return (u32)mChannels.size(); }

		/** Returns the morph channel at the specified index. */
		SPtr<MorphChannel> GetChannel(u32 index) const { return mChannels[index]; }

		/** Returns a list of all morph channels in the morph animation. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Channels))
		const Vector<SPtr<MorphChannel>>& GetChannels() const { return mChannels; }

		/** Returns the number of vertices per morph shape. */
		u32 GetVertexCount() const { return mVertexCount; }

		/** Creates a new set of morph shapes. */
		static SPtr<MorphShapes> Create(const Vector<SPtr<MorphChannel>>& channels, u32 numVertices);

	private:
		MorphShapes() = default;
		MorphShapes(const Vector<SPtr<MorphChannel>>& channels, u32 numVertices);

		Vector<SPtr<MorphChannel>> mChannels;
		u32 mVertexCount;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphShapesRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

		/**
		 * Creates MorphShapes with no data. You must populate its data manually.
		 *
		 * @note	For serialization use only.
		 */
		static SPtr<MorphShapes> CreateEmpty();
	};

	/** @} */
} // namespace b3d
