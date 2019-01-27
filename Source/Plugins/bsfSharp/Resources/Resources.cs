//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Resources
     *  @{
     */

    /// <summary>
    /// Handles dynamic loading of resources during runtime. 
    /// </summary>
    public static class Resources
    {
        /// <summary>
        /// Loads a resource at the specified path. If resource is already loaded an existing instance is returned.
        /// </summary>
        /// <remarks>
        /// If running outside of the editor you must make sure to mark that the resource gets included in the build. If
        /// running inside the editor this has similar functionality as if loading using the project library.
        /// </remarks>
        /// <typeparam name="T">Type of the resource.</typeparam>
        /// <param name="path">Path of the resource, relative to game directory. If running from editor this will be
        ///                    relative to the project library. If a sub-resource within a file is needed, append the name
        ///                    of the subresource to the path (for example mymesh.fbx/my_animation).</param>
        /// <param name="flags">Flags used to control the load process.</param>
        /// <returns>Loaded resource, or null if resource cannot be found.</returns>
        public static T Load<T>(string path, ResourceLoadFlag flags = ResourceLoadFlag.Default) where T : Resource
        {
            return (T)Internal_Load(path, flags);
        }

        /// <summary>
        /// Loads a resource with the specified UUID. If resource is already loaded an existing instance is returned.
        /// </summary>
        /// <remarks>
        /// If running outside of the editor you must make sure to mark that the resource gets included in the build. If
        /// running inside the editor this has similar functionality as if loading using the project library.
        /// </remarks>
        /// <typeparam name="T">Type of the resource.</typeparam>
        /// <param name="uuid">Unique identifier of the resource to load.</param>
        /// <param name="flags">Flags used to control the load process.</param>
        /// <returns>Loaded resource, or null if resource cannot be found.</returns>
        public static T Load<T>(UUID uuid, ResourceLoadFlag flags = ResourceLoadFlag.Default) where T : Resource
        {
            return (T)Internal_LoadFromUUID(ref uuid, flags);
        }

        /// <summary>
        /// Loads a resource at the specified path asynchonously (on a separate thread). If resource is already loaded
        /// an existing instance is returned. Use <see cref="RRefBase.IsLoaded"/> to confirm the resource has been loaded
        /// before using it. Use <see cref="GetLoadProgress"/> to track the loading progress of the resource.
        /// </summary>
        /// <remarks>
        /// If running outside of the editor you must make sure to mark that the resource gets included in the build. If
        /// running inside the editor this has similar functionality as if loading using the project library.
        /// </remarks>
        /// <typeparam name="T">Type of the resource.</typeparam>
        /// <param name="path">Path of the resource, relative to game directory. If running from editor this will be
        ///                    relative to the project library. If a sub-resource within a file is needed, append the name
        ///                    of the subresource to the path (for example mymesh.fbx/my_animation).</param>
        /// <param name="flags">Flags used to control the load process.</param>
        /// <returns>Loaded resource, or null if resource cannot be found.</returns>
        public static RRef<T> LoadAsync<T>(string path, ResourceLoadFlag flags = ResourceLoadFlag.Default) where T : Resource
        {
            return Internal_LoadAsync(path, flags).As<T>();
        }

        /// <summary>
        /// Loads a resource with the specified UUID asynchonously (on a separate thread). If resource is already loaded
        /// an existing instance is returned. Use <see cref="RRefBase.IsLoaded"/> to confirm the resource has been loaded
        /// before using it. Use <see cref="GetLoadProgress"/> to track the loading progress of the resource.
        /// </summary>
        /// <remarks>
        /// If running outside of the editor you must make sure to mark that the resource gets included in the build. If
        /// running inside the editor this has similar functionality as if loading using the project library.
        /// </remarks>
        /// <typeparam name="T">Type of the resource.</typeparam>
        /// <param name="uuid">Unique identifier of the resource to load.</param>
        /// <param name="flags">Flags used to control the load process.</param>
        /// <returns>Loaded resource, or null if resource cannot be found.</returns>
        public static RRef<T> LoadAsync<T>(UUID uuid, ResourceLoadFlag flags = ResourceLoadFlag.Default) where T : Resource
        {
            return Internal_LoadAsyncFromUUID(ref uuid, flags).As<T>();
        }

        /// <summary>
        /// Releases an internal reference to the resource held by the resources system. This allows the resource
        ///	to be unloaded when it goes out of scope, if the resource was loaded with "keepLoaded" parameter.
        ///
        /// Alternatively you can also skip manually calling <see cref="Release(RRefBase)"/> and call <see cref="UnloadUnused"/> 
        /// which will unload all resources that do not have any external references, but you lose the fine grained control 
        /// of what will be unloaded.
        /// </summary>
        /// <param name="resource">Resource to release</param>
        public static void Release(RRefBase resource)
        {
            if (resource == null)
                return;

            Internal_ReleaseRef(resource.GetCachedPtr());
        }

        /// <summary>
        /// Releases an internal reference to the resource held by the resources system. This allows the resource
        ///	to be unloaded when it goes out of scope, if the resource was loaded with "keepLoaded" parameter.
        ///
        /// Alternatively you can also skip manually calling <see cref="Release(Resource)"/> and call <see cref="UnloadUnused"/> 
        /// which will unload all resources that do not have any external references, but you lose the fine grained control 
        /// of what will be unloaded.
        /// </summary>
        /// <param name="resource">Resource to release</param>
        public static void Release(Resource resource)
        {
            if(resource == null)
                return;

            Internal_Release(resource.GetCachedPtr());
        }

        /// <summary>
        /// Unloads all resources that are no longer referenced. This only applies to resources loaded with "keepLoaded"
        /// parameter, as all other resources will be unloaded when they go out of scope.
        /// </summary>
        public static void UnloadUnused()
        {
            Internal_UnloadUnused();
        }

        /// <summary>
        /// Returns the loading progress of a resource that's being asynchronously loaded. 
        /// </summary>
        /// <param name="resource">Resource whose load progress to check.</param>
        /// <param name="includeDependencies">If false the progress will reflect the load progress only for this individual
        ///                                   resource. If true the progress will reflect load progress of this resource
        ///                                   and all of its dependencies.</param>
        /// <returns>Load progress in range [0, 1].</returns>
        public static float GetLoadProgress(RRefBase resource, bool includeDependencies = true)
        {
            if(resource == null)
                return 0.0f;

            return Internal_GetLoadProgress(resource.GetCachedPtr(), includeDependencies);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Resource Internal_Load(string path, ResourceLoadFlag flags);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Resource Internal_LoadFromUUID(ref UUID uuid, ResourceLoadFlag flags);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RRefBase Internal_LoadAsync(string path, ResourceLoadFlag flags);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RRefBase Internal_LoadAsyncFromUUID(ref UUID uuid, ResourceLoadFlag flags);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetLoadProgress(IntPtr resource, bool loadDependencies);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Release(IntPtr resource);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ReleaseRef(IntPtr resource);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UnloadUnused();
    }

    /** @} */
}
