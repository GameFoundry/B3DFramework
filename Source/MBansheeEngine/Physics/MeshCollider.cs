//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// A collider represented by an arbitrary mesh.
    /// </summary>
    public sealed class MeshCollider : Collider
    {
        [SerializeField]
        private PhysicsMesh mesh;

        /// <summary>
        /// Mesh that represents the collider geometry. This can be a generic triangle mesh, or and convex mesh. Triangle
        /// meshes are not supported as triggers, nor are they supported for colliders that are parts of a non-kinematic
        /// rigidbody.
        /// </summary>
        public PhysicsMesh Mesh
        {
            get { return mesh; }
            set
            {
                if (mesh == value)
                    return;

                if (IsTrigger && mesh.Type == PhysicsMeshType.Triangle)
                {
                    Debug.LogWarning("Triangle meshes are not supported on Trigger colliders.");
                    return;
                }

                mesh = value;

                if (Native != null)
                {
                    Native.Mesh = value;

                    if (parent != null)
                    {
                        // If triangle mesh its possible the parent can no longer use this collider (they're not supported for 
                        // non-kinematic rigidbodies)
                        if (mesh != null && mesh.Type == PhysicsMeshType.Triangle)
                            UpdateParentRigidbody();
                        else
                            parent.UpdateMassDistribution();
                    }
                }
            }
        }

        /// <inheritdoc/>
        protected internal override bool IsValidParent(Rigidbody parent) 
	    {
		    // Triangle mesh colliders cannot be used for non-kinematic rigidbodies
		    return mesh == null|| mesh.Type == PhysicsMeshType.Convex || parent.Kinematic;
        }

        /// <summary>
        /// Returns the native mesh collider wrapped by this component.
        /// </summary>
        private NativeMeshCollider Native
        {
            get { return (NativeMeshCollider)native; }
        }

        /// <inheritdoc/>
        internal override NativeCollider CreateCollider()
        {
            NativeMeshCollider meshCollider = new NativeMeshCollider();
            meshCollider.Mesh = mesh;

            return meshCollider;
        }
    }

    /** @} */
}
