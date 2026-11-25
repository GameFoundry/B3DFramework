//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests for acceleration structures and ray tracing. On devices without the capabilities, the tests verify the errors instead. */
	class GpuRayTracingTestSuite : public TestSuite
	{
	public:
		GpuRayTracingTestSuite();

	private:
		/**
		 * Verifies acceleration structure creation: the creation errors, the type, size and bindless slot, the resting state, barriers in
		 * and out of an acceleration structure build, the stages that cannot access structures or buffers, and the rules for build input
		 * buffers. Without RSC_ACCELERATION_STRUCTURES, verifies that creation fails.
		 */
		void TestAccelerationStructureCreation();

		/**
		 * Verifies the size query and the instance writes, and the errors of invalid inputs, instances and builds. Without
		 * RSC_ACCELERATION_STRUCTURES, verifies that the queries, instance writes and builds fail.
		 */
		void TestAccelerationStructureQueries();

		/**
		 * Builds bottom level structures of known triangles and top level structures of their instances, and traces rays against them
		 * inline, with the top level structure bound to a set and taken from the bindless heap. Checks distances, barycentrics,
		 * instance, geometry and primitive indices, facing, instance masks, ray and instance flags, any-hit callbacks, and that a top
		 * level structure keeps its bottom levels alive. Runs only with RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY.
		 */
		void TestInlineRayTracing();

		/**
		 * Writes top level instances with RayTracingInstances.bslinc from a compute shader, and checks that they match what
		 * GpuDevice::WriteRayTracingInstances() writes for the same instances byte for byte: every valid flag combination, empty
		 * instances, the largest ids and hit group offsets, several masks and affine transforms. Then builds a top level structure from
		 * each, and checks that rays hit the same instances in both. Also checks the errors of
		 * GpuDevice::WriteAccelerationStructureReferences(). Runs only with RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY.
		 */
		void TestGpuWrittenInstances();

		/**
		 * Verifies the creation errors of ray tracing pipelines and shader binding tables, the payload and attribute sizes of a pipeline, and
		 * the errors of DispatchRays(). Without RSC_RAY_TRACING_PIPELINE, verifies that creation and dispatches fail.
		 */
		void TestRayTracingPipelineCreation();

		/**
		 * Traces rays through a ray tracing pipeline against two instances of a bottom level structure with two triangles, and checks which
		 * program ran for each: hit groups picked by the instance offset, the ray's hit group contribution and the geometry multiplier, an
		 * empty hit group record, miss programs picked by the miss index, and the ray generation record a dispatch picks. Runs only with
		 * RSC_ACCELERATION_STRUCTURES and RSC_RAY_TRACING_PIPELINE.
		 */
		void TestRayTracingPipelineRecords();

		/**
		 * Builds a bottom and a top level structure in a frame graph and traces a ray against the result: in one graph that builds both and
		 * traces, in a graph that rebuilds and exports the top level structure on the async compute queue from a bottom level structure at
		 * rest, and in a graph that traces against the structure at rest. Checks that a build of a structure that is not imported is an
		 * error. Runs only with RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY.
		 */
		void TestFrameGraph();

		/**
		 * Refits a bottom and a top level structure built with GpuAccelerationStructureBuildFlag::AllowRefit in place, in frame graphs, and
		 * traces a ray against the result: after the vertices moved, after the instance moved, and with the vertices in another buffer than
		 * the one the structure was built from. Checks that only structures that allow refits need refit scratch memory, and in development
		 * builds that refits from inputs without the flag, or from a structure of another level, are rejected. Runs only with
		 * RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY.
		 */
		void TestAccelerationStructureRefits();

		/**
		 * Builds a bottom level structure that allows compaction, writes its compacted size into queries of one call, clones and compacts it,
		 * and traces a ray against each copy after the original is destroyed. Where the device supports serialization, serializes the
		 * compacted structure, reads the header of the data, rejects a header of another driver, deserializes the data and traces against
		 * the result. Clones a top level structure and traces against the clone after every other reference to its bottom level
		 * structure is gone. In development builds, checks that invalid copies, queries and serializations are rejected. Runs only with
		 * RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY.
		 */
		void TestAccelerationStructureCopies();

		/**
		 * Builds the ray tracing geometry of a mesh with two triangle list sub-meshes and a line list, in a RayTracingScene, and reads the
		 * triangles rays hit through RayTracingGeometryAccess.bslinc, inline and from closest hit programs: indices, interpolated positions,
		 * normals and texture coordinates. Then moves, removes and adds instances, writes the mesh, and outgrows the top level structure,
		 * and checks the passes each build adds, also of a second scene that shares the geometry. Then compacts the geometries with
		 * RayTracingManager::Update(), rebuilds a compacted one, and checks the structures, the passes and the hits. Checks that meshes
		 * without MeshFlag::SupportsRayTracing, or on devices without RSC_ACCELERATION_STRUCTURES and RSC_RAY_QUERY, have no geometry.
		 */
		void TestRayTracingGeometry();
	};
} // namespace b3d
