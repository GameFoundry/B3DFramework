//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Physics
     *  @{
     */

    /// <summary>
    /// Material that controls how two physical objects interact with each other. Materials of both objects are used during
    /// their interaction and their combined values are used.
    /// </summary>
    public class PhysicsMaterial : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private PhysicsMaterial(bool dummy)
        { }

        /// <summary>
        /// Creates a brand new physics material.
        /// </summary>
        /// <param name="staticFriction">Controls friction when two in-contact objects are not moving lateral to each other.
        ///                              </param>
        /// <param name="dynamicFriction">Controls friction when two in-contact objects are moving lateral to each other.
        ///                               </param>
        /// <param name="restitution">Controls "bounciness" of an object during a collision. Value of 1 means the collision
        /// is elastic, and value of 0 means the value is inelastic.</param>
        public PhysicsMaterial(float staticFriction = 0.0f, float dynamicFriction = 0.0f, float restitution = 0.0f)
        {
            Internal_CreateInstance(this, staticFriction, dynamicFriction, restitution);
        }

        /// <summary>
        /// Controls friction when two in-contact objects are not moving lateral to each other (for example how difficult is
        /// to get an object moving from a static state while it is in contact other object(s)).
        /// </summary>
        public float StaticFriction
        {
            get { return Internal_GetStaticFriction(mCachedPtr); }
            set { Internal_SetStaticFriction(mCachedPtr, value); }
        }


        /// <summary>
        /// Controls friction when two in-contact objects are moving lateral to each other (for example how quickly does an
        /// object slow down when sliding along another object).
        /// </summary>
        public float DynamicFriction
        {
            get { return Internal_GetDynamicFriction(mCachedPtr); }
            set { Internal_SetDynamicFriction(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls "bounciness" of an object during a collision. Value of 1 means the collision is elastic, and value
        /// of 0 means the value is inelastic. Must be in [0, 1] range.
        /// </summary>
        public float Restitution
        {
            get { return Internal_GetRestitution(mCachedPtr); }
            set { Internal_SetRestitution(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(PhysicsMaterial instance, float staticFriction,
            float dynamicFriction, float restitution);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetStaticFriction(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStaticFriction(IntPtr thisPtr, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetDynamicFriction(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDynamicFriction(IntPtr thisPtr, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRestitution(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRestitution(IntPtr thisPtr, float value);
    }

    /** @} */
}
