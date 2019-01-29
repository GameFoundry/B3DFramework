//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Information describing a ParticleEmitterStaticMeshShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleStaticMeshShapeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleStaticMeshShapeOptions Default()
		{
			ParticleStaticMeshShapeOptions value = new ParticleStaticMeshShapeOptions();
			value.type = ParticleEmitterMeshType.Triangle;
			value.sequential = false;
			value.mesh = null;

			return value;
		}

		/// <summary>Determines from which portion of the mesh are the particles emitted from.</summary>
		public ParticleEmitterMeshType type;
		/// <summary>
		/// When enabled the particles will be emitted sequentially from mesh vertices in the order they are defined.  Only 
		/// relevant for the Vertex emit mode.
		/// </summary>
		public bool sequential;
		/// <summary>
		/// Mesh to spawn particles on. Must at least contain per-vertex position data encoded as 3D float vectors. Can 
		/// optionally contain per-vertex normals encoded as 3D float vectors or as 4-byte unsigned-normalized format.
		/// </summary>
		public RRef<Mesh> mesh;
	}

	/** @} */
}
