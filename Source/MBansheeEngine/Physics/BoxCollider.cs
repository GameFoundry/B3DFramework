//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Collider with box geometry.
    /// </summary>
    public sealed class BoxCollider : Collider
    {
        [SerializeField]
        private Vector3 extents = new Vector3(0.5f, 0.5f, 0.5f);

        /// <summary>
        /// Extents (half size) of the geometry of the box.
        /// </summary>
        public Vector3 Extents
        {
            get { return extents; }
            set
            {
                value = new Vector3(MathEx.Max(value.x, 0.01f), MathEx.Max(value.y, 0.01f), MathEx.Max(value.z, 0.01f));

                if (extents == value)
                    return;

                extents = value;

                if (Native != null)
                {
                    Native.Extents = value;

                    if (parent != null)
                        parent.UpdateMassDistribution();
                }
            }
        }

        /// <summary>
        /// Position of the box shape, relative to the component's scene object.
        /// </summary>
        public Vector3 Center
        {
            get { return serializableData.localPosition; }
            set
            {
                if (serializableData.localPosition == value)
                    return;

                serializableData.localPosition = value;

                if (Native != null)
                    UpdateTransform();
            }
        }

        /// <summary>
        /// Returns the native box collider wrapped by this component.
        /// </summary>
        private NativeBoxCollider Native
        {
            get { return (NativeBoxCollider)native; }
        }

        /// <inheritdoc/>
        internal override NativeCollider CreateCollider()
        {
            NativeBoxCollider boxCollider = new NativeBoxCollider();
            boxCollider.Extents = extents;

            return boxCollider;
        }
    }

    /** @} */
}
