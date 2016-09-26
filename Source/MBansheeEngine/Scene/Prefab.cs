//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Scene
     *  @{
     */

    /// <summary>
    /// Prefab is a saveable hierarchy of scene objects. In general it can serve as any grouping of scene objects 
    /// (for example a level) or be used as a form of a template instantiated and reused throughout the scene.
    /// </summary>
    public class Prefab : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private Prefab()
        { }

        /// <summary>
        /// Creates a new prefab from the provided scene object. If the scene object has an existing prefab link it will 
        /// be broken. After the prefab is created the scene object will be automatically linked to it.
        /// </summary>
        /// <param name="so">Scene object to generate the prefab for.</param>
        /// <param name="isScene">Determines if the prefab represents a scene or just a generic group of objects.
        ///                       <see cref="IsScene"/></param>
        public Prefab(SceneObject so, bool isScene = true)
        {
            IntPtr soPtr = so.GetCachedPtr();
            Internal_CreateInstance(this, soPtr, isScene);
        }

        /// <summary>
        /// Instantiates a prefab by creating an instance of the prefab's scene object hierarchy. The returned hierarchy 
        /// will be parented to world root by default.
        /// </summary>
        /// <returns>New scene object instance containing a copy of the prefab's hierarchy.</returns>
        public SceneObject Instantiate()
        {
            return Internal_Instantiate(mCachedPtr);
        }

        /// <summary>
        /// Determines if the prefab represents a scene or just a generic group of objects. The only difference between the
        /// two is the way root object is handled: scenes are assumed to be saved with the scene root object (which is 
        /// hidden), while object group root is a normal scene object (not hidden). This is relevant when when prefabs are
        /// loaded, so the systems knows to append the root object to non-scene prefabs.
        /// </summary>
        public bool IsScene
        {
            get { return Internal_IsScene(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(Prefab instance, IntPtr so, bool isScene);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_Instantiate(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsScene(IntPtr thisPtr);
    }

    /** @} */
}
