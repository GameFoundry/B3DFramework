//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Particles/BsParticleModule.h"
#include "Math/BsDegree.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"

namespace bs
{
	class Random;

	/** @addtogroup Particles-Internal 
	 *  @{
	 */

	class ParticleSet;

	/** Contains particle emitter state that varies from frame to frame. */
	struct ParticleEmitterState
	{
		/** Emitter state relevant for skinned mesh emitter shape. */
		struct SkinnedMesh
		{
			Matrix4* bones;
			UINT32 numBones;
		};

		union
		{
			SkinnedMesh skinnedMesh;
		};
	};

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** 
	 * Base class from all emitter shapes. Emitter shapes determine the position and direction of newly created particles.
	 */
	class BS_CORE_EXPORT ParticleEmitterShape
	{
	public:
		virtual ~ParticleEmitterShape() = default;

		/** 
		 * Spawns a new set of particles using the current shape's distribution. 
		 *
		 * @param[in]	random		Random number generator.
		 * @param[in]	particles	Particle set in which to insert new particles.
		 * @param[in]	count		Number of particles to spawn.
		 * @param[in]	state		Optional state that can contain various per-frame information required for spawning
		 *							the particles.
		 * @return					Index at which the first of the particles was inserted, with other particles following
		 *							sequentially.
		 */
		virtual UINT32 spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const = 0;

	protected:
		ParticleEmitterShape() = default;
	};


	/** Determines the emission type for the cone particle emitter shape. */
	enum class ParticleEmitterConeType
	{
		/** Emit particles only from the cone base. */
		Base,
		/** Emit particles from the entire cone volume. */
		Volume
	};

	/** Information describing a ParticleEmitterConeShape. */
	struct PARTICLE_CONE_SHAPE_DESC
	{
		/** Determines where on the cone are the particles emitter from. */
		ParticleEmitterConeType type = ParticleEmitterConeType::Base;

		/** Radius of the cone base. */
		float radius = 1.0f;

		/** Angle of the cone. */
		Degree angle = Degree(45.0f);

		/** Length of the cone. Irrelevant if emission type is Base. */
		float length = 1.0f;

		/** 
		 * Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the
		 * edge of the cone, while thickness of 1 results in particles being emitted from the entire volume. In-between
		 * values will use a part of the volume.
		 */
		float thickness = 0.0f;

		/** Angular portion of the cone from which to emit particles from, in degrees. */
		Degree arc = Degree(360.0f);
	};

