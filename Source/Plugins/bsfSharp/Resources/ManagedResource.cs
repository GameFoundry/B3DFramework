//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.CompilerServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Resources
     *  @{
     */

    /// <summary>
    /// Base class for all user-defined managed resources. Managed resources are automatically serialized, can be saved
    /// and persistently referenced by other objects.
    /// </summary>
    public class ManagedResource : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        protected ManagedResource()
        { }

        /// <summary>
        /// Creates a new managed resource.
        /// </summary>
        /// <typeparam name="T">Type of the managed resource to create.</typeparam>
        /// <returns>A new instance of a managed resource with default values.</returns>
        static public T Create<T>() where T : ManagedResource, new()
        {
            T newResource = new T();
            Internal_CreateInstance(newResource);

            return newResource;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(ManagedResource resource);
    }

    /** @} */
}
