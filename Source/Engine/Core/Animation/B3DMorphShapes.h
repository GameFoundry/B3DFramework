//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Script/B3DIScriptExportable.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Animation
	 *  @{
	 */

	/**
	 * A single vertex moved by a morph shape: the difference between the base mesh and the shape, and the index of the vertex in the mesh.
	 * Input of MorphShapeDeltas::Create().
	 */
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
	 * Name and weight of a single shape in a morph target animation. The vertices the shape moves are part of the mesh's
	 * MorphShapeDeltas.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) MorphShape : public IReflectable, public IScriptExportable
	{
	public:
		MorphShape(const String& name, float weight);

		/** Returns the name of the shape. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Name))
		const String& GetName() const { return mName; }

		/** Returns the weight of the shape, determining how are different shapes within a channel blended. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Weight))
		float GetWeight() const { return mWeight; }

		/**
		 * Creates a new morph shape.
		 *
		 * @param	name		Name of the frame. Must be unique within a morph channel.
		 * @param	weight		Weight in range [0, 1]. Determines how are sequential shapes animated between within a
		 *						morph channel. e.g. if there is a shape A with weight 0.3 and shape B with weight 0.8
		 *						then shape A will be displayed first and then be blended towards shape B as time passes.
		 */
		static TShared<MorphShape> Create(const String& name, float weight);

	private:
		String mName;
		float mWeight;

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
		TShared<MorphShape> GetShape(u32 index) const { return mShapes[index]; }

		/** Returns all morph shapes within this channel, in order from lowest to highest. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Shapes))
		const Vector<TShared<MorphShape>>& GetShapes() const { return mShapes; }

		/** Creates a new channel from a set of morph shapes. */
		static TShared<MorphChannel> Create(const String& name, const Vector<TShared<MorphShape>>& shapes);

	private:
		MorphChannel() = default;
		MorphChannel(const String& name, const Vector<TShared<MorphShape>>& shapes);

		String mName;
		Vector<TShared<MorphShape>> mShapes;

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
		static TShared<MorphChannel> CreateEmpty();
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
		TShared<MorphChannel> GetChannel(u32 index) const { return mChannels[index]; }

		/** Returns a list of all morph channels in the morph animation. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Channels))
		const Vector<TShared<MorphChannel>>& GetChannels() const { return mChannels; }

		/** Returns the number of vertices per morph shape. */
		u32 GetVertexCount() const { return mVertexCount; }

		/** Returns the number of morph shapes, in all channels. */
		u32 GetShapeCount() const;

		/** Creates a new set of morph shapes. */
		static TShared<MorphShapes> Create(const Vector<TShared<MorphChannel>>& channels, u32 numVertices);

	private:
		MorphShapes() = default;
		MorphShapes(const Vector<TShared<MorphChannel>>& channels, u32 numVertices);

		Vector<TShared<MorphChannel>> mChannels;
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
		static TShared<MorphShapes> CreateEmpty();
	};

	/**
	 * The vertex deltas of all morph shapes of a mesh, in the layout the GPU reads them in: the index of each vertex's first delta, then the
	 * deltas sorted by vertex. Shapes are numbered in the order of MorphShapes: its channels, then the shapes within a channel. A mesh hands
	 * them to the GPU when it is initialized (see MeshCreateInformation::MorphShapeDeltas).
	 */
	class B3D_EXPORT MorphShapeDeltas : public IReflectable
	{
	public:
		/** Size of a delta, in bytes: a position delta and a normal delta of three floats each, then the index of the shape as a u32. */
		static constexpr u32 kDeltaSize = 28;

		/** Returns the number of vertices of the mesh. */
		u32 GetVertexCount() const { return mVertexRanges.empty() ? 0 : (u32)mVertexRanges.size() - 1; }

		/** Returns the number of deltas, of all shapes. */
		u32 GetDeltaCount() const { return (u32)mDeltas.size() / kDeltaSize; }

		/** Returns the number of shapes. */
		u32 GetShapeCount() const { return mShapeCount; }

		/**
		 * Returns the index of each vertex's first delta in GetDeltas(), with one more value at the end: the number of deltas. A vertex's
		 * deltas are [ranges[v], ranges[v + 1]).
		 */
		TArrayView<const u32> GetVertexRanges() const { return mVertexRanges; }

		/** @copydoc GetVertexRanges() const */
		TArrayView<u32> GetVertexRanges() { return mVertexRanges; }

		/** Returns the deltas, sorted by vertex, and by shape within a vertex. kDeltaSize bytes each. */
		TArrayView<const u8> GetDeltas() const { return mDeltas; }

		/** @copydoc GetDeltas() const */
		TArrayView<u8> GetDeltas() { return mDeltas; }

		/**
		 * Creates the deltas of a mesh with @p vertexCount vertices from the vertices each shape moves: one list per shape in
		 * @p shapeVertices, in the order of the mesh's MorphShapes. A vertex a shape moves outside the mesh is skipped, with an error.
		 */
		static TShared<MorphShapeDeltas> Create(u32 vertexCount, TArrayView<const Vector<MorphVertex>> shapeVertices);

		/** Creates zeroed deltas of the given counts, for the caller to fill in. */
		static TShared<MorphShapeDeltas> Create(u32 vertexCount, u32 deltaCount, u32 shapeCount);

	private:
		MorphShapeDeltas() = default;
		MorphShapeDeltas(u32 vertexCount, u32 deltaCount, u32 shapeCount);

		Vector<u32> mVertexRanges;
		Vector<u8> mDeltas;
		u32 mShapeCount = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MorphShapeDeltasRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
