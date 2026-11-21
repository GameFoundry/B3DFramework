//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * Tests for the skin cache, on a procedural mesh: a column of rings skinned to three bones, with up to four bones per vertex, and two
	 * morph shapes that overlap on some rings. The deformed vertices are read back and compared with a CPU implementation of the same math.
	 */
	class SkinCacheTestSuite : public TestSuite
	{
	public:
		SkinCacheTestSuite();

	private:
		/**
		 * Deforms an entry to the bind pose: no vertex data before the first update, positions equal to the mesh's, normals and tangents
		 * within 1/255 of the mesh's normalized ones, and the tangents' handedness kept.
		 */
		void TestBindPose();

		/**
		 * Deforms an entry to fixed bone transforms: positions within 1e-5 of the CPU reference, normals and tangents within 1/255, and the
		 * tangents' handedness kept.
		 */
		void TestSkinning();

		/**
		 * Deforms an entry of a mesh with morph shapes and no skeleton to all-zero, positive, negative and zero weights, on vertices that no
		 * shape moves, that one shape moves and that two shapes move: positions within 1e-5 of the CPU reference, normals and tangents
		 * within 1/255.
		 */
		void TestMorph();

		/**
		 * Builds morph shape deltas from per-shape vertex lists and compares their vertex ranges and sorted deltas with the CPU reference.
		 * Creates a mesh with the deltas: it has morph shape buffers once initialized, and keeps no copy of the deltas unless it has
		 * MeshFlag::KeepCPUCopy. Saves the mesh and loads it again: the loaded mesh's entry deforms like TestMorph() expects.
		 */
		void TestMorphShapeDeltas();

		/**
		 * Deforms entries of a mesh with a skeleton and morph shapes, of one with only a skeleton and of one with only morph shapes in one
		 * update, each with its own shader variation, and compares them with the CPU reference.
		 */
		void TestMorphAndSkinning();

		/**
		 * Deforms an entry that keeps previous positions through several poses and checks which buffers its vertex data binds as POSITION0
		 * and POSITION1, and what they hold: one buffer for both after the first deformation, a second buffer after the second, swapped
		 * roles after the third, the current buffer for both after an update without a new pose, and the right previous positions after
		 * the next pose.
		 */
		void TestPreviousPositions();

		/**
		 * Checks that an update adds a pass only for changed poses: the same pose twice adds none, a changed morph shape weight alone adds
		 * one. Checks that poses with the wrong number of bones or weights are rejected, and which meshes get no entry.
		 */
		void TestChangeDetection();

		/**
		 * Checks the entry's vertex description: POSITION0, NORMAL0 and TANGENT0 resolve to the entry's streams, TEXCOORD0 to the mesh's,
		 * and the mesh's stream keeps its stride. Draws the deformed mesh in the graph that deformed it and probes pixels that only the
		 * deformed shape covers.
		 */
		void TestVertexLayout();

		/**
		 * Deforms an entry with ray tracing geometry and previous positions through several poses, with a ray tracing scene that has an
		 * instance of the entry's geometry. Each update adds a pass that builds the geometry after the deformation, and the scene rebuilds
		 * its top level structure because the geometry's contents version changed, and only then. Inline rays hit the deformed surface,
		 * and read the deformed positions and normals and the mesh's texture coordinates through the geometry's layouts, also after the
		 * position buffers swapped. On devices without acceleration structures the entry has no geometry and builds nothing.
		 */
		void TestRayTracingGeometry();

		/**
		 * Deforms two entries, one of them with previous positions, in one graph: one pass, not culled, and no validation errors. A later
		 * graph draws the exported buffers at rest without declaring them, and a third deforms them again after that draw.
		 */
		void TestFrameGraph();
	};
} // namespace b3d
