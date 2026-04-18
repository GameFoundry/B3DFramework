//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Particles/B3DParticleModule.h"
#include "Math/B3DDegree.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DMatrix4.h"
#include "B3DParticleDistribution.h"

namespace b3d
{
	class Random;
	class ParticleSet;

	/** @addtogroup Particles
	 *  @{
	 */

	/** Types of emission modes. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmissionModeType
	{
		/** Position will be picked randomly on a shape. */
		Random,

		/** Positions will loop around the shape in a predictable fashion. */
		Loop,

		/** Similar to Loop, except the order will be reversed when one loop iteration finishes. */
		PingPong,

		/**
		 * All particles spawned on the shape at some instant (usually a frame) will be spread around the shape equally.
		 */
		Spread
	};

	/** Controls how are particle positions on a shape chosen. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true)) ParticleEmissionMode
	{
		/** Type that determines general behaviour. */
		ParticleEmissionModeType Type = ParticleEmissionModeType::Random;

		/**
		 * Speed along which particle generation should move around the shape, relevant for Loop and PingPing emission
		 * modes.
		 */
		float Speed = 1.0f;

		/**
		 * Determines the minimum interval allowed between the generated particles. 0 specifies the particles can be
		 * generated anywhere on the shape.
		 */
		float Interval = 0.0f;
	};

	/**
	 * Base class from all emitter shapes. Emitter shapes determine the position and direction of newly created particles.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterShape : public IReflectable, public IScriptExportable
	{
	public:
		virtual ~ParticleEmitterShape() = default;

		/**
		 * @name Internal
		 * @{
		 */

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
		virtual u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const = 0;

		/** @} */
	protected:
		friend class ParticleEmitter;

		ParticleEmitterShape() = default;

		/**
		 * Calculates the bounds of the emitter shape.
		 *
		 * @param[in]	shape		AABB for the emitter shape itself.
		 * @param[in]	velocity	AABB for the generated normals.
		 */
		virtual void CalcBounds(AABox& shape, AABox& velocity) const = 0;

		/**
		 * Checks has the emitter been initialized properly. If the emitter is not valid then the spawn() method is
		 * not allowed to be called.
		 */
		bool IsValid() const { return mIsValid; }

		bool mIsValid = true;
	};

	/** Determines the emission type for the cone particle emitter shape. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterConeType
	{
		/** Emit particles only from the cone base. */
		Base,
		/** Emit particles from the entire cone volume. */
		Volume
	};

	/** Information describing a ParticleEmitterConeShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleConeShapeSettings)) ParticleConeShapeSettings
	{
		/** Determines where on the cone are the particles emitter from. */
		ParticleEmitterConeType Type = ParticleEmitterConeType::Base;

		/** Radius of the cone base. */
		float Radius = 0.0f;

		/** Angle of the cone. */
		Degree Angle = Degree(45.0f);

		/** Length of the cone. Irrelevant if emission type is Base. */
		float Length = 1.0f;

		/**
		 * Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the
		 * edge of the cone, while thickness of 1 results in particles being emitted from the entire volume. In-between
		 * values will use a part of the volume.
		 */
		float Thickness = 1.0f;

		/** Angular portion of the cone from which to emit particles from, in degrees. */
		Degree Arc = Degree(360.0f);

		/** Determines how will particle positions on the shape be generated. */
		ParticleEmissionMode Mode;
	};

	/**
	 * Particle emitter shape that emits particles from a cone. Particles can be created on cone base or volume, while
	 * controlling the radial arc of the emitted portion of the volume, as well as thickness of the cone emission volume.
	 * All particles will have random normals within the distribution of the cone.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterConeShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterConeShape(const ParticleConeShapeSettings& settings);
		ParticleEmitterConeShape() = default;
		virtual ~ParticleEmitterConeShape() = default;

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleConeShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleConeShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter cone shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterConeShape> Create(const ParticleConeShapeSettings& settings);

		/** Creates a new particle emitter cone shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterConeShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle randomly, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** Spawns a single particle on the specified point on the cone, generating its position and normal. */
		void SpawnInternal(float t, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		/** Generates a position and normal of a particle based on the input 2D position on the cone circle base. */
		void GetPointInCone(const Vector2& pos2D, float distance, Vector3& position, Vector3& normal) const;

		ParticleConeShapeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterConeShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information describing a ParticleEmitterSphereShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleSphereShapeSettings)) ParticleSphereShapeSettings
	{
		/** Radius of the sphere. */
		float Radius = 1.0f;

		/**
		 * Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the
		 * edge of the volume, while thickness of 1 results in particles being emitted from the entire volume. In-between
		 * values will use a part of the volume.
		 */
		float Thickness = 0.0f;
	};

	/**
	 * Particle emitter shape that emits particles from a sphere. Particles can be emitted from sphere surface, the entire
	 * volume or a proportion of the volume depending on the thickness parameter. All particles will have normals pointing
	 * outwards in a spherical direction.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterSphereShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterSphereShape() = default;
		ParticleEmitterSphereShape(const ParticleSphereShapeSettings& settings);

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleSphereShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleSphereShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter sphere shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterSphereShape> Create(const ParticleSphereShapeSettings& settings);

		/** Creates a new particle emitter sphere shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterSphereShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleSphereShapeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterSphereShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information describing a ParticleEmitterHemisphereShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleHemisphereShapeSettings)) ParticleHemisphereShapeSettings
	{
		/** Radius of the hemisphere. */
		float Radius = 1.0f;

		/**
		 * Proportion of the volume that can emit particles. Thickness of 0 results in particles being emitted only from the
		 * edge of the volume, while thickness of 1 results in particles being emitted from the entire volume. In-between
		 * values will use a part of the volume.
		 */
		float Thickness = 0.0f;
	};

	/**
	 * Particle emitter shape that emits particles from a hemisphere. Particles can be emitted from the hemisphere surface,
	 * the entire volume or a proportion of the volume depending on the thickness parameter. All particles will have
	 * normals pointing outwards in a spherical direction.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterHemisphereShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterHemisphereShape() = default;
		ParticleEmitterHemisphereShape(const ParticleHemisphereShapeSettings& settings);

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleHemisphereShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleHemisphereShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter sphere shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterHemisphereShape> Create(const ParticleHemisphereShapeSettings& settings);

		/** Creates a new particle emitter sphere shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterHemisphereShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleHemisphereShapeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterHemisphereShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Determines the emission type for the cone particle emitter shape. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterBoxType
	{
		/** Particles will be emitted from the entire volume. */
		Volume,
		/** Particles will be emitted only from box surface. */
		Surface,
		/** Particles will be emitted only from box edge. */
		Edge
	};

	/** Information describing a ParticleEmitterBoxShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleBoxShapeSettings)) ParticleBoxShapeSettings
	{
		/** Determines from which portion of the box should particles be emitted from. */
		ParticleEmitterBoxType Type = ParticleEmitterBoxType::Volume;

		/** Extends of the box. */
		Vector3 Extents = Vector3::kOne;
	};

	/**
	 * Particle emitter shape that emits particles from an axis aligned box. Particles can be emitted from box volume,
	 * surface or edges. All particles have their normals set to positive Z direction.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterBoxShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterBoxShape() = default;
		ParticleEmitterBoxShape(const ParticleBoxShapeSettings& settings);

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleBoxShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleBoxShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter box shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterBoxShape> Create(const ParticleBoxShapeSettings& settings);

		/** Creates a new particle emitter box shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterBoxShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleBoxShapeSettings mSettings;

		float mSurfaceArea[3];
		float mEdgeLengths[3];

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterBoxShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information describing a ParticleEmitterLineShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleLineShapeSettings)) ParticleLineShapeSettings
	{
		/** Length of the line. */
		float Length = 1.0f;

		/** Determines how will particle positions on the shape be generated. */
		ParticleEmissionMode Mode;
	};

	/** Particle emitter shape that emits particles from a line segment. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterLineShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterLineShape() = default;
		ParticleEmitterLineShape(const ParticleLineShapeSettings& settings);

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleLineShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleLineShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter edge shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterLineShape> Create(const ParticleLineShapeSettings& settings);

		/** Creates a new particle emitter edge shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterLineShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle randomly, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** Spawns a single particle on the specified point on the line, generating its position and normal. */
		void SpawnInternal(float t, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleLineShapeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterLineShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information describing a ParticleEmitterCircleShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleCircleShapeSettings)) ParticleCircleShapeSettings
	{
		/** Radius of the circle. */
		float Radius = 1.0f;

		/**
		 * Proportion of the surface that can emit particles. Thickness of 0 results in particles being emitted only from
		 * the edge of the circle, while thickness of 1 results in particles being emitted from the entire surface.
		 * In-between values will use a part of the surface.
		 */
		float Thickness = 0.0f;

		/** Angular portion of the cone from which to emit particles from, in degrees. */
		Degree Arc = Degree(360.0f);

		/** Determines how will particle positions on the shape be generated. */
		ParticleEmissionMode Mode;
	};

	/**
	 * Particle emitter shape that emits particles from a circle. Using the thickness parameter you can control whether to
	 * emit only from circle edge, the entire surface or just a part of the surface. Using the arc parameter you can emit
	 * from a specific angular portion of the circle.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterCircleShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterCircleShape() = default;
		ParticleEmitterCircleShape(const ParticleCircleShapeSettings& settings);
		virtual ~ParticleEmitterCircleShape() = default;

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleCircleShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleCircleShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter circle shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterCircleShape> Create(const ParticleCircleShapeSettings& settings);

		/** Creates a new particle emitter circle shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterCircleShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle randomly, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** Spawns a single particle on the specified point on the circle, generating its position and normal. */
		void SpawnInternal(float t, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleCircleShapeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterCircleShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information describing a ParticleEmitterRectShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleRectShapeSettings)) ParticleRectangleShapeSettings
	{
		/** Extents of the rectangle. */
		Vector2 Extents = Vector2::kOne;
	};

	/** Particle emitter shape that emits particles from the surface of a rectangle. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterRectShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterRectShape() = default;
		ParticleEmitterRectShape(const ParticleRectangleShapeSettings& settings);

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleRectangleShapeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleRectangleShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter rectangle shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterRectShape> Create(const ParticleRectangleShapeSettings& settings);

		/** Creates a new particle emitter rectangle shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterRectShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** Spawns a single particle, generating its position and normal. */
		void SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleRectangleShapeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterRectShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Determines the emission type for the mesh particle emitter shape. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterMeshType
	{
		/** Particles will be emitted from mesh vertices. */
		Vertex,
		/** Particles will be emitted from mesh edges. */
		Edge,
		/** Particles will be emitted from mesh triangles. */
		Triangle
	};

	/** Information describing a ParticleEmitterStaticMeshShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleStaticMeshShapeSettings)) ParticleStaticMeshShapeSettings
	{
		/** Determines from which portion of the mesh are the particles emitted from. */
		ParticleEmitterMeshType Type = ParticleEmitterMeshType::Triangle;

		/**
		 * When enabled the particles will be emitted sequentially from mesh vertices in the order they are defined.
		 * Only relevant for the Vertex emit mode.
		 */
		bool Sequential = false;

		/**
		 * Mesh to spawn particles on. Must at least contain per-vertex position data encoded as 3D float vectors. Can
		 * optionally contain per-vertex normals encoded as 3D float vectors or as 4-byte unsigned-normalized format.
		 */
		HMesh Mesh;
	};

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/**
	 * Calculates and stores per-triangle weights that can be used for easily picking a random triangle on a mesh, ensuring
	 * larger triangles are picked more likely.
	 */
	class MeshWeightedTriangles
	{
		/** Contains the cumulative, normalized weight of the triangle and its vertex indices. */
		struct TriangleWeight
		{
			float CumulativeWeight;
			u32 Indices[3];
		};

	public:
		MeshWeightedTriangles() = default;
		MeshWeightedTriangles(const MeshData& meshData);

		/** Updates the weights from the provided mesh data. */
		void Calculate(const MeshData& meshData);

		/** Find a random triangle on the mesh and outputs its vertex indices. */
		void GetTriangle(const Random& random, std::array<u32, 3>& indices) const;

	private:
		Vector<TriangleWeight> mWeights;
	};

	/** Contains common functionality for particle mesh emitters. */
	class MeshEmissionHelper
	{
	public:
		/**
		 * Initializes the emission helper if the provided mesh contains necessary data for particle emission. Otherwise
		 * reports any issues in the log.
		 *
		 * @param[in]	mesh		Mesh to validate.
		 * @param[in]	perVertex	Set to true if particle emission is happening on mesh vertices.
		 * @param[in]	skinning	Set to true if the mesh will be animated using skinning.
		 * @return					True if initialized, or false if issues were detected.
		 */
		bool Initialize(const HMesh& mesh, bool perVertex, bool skinning);

		/**
		 * Returns the next sequential vertex on the mesh and increments the internal counter so the next vertex is
		 * returned on the following call. Loops around if end is reached. Returns vertex position, normal and index.
		 */
		void GetSequentialVertex(Vector3& position, Vector3& normal, u32& idx) const;

		/** Randomly picks a vertex on the mesh and returns its position, normal and index. */
		void GetRandomVertex(const Random& random, Vector3& position, Vector3& normal, u32& idx) const;

		/** Randomly picks an edge on the mesh and returns the position, normal and indices of its vertices. */
		void GetRandomEdge(const Random& random, std::array<Vector3, 2>& position, std::array<Vector3, 2>& normal, std::array<u32, 2>& idx) const;

		/** Randomly picks an triangle on the mesh and returns the position, normal and indices of its vertices. */
		void GetRandomTriangle(const Random& random, std::array<Vector3, 3>& position, std::array<Vector3, 3>& normal, std::array<u32, 3>& idx) const;

		/** Evaluates a blend matrix for a vertex at the specified index. */
		Matrix4 GetBlendMatrix(const Matrix4* bones, u32 vertexIdx) const;

	private:
		MeshWeightedTriangles mWeightedTriangles;

		u8* mVertices = nullptr;
		u8* mNormals = nullptr;
		u32 mNumVertices = 0;
		u32 mVertexStride = 0;
		bool m32BitNormals = true;

		u8* mBoneIndices = nullptr;
		u8* mBoneWeights = nullptr;

		SPtr<MeshData> mMeshData;

		// Transient
		mutable u32 mNextSequentialIdx = 0;
	};

	/** @} */

	/**
	 * Particle emitter shape that emits particles from a surface of a static (non-animated) mesh. Particles can be
	 * emitted from mesh vertices, edges or triangles. If information about normals exists, particles will also inherit
	 * the normals.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterStaticMeshShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterStaticMeshShape(const ParticleStaticMeshShapeSettings& settings);
		ParticleEmitterStaticMeshShape();
		virtual ~ParticleEmitterStaticMeshShape() = default;

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleStaticMeshShapeSettings& settings);

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleStaticMeshShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter static mesh shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterStaticMeshShape> Create(const ParticleStaticMeshShapeSettings& settings);

		/** Creates a new particle emitter static mesh shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterStaticMeshShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleStaticMeshShapeSettings mSettings;
		MeshEmissionHelper mMeshEmissionHelper;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterStaticMeshShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Information describing a ParticleEmitterSkinnedMeshShape. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleSkinnedMeshShapeSettings)) ParticleSkinnedMeshShapeSettings
	{
		/** Determines from which portion of the mesh are the particles emitted from. */
		ParticleEmitterMeshType Type = ParticleEmitterMeshType::Triangle;

		/**
		 * When enabled the particles will be emitted sequentially from mesh vertices in the order they are defined.
		 * Only relevant for the Vertex emit mode.
		 */
		bool Sequential = false;

		/**
		 * Renderable object containing a mesh to spawn particles on, as well as the attached Animation object resposible
		 * for performing skinned animation. Mesh must at least contain per-vertex position data encoded as 3D float
		 * vectors, blend indices encoded in 4-byte format, and blend weights encoded a 4D float vectors. Can optionally
		 * contain per-vertex normals encoded as 3D float vectors or as 4-byte unsigned-normalized format.
		 */
		HRenderable Renderable;
	};

	/**
	 * Particle emitter shape that emits particles from a surface of a skinned (animated) mesh. Particles can be
	 * emitted from mesh vertices, edges or triangles. If information about normals exists, particles will also inherit
	 * the normals.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitterSkinnedMeshShape : public ParticleEmitterShape
	{
	public:
		ParticleEmitterSkinnedMeshShape(const ParticleSkinnedMeshShapeSettings& settings);
		ParticleEmitterSkinnedMeshShape();
		virtual ~ParticleEmitterSkinnedMeshShape() = default;

		/** Options describing the shape. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), UI(Inline))
		void SetSettings(const ParticleSkinnedMeshShapeSettings& settings);

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleSkinnedMeshShapeSettings& GetSettings() const { return mSettings; }

		/** Creates a new particle emitter skinned mesh shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterSkinnedMeshShape> Create(const ParticleSkinnedMeshShapeSettings& settings);

		/** Creates a new particle emitter skinned mesh shape. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitterSkinnedMeshShape> Create();

		/**
		 * @name Internal
		 * @{
		 */

		u32 SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const override;

		/** @} */
	protected:
		void CalcBounds(AABox& shape, AABox& velocity) const override;

		ParticleSkinnedMeshShapeSettings mSettings;
		MeshEmissionHelper mMeshEmissionHelper;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterSkinnedMeshShapeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Specifies a burst of particles that occurs at a certain time point. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true)) ParticleBurst
	{
		ParticleBurst() = default;

		ParticleBurst(float time, FloatDistribution count, u32 cycles = 1, float interval = 1.0f)
			: Time(time), Count(std::move(count)), Cycles(cycles), Interval(interval)
		{}

		/** Time at which to trigger the burst, in seconds. */
		float Time = 0.0f;

		/** Number of particles to emit when the burst triggers. */
		FloatDistribution Count = 0;

		/**
		 * Determines how many times to trigger the burst. If 0 the burst will trigger infinitely. Use @p interval to
		 * to control the time between each cycle.
		 */
		u32 Cycles = 1;

		/** Controls how much time needs to pass before triggering another burst cycle, in seconds. */
		float Interval = 1.0f;
	};

	/** Handles spawning of new particles using the specified parameters and shape. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEmitter : public ParticleModule
	{
	public:
		/** Shape over which to emit the particles. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Shape))
		void SetShape(SPtr<ParticleEmitterShape> shape) { mShape = std::move(shape); }

		/** @copydoc SetShape */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Shape))
		const SPtr<ParticleEmitterShape>& GetShape() const { return mShape; }

		/** Determines the number of particles that are emitted every second. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(EmissionRate))
		void SetEmissionRate(FloatDistribution value) { mEmissionRate = std::move(value); }

		/** @copydoc SetEmissionRate */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(EmissionRate))
		const FloatDistribution& GetEmissionRate() const { return mEmissionRate; }

		/** Determines discrete intervals to emit particles. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(EmissionBursts))
		void SetEmissionBursts(Vector<ParticleBurst> bursts);

		/** @copydoc SetEmissionBursts */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(EmissionBursts))
		const Vector<ParticleBurst>& GetEmissionBursts() const { return mBursts; }

		/** Determines the lifetime of particles when they are initially spawned, in seconds. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialLifetime))
		void SetInitialLifetime(FloatDistribution value) { mInitialLifetime = std::move(value); }

		/** @copydoc SetInitialLifetime */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialLifetime))
		const FloatDistribution& GetInitialLifetime() const { return mInitialLifetime; }

		/**
		 * Sets the initial speed of the particles, in meters/second. The speed is applied along the particle's velocity
		 * direction, which is determined by the emission shape and potentially other properties.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialSpeed))
		void SetInitialSpeed(FloatDistribution value) { mInitialSpeed = std::move(value); }

		/** @copydoc SetInitialSpeed */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialSpeed))
		const FloatDistribution& GetInitialSpeed() const { return mInitialSpeed; }

		/**
		 * Determines the size of the particles when initially spawned. The size is applied uniformly in all dimensions.
		 * Only used if 3D size is disabled.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialSize))
		void SetInitialSize(FloatDistribution value) { mInitialSize = std::move(value); }

		/** @copydoc SetInitialSize */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialSize))
		const FloatDistribution& GetInitialSize() const { return mInitialSize; }

		/**
		 * Determines the size of the particles when initially spawned. Size can be specified for each dimension separately.
		 * Only used if 3D size is enabled.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialSize3D))
		void SetInitialSize3D(Vector3Distribution value) { mInitialSize3D = std::move(value); }

		/** @copydoc SetInitialSize3D */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialSize3D))
		const Vector3Distribution& GetInitialSize3D() const { return mInitialSize3D; }

		/**
		 * Determines should the initial particle size be applied uniformly (if disabled), or evaluated separately for each
		 * dimension (if enabled).
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Use3DSize))
		void SetUse3DSize(bool value) { mUse3DSize = value; }

		/** @copydoc SetUse3DSize */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Use3DSize))
		bool GetUse3DSize() const { return mUse3DSize; }

		/**
		 * Determines the rotation of the particles when initially spawned, in degrees. The rotation is applied around the
		 * particle's local Z axis. Only used if 3D rotation is disabled.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialRotation))
		void SetInitialRotation(FloatDistribution value) { mInitialRotation = std::move(value); }

		/** @copydoc SetInitialRotation */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialRotation))
		const FloatDistribution& GetInitialRotation() const { return mInitialRotation; }

		/**
		 * Determines the rotation of the particles when initially spawned, in Euler angles. Only used if 3D rotation is
		 * enabled.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialRotation3D))
		void SetInitialRotation3D(Vector3Distribution value) { mInitialRotation3D = std::move(value); }

		/** @copydoc SetInitialRotation3D */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialRotation3D))
		const Vector3Distribution& GetInitialRotation3D() const { return mInitialRotation3D; }

		/**
		 * Determines should the initial particle rotation be a single angle applied around a Z axis (if disabled), or a
		 * set of Euler angles that allow you to rotate around every axis (if enabled).
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Use3DRotation))
		void SetUse3DRotation(bool value) { mUse3DRotation = value; }

		/** @copydoc SetUse3DRotation */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Use3DRotation))
		bool GetUse3DRotation() const { return mUse3DRotation; }

		/** Determines the initial color (in RGB channels) and transparency (in A channel) of particles. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(InitialColor))
		void SetInitialColor(const ColorDistribution& value) { mInitialColor = value; }

		/** @copydoc SetInitialColor */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(InitialColor))
		const ColorDistribution& GetInitialColor() const { return mInitialColor; }

		/**
		 * Determines a range of values determining a random offset to apply to particle position after it has been emitted.
		 * Offset will be randomly selected in all three axes in range [-value, value].
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(RandomOffset))
		void SetRandomOffset(float value) { mRandomOffset = value; }

		/** @copydoc SetRandomOffset */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RandomOffset))
		float GetRandomOffset() const { return mRandomOffset; }

		/**
		 * Determines should particle U texture coordinate be randomly flipped, mirroring the image. The value represents
		 * a percent of particles that should be flipped, in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(FlipU))
		void SetFlipU(float value) { mFlipU = Math::Clamp01(value); }

		/** @copydoc SetFlipU */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(FlipU))
		float GetFlipU() const { return mFlipU; }

		/**
		 * Determines should particle V texture coordinate be randomly flipped, mirroring the image. The value represents
		 * a percent of particles that should be flipped, in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(FlipV))
		void SetFlipV(float value) { mFlipV = Math::Clamp01(value); }

		/** @copydoc SetFlipV */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(FlipV))
		float GetFlipV() const { return mFlipV; }

		/** Creates a new emitter. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleEmitter> Create();

	private:
		friend class ParticleSystem;
		friend class ParticleScene;

		/**
		 * Spawns new particles in the specified time increment (if any).
		 *
		 * @param[in]	random			Random number generator.
		 * @param[in]	state			Various per-frame information provided by the parent particle system.
		 * @param[in]	set				Set to which to append new particles to.
		 */
		void Spawn(Random& random, const ParticleSystemState& state, ParticleSet& set) const;

		/**
		 * Spawns the specified number of particles.
		 *
		 * @param[in]	count			Number of particles to spawn.
		 * @param[in]	random			Random number generator.
		 * @param[in]	state			Various per-frame information provided by the parent particle system.
		 * @param[in]	set				Set to which to append new particles to.
		 * @param[in]	spacing			When false all particles will use the current emitter time. When true the particles
		 *								will be assigned a time between current time and time step end time, so they are
		 *								unifomly distributed in this time range.
		 * @return						Actual number of spawned particles.
		 */
		u32 Spawn(u32 count, Random& random, const ParticleSystemState& state, ParticleSet& set, bool spacing) const;

		// User-visible properties
		SPtr<ParticleEmitterShape> mShape;

		FloatDistribution mEmissionRate = 50.0f;
		Vector<ParticleBurst> mBursts;

		FloatDistribution mInitialLifetime = 10.0f;
		FloatDistribution mInitialSpeed = 1.0f;

		FloatDistribution mInitialSize = 0.1f;
		Vector3Distribution mInitialSize3D = Vector3::kOne;
		bool mUse3DSize = false;

		FloatDistribution mInitialRotation = 0.0f;
		Vector3Distribution mInitialRotation3D = Vector3::kZero;
		bool mUse3DRotation = false;

		ColorDistribution mInitialColor = Color::kWhite;

		float mFlipU = 0.0f;
		float mFlipV = 0.0f;

		float mRandomOffset = 0.0f;

		// Internal state
		mutable float mEmitAccumulator = 0.0f;
		mutable Vector<float> mBurstAccumulator;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleEmitterRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

} // namespace b3d
