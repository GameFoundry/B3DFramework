//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @cond INTEROP */

    /// <summary>
    /// Wrapper around the native Renderable class.
    /// <see cref="Renderable"/>
    /// </summary>
    internal class NativeRenderable : ScriptObject
    {
        internal Mesh Mesh
        {
            get
            {
                return mesh; 
            }
            set
            {
                mesh = value;

                IntPtr meshPtr = IntPtr.Zero;
                if (mesh != null)
                    meshPtr = mesh.GetCachedPtr();

                Internal_SetMesh(mCachedPtr, meshPtr); 
                
            }
        }

        internal Bounds GetBounds(SceneObject parent)
        {
            AABox box;
            Sphere sphere;

            Internal_GetBounds(mCachedPtr, parent.mCachedPtr, out box, out sphere);

            return new Bounds(box, sphere);
        }

        internal UInt64 Layers
        {
            get { return Internal_GetLayers(mCachedPtr); }
            set { Internal_SetLayers(mCachedPtr, value); }
        }

        internal NativeAnimation Animation
        {
            set
            {
                IntPtr animationPtr = IntPtr.Zero;
                if (value != null)
                    animationPtr = value.GetCachedPtr();

                Internal_SetAnimation(mCachedPtr, animationPtr);
            }
        }

        private Material[] materials = new Material[1];
        private Mesh mesh;
        
        public NativeRenderable(SceneObject sceneObject)
        {
            IntPtr sceneObjPtr = IntPtr.Zero;
            if (sceneObject != null)
                sceneObjPtr = sceneObject.GetCachedPtr();

            Internal_Create(this, sceneObjPtr);
        }

        internal Material[] Materials
        {
            get
            {
                if (materials == null)
                    return null;

                Material[] copy = new Material[materials.Length];
                Array.Copy(materials, copy, materials.Length);

                return copy;
            }
            set
            {
                materials = new Material[value.Length];
                Array.Copy(value, materials, value.Length);

                Internal_SetMaterials(mCachedPtr, value);
            }
        }

        internal Material GetMaterial(int index = 0)
        {
            return materials[index];
        }

        internal void SetMaterial(Material material, int index = 0)
        {
            materials[index] = material;

            IntPtr materialPtr = IntPtr.Zero;
            if (material != null)
                materialPtr = material.GetCachedPtr();

            Internal_SetMaterial(mCachedPtr, materialPtr, index);
        }

        internal void UpdateTransform(SceneObject sceneObject)
        {
            Internal_UpdateTransform(mCachedPtr, sceneObject.mCachedPtr);
        }
        
        internal void OnDestroy()
        {
            Internal_OnDestroy(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(NativeRenderable instance, IntPtr parentSO);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAnimation(IntPtr thisPtr, IntPtr animation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UpdateTransform(IntPtr thisPtr, IntPtr parentSO);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMesh(IntPtr thisPtr, IntPtr mesh);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetBounds(IntPtr thisPtr, IntPtr parentSO, out AABox box, out Sphere sphere);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern UInt64 Internal_GetLayers(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLayers(IntPtr thisPtr, UInt64 layers);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMaterial(IntPtr thisPtr, IntPtr material, int index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMaterials(IntPtr thisPtr, Material[] materials);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_OnDestroy(IntPtr thisPtr);
    }

    /** @endcond */
}
