//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    /// <summary>
    /// Renderable represents any visible object in the scene. It has a mesh, bounds and a set of materials. Renderer will 
    /// render any Renderable objects visible by a camera.
    /// </summary>
    [RunInEditor]
    public sealed class Renderable : Component
    {
        private NativeRenderable _native;
        private Animation animation;

        [SerializeField]
        private SerializableData serializableData = new SerializableData();

        /// <summary>
        /// Returns the non-component version of Renderable that is wrapped by this component. 
        /// </summary>
        internal NativeRenderable Native
        {
            get { return _native; }
        }

        /// <summary>
        /// Mesh to render. 
        /// </summary>
        public Mesh Mesh
        {
            get { return _native.Mesh; }
            set 
            { 
                _native.Mesh = value; 
                serializableData.mesh = value;

                int subMeshCount = 0;
                if (value != null)
                    subMeshCount = value.SubMeshCount;

                Material[] newMaterials = new Material[subMeshCount];
                int numToCopy = MathEx.Min(newMaterials.Length, serializableData.materials.Length);
                Array.Copy(serializableData.materials, newMaterials, numToCopy);
                serializableData.materials = newMaterials;

                if (animation != null)
                    animation.UpdateBounds(false);
            }
        }

        /// <summary>
        /// Material to use when rendering the mesh. If the mesh contains multiple sub-meshes then you may set individual
        /// materials for each sub-mesh.
        /// </summary>
        public Material Material
        {
            get { return _native.GetMaterial(0); }
            set 
            { _native.SetMaterial(value); serializableData.materials[0] = value; }
        }

        /// <summary>
        /// Materials to use when rendering the mesh. 
        /// </summary>
        public Material[] Materials
        {
            get { return _native.Materials; }
            set
            {
                _native.Materials = value;

                serializableData.materials = new Material[value.Length];
                Array.Copy(value, serializableData.materials, value.Length);
            }
        }

        /// <summary>
        /// Returns a material for a specific sub-mesh.
        /// </summary>
        /// <param name="index">Index of the sub-mesh.</param>
        /// <returns>Material used for rendering the sub-mesh at the specified index.</returns>
        public Material GetMaterial(int index = 0)
        {
            return _native.GetMaterial(index);
        }

        /// <summary>
        /// Sets a material for a specific sub-mesh.
        /// </summary>
        /// <param name="material">Material to use for rendering the sub-mesh at the specified index.</param>
        /// <param name="index">Index of the sub-mesh.</param>
        public void SetMaterial(Material material, int index = 0)
        {
            _native.SetMaterial(material, index);
            serializableData.materials[index] = material;
        }

        /// <summary>
        /// Layer bitfield that controls whether a renderable is considered visible in a specific camera. Renderable layer 
        /// must match camera layer in order for the camera to render the component.
        /// </summary>
        public UInt64 Layers
        {
            get { return _native.Layers; }
            set { _native.Layers = value; serializableData.layers = value; }
        }

        /// <summary>
        /// Gets world bounds of the mesh rendered by this object.
        /// </summary>
        public Bounds Bounds
        {
            get { return _native.GetBounds(SceneObject); }
        }

        private void OnInitialize()
        {
            animation = SceneObject.GetComponent<Animation>();
            if (animation != null)
            {
                RegisterAnimation(animation);
                animation.RegisterRenderable(this);
            }
        }

        private void OnReset()
        {
            if (_native != null)
                _native.OnDestroy();

            _native = new NativeRenderable(SceneObject);

            // Restore saved values after reset
            _native.Mesh = serializableData.mesh;

            if (serializableData.materials != null)
                _native.Materials = serializableData.materials;

            _native.Layers = serializableData.layers;
        }

        private void OnUpdate()
        {
            _native.UpdateTransform(SceneObject, false);
        }

        private void OnDestroy()
        {
            if (animation != null)
                animation.UnregisterRenderable();

            _native.OnDestroy();
        }

        /// <summary>
        /// Registers an <see cref="Animation"/> component with the renderable. Rendering will be affected by the animation.
        /// </summary>
        /// <param name="animation">Component that was added</param>
        internal void RegisterAnimation(Animation animation)
        {
            this.animation = animation;

            if (_native != null)
            {
                _native.Animation = animation.Native;

                // Need to update transform because animated renderables handle local transforms through bones, so it
                // shouldn't be included in the renderable's transform.
                _native.UpdateTransform(SceneObject, true);
            }
        }

        /// <summary>
        /// Removes animation from the renderable. Rendering will no longer be affected by animation.
        /// </summary>
        internal void UnregisterAnimation()
        {
            animation = null;

            if (_native != null)
            {
                _native.Animation = null;

                // Need to update transform because animated renderables handle local transforms through bones, so it
                // shouldn't be included in the renderable's transform.
                _native.UpdateTransform(SceneObject, true);
            }
        }

        /// <inheritdoc/>
        protected internal override bool CalculateBounds(out AABox box, out Sphere sphere)
        {
            Bounds bounds = Bounds;

            box = bounds.Box;
            sphere = bounds.Sphere;

            return true;
        }

        /// <summary>
        /// Holds all data the renderable component needs to persist through serialization.
        /// </summary>
        [SerializeObject]
        private class SerializableData
        {
            public Mesh mesh;
            public Material[] materials = new Material[1];
            public UInt64 layers = 1;
        }
    }

    /** @} */
}