	/**
	 * Particle emitter shape that emits particles from a cone. Particles can be created on cone base or volume, while
	 * controling the radial arc of the emitted portion of the volume, as well as thickness of the cone emission volume.
	 * All particles will have random normals within the distribution of the cone.
	 */
	class BS_CORE_EXPORT ParticleEmitterConeShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc);
		virtual ~ParticleEmitterConeShape() = default;

		/** Creates a new particle emitter cone shape. */
		static UPtr<ParticleEmitterConeShape> create(const PARTICLE_CONE_SHAPE_DESC& desc);
	protected:
		PARTICLE_CONE_SHAPE_DESC mInfo;
	};

	/** Information describing a ParticleEmitterSphereShape. */
	struct PARTICLE_SPHERE_SHAPE_DESC
	{
		/** Radius of the sphere. */
		float radius = 0.0f;

		/** 
		 * Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the
		 * edge of the volume, while thickness of 1 results in particles being emitted from the entire volume. In-between
		 * values will use a part of the volume.
		 */
		float thickness = 0.0f;
	};

	/**
	 * Particle emitter shape that emits particles from a sphere. Particles can be emitted from sphere surface, the entire
	 * volume or a proportion of the volume depending on the thickness parameter. All particles will have normals pointing
	 * outwards in a spherical direction.
	 */
	class BS_CORE_EXPORT ParticleEmitterSphereShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterSphereShape(const PARTICLE_SPHERE_SHAPE_DESC& desc);

		/** @copydoc ParticleEmitterShape::spawn */
		UINT32 spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const;

		/** Creates a new particle emitter sphere shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_SPHERE_SHAPE_DESC& desc);
	protected:
		PARTICLE_SPHERE_SHAPE_DESC mInfo;
	};

	/** Information describing a ParticleEmitterHemisphereShape. */
	struct PARTICLE_HEMISPHERE_SHAPE_DESC
	{
		/** Radius of the hemisphere. */
		float radius = 0.0f;

		/** 
		 * Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the
		 * edge of the volume, while thickness of 1 results in particles being emitted from the entire volume. In-between
		 * values will use a part of the volume.
		 */
		float thickness = 0.0f;
	};

	/**
	 * Particle emitter shape that emits particles from a hemisphere. Particles can be emitted from the hemisphere surface,
	 * the entire volume or a proportion of the volume depending on the thickness parameter. All particles will have 
	 * normals pointing outwards in a spherical direction.
	 */
	class BS_CORE_EXPORT ParticleEmitterHemisphereShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterHemisphereShape(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc);

		/** @copydoc ParticleEmitterShape::spawn */
		UINT32 spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const;

		/** Creates a new particle emitter sphere shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc);

	protected:
		PARTICLE_HEMISPHERE_SHAPE_DESC mInfo;
	};

	/** Determines the emission type for the cone particle emitter shape. */
	enum class ParticleEmitterBoxType
	{
		/** Particles will be emitted from the entire volume. */
		Volume,
		/** Particles will be emitted only from box surface. */
		Surface, 
		/** Particles will be emitted only from box edge. */
		Edge
	};

	/** Information describing a ParticleEmitterBoxShape. */
	struct PARTICLE_BOX_SHAPE_DESC
	{
		/** Determines from which portion of the box should particles be emitted from. */
		ParticleEmitterBoxType type = ParticleEmitterBoxType::Volume;

		/** Extends of the box. */
		Vector3 extents = Vector3::ONE;
	};

	/** 
	 * Particle emitter shape that emits particles from an axis aligned box. Particles can be emitted from box volume,
	 * surface or edges. All particles have their normals set to positive Z direction.
	 */
	class BS_CORE_EXPORT ParticleEmitterBoxShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc);
		virtual ~ParticleEmitterBoxShape() = default;

		/** Creates a new particle emitter box shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_BOX_SHAPE_DESC& desc);

	protected:
		PARTICLE_BOX_SHAPE_DESC mInfo;
	};

	/** Information describing a ParticleEmitterLineShape. */
	struct PARTICLE_LINE_SHAPE_DESC
	{
		/** Length of the line. */
		float length = 1.0f;
	};

	/** Particle emitter shape that emits particles from a line segment. */
	class BS_CORE_EXPORT ParticleEmitterLineShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterLineShape(const PARTICLE_LINE_SHAPE_DESC& desc);

		/** @copydoc ParticleEmitterShape::spawn */
		UINT32 spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const;

		/** Creates a new particle emitter edge shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_LINE_SHAPE_DESC& desc);

	protected:
		PARTICLE_LINE_SHAPE_DESC mInfo;
	};

	/** Information describing a ParticleEmitterCircleShape. */
	struct PARTICLE_CIRCLE_SHAPE_DESC
	{
		/** Radius of the circle. */
		float radius = 1.0f;

		/** 
		 * Proportion of the surface that can emit particles. Thickness of 0 results in particles being emitted only from
		 * the edge of the circle, while thickness of 1 results in particles being emitted from the entire surface. 
		 * In-between values will use a part of the surface.
		 */
		float thickness = 0.0f;

		/** Angular portion of the cone from which to emit particles from, in degrees. */
		Degree arc = Degree(360.0f);
	};

	/** 
	 * Particle emitter shape that emits particles from a circle. Using the thickness parameter you can control whether to
	 * emit only from circle edge, the entire surface or just a part of the surface. Using the arc parameter you can emit
	 * from a specific angular portion of the circle.
	 */
	class BS_CORE_EXPORT ParticleEmitterCircleShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterCircleShape(const PARTICLE_CIRCLE_SHAPE_DESC& desc);
		virtual ~ParticleEmitterCircleShape() = default;

		/** Creates a new particle emitter circle shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_CIRCLE_SHAPE_DESC& desc);

	protected:
		PARTICLE_CIRCLE_SHAPE_DESC mInfo;
	};

	/** Information describing a ParticleEmitterRectShape. */
	struct PARTICLE_RECT_SHAPE_DESC
	{
		/** Extents of the rectangle. */
		Vector2 extents = Vector2::ONE;
	};

	/** Particle emitter shape that emits particles from the surface of a rectangle. */
	class BS_CORE_EXPORT ParticleEmitterRectShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterRectShape(const PARTICLE_RECT_SHAPE_DESC& desc);

		/** @copydoc ParticleEmitterShape::spawn */
		UINT32 spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const;

		/** Creates a new particle emitter rectangle shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_RECT_SHAPE_DESC& desc);

	protected:
		PARTICLE_RECT_SHAPE_DESC mInfo;
	};

	/** Determines the emission type for the mesh particle emitter shape. */
	enum class ParticleEmitterMeshType
	{
		/** Particles will be emitted from mesh vertices. */
		Vertex, 
		/** Particles will be emitted from mesh edges. */
		Edge, 
		/** Particles will be emitted from mesh triangles. */
		Triangle
	};

	/** Information describing a ParticleEmitterStaticMeshShape and ParticleEmitterSkinnedMeshShape. */
	struct PARTICLE_MESH_SHAPE_DESC
	{
		/** Determines from which portion of the mesh are the particles emitted from. */
		ParticleEmitterMeshType type = ParticleEmitterMeshType::Triangle;

		/** 
		 * Data describing the mesh vertices and indices. Must not be null and must at least contain the following
		 * attributes:
		 *  - VES_POSITION of VET_FLOAT3 type, representing vertex positions. Required for both static and skinned emitters.
		 *  - VES_BLEND_INDICES of VET_UBYTE4 type, representing bone indices. Required only for skinned emitters.
		 *  - VES_BLEND_WEIGHTS of VET_FLOAT4 type, representing bone weights. Required only for skinned emitters.
		 *  - VES_NORMAL of either VET_FLOAT3 or VET_UBYTE4_NORM type, representing vertex normals. Optional for both
		 *    static and skinned emitters.
		 */
		SPtr<MeshData> meshData;
	};

	/** 
	 * Particle emitter shape that emits particles from a surface of a static (non-animated) mesh. Particles can be
	 * emitted from mesh vertices, edges or triangles. If information about normals exists, particles will also inherit
	 * the normals.
	 */
	class BS_CORE_EXPORT ParticleEmitterStaticMeshShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterStaticMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc);
		virtual ~ParticleEmitterStaticMeshShape() = default;

		/** Creates a new particle emitter static mesh shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_MESH_SHAPE_DESC& desc);

	protected:
		PARTICLE_MESH_SHAPE_DESC mInfo;
		UINT8* mVertices;
		UINT8* mNormals;
		UINT32 mNumVertices;
		UINT32 mVertexStride;
		bool m32BitNormals;
	};

	/** Particle emitter shape that emits particles from a surface of a skinned (animated) mesh. Particles can be
	 * emitted from mesh vertices, edges or triangles. If information about normals exists, particles will also inherit
	 * the normals.
	 */
	class BS_CORE_EXPORT ParticleEmitterSkinnedMeshShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterSkinnedMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc);
		virtual ~ParticleEmitterSkinnedMeshShape() = default;

		/** Creates a new particle emitter skinned mesh shape. */
		static UPtr<ParticleEmitterShape> create(const PARTICLE_MESH_SHAPE_DESC& desc);
	protected:
		/** Evaluates a blend matrix for a vertex at the specified index. */
		Matrix4 getBlendMatrix(const ParticleEmitterState& state, UINT32 vertexIdx) const;

		PARTICLE_MESH_SHAPE_DESC mInfo;
		UINT8* mVertices;
		UINT8* mNormals;
		UINT8* mBoneIndices;
		UINT8* mBoneWeights;
		UINT32 mNumVertices;
		UINT32 mVertexStride;
		bool m32BitNormals;
	};

	/** Handles spawning of new particles using the specified parameters and shape. */
	class BS_CORE_EXPORT ParticleEmitter : public ParticleModule
	{
	public:
		ParticleEmitter();

		/** Shape over which to emit the particles. */
		void setShape(UPtr<ParticleEmitterShape> shape) { mShape = std::move(shape); }

		/** @copydoc setShape */
		ParticleEmitterShape* getShape() const { return mShape.get(); }

		/** 
		 * Spawns new particles in the specified time increment (if any). 
		 *
		 * @param[in]	timeDelta		Determines how much time has passed since the last call to this method, which in
		 *								turn controls how many particles to spawn (depending on other properties).
		 * @param[in]	random			Random number generator.
		 * @param[in]	state			Per-frame state required by some emitters.
		 * @param[in]	set				Set to which to append new particles to.
		 */
		void spawn(float timeDelta, Random& random, const ParticleEmitterState& state, ParticleSet& set) const;

	private:
		UPtr<ParticleEmitterShape> mShape;
	};

	/** @} */

}
