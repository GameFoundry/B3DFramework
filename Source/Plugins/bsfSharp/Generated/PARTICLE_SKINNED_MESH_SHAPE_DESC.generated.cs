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

	/// <summary>Information describing a ParticleEmitterSkinnedMeshShape.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleSkinnedMeshShapeOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleSkinnedMeshShapeOptions Default()
		{
			ParticleSkinnedMeshShapeOptions value = new ParticleSkinnedMeshShapeOptions();
			value.type = ParticleEmitterMeshType.Triangle;
			value.sequential = false;
			value.renderable = null;

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
		/// Renderable object containing a mesh to spawn particles on, as well as the attached Animation object resposible for 
		/// performing skinned animation. Mesh must at least contain per-vertex position data encoded as 3D float vectors, blend 
		/// indices encoded in 4-byte format, and blend weights encoded a 4D float vectors. Can optionally  contain per-vertex 
		/// normals encoded as 3D float vectors or as 4-byte unsigned-normalized format.
		/// </summary>
		public Renderable renderable;
	}

	/** @} */
}
