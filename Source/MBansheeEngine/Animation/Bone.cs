//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;

namespace BansheeEngine
{
    /** @addtogroup Animation
     *  @{
     */

    /// <summary>
    /// Component that maps animation for specific bone also be applied to the <see cref="SceneObject"/> this component
    /// is attached to. The component will attach to the first found parent <see cref="Animation"/> component.
    /// </summary>
    public class Bone : Component
    {
        [SerializeField]
        private string name;

        private Animation parent;

        /// <summary>
        /// Name of the bone to attach to.
        /// </summary>
        public string Name
        {
            get { return name; }
            set
            {
                if (name != value)
                {
                    name = value;

                    if (parent != null)
                        parent.NotifyBoneChanged(this);
                }
            }
        }

        private void OnInitialize()
        {
            NotifyFlags = TransformChangedFlags.Parent;
        }

        private void OnEnable()
        {
            UpdateParentAnimation();
        }

        private void OnDisable()
        {
            if (parent != null)
                parent.RemoveBone(this);

            parent = null;
        }

        private void OnDestroy()
        {
            if (parent != null)
                parent.RemoveBone(this);

            parent = null;
        }

        private void OnTransformChanged(TransformChangedFlags flags)
        {
            if (!SceneObject.Active)
                return;

            if ((flags & TransformChangedFlags.Parent) != 0)
                UpdateParentAnimation();
        }

        /// <summary>
        /// Attempts to find the parent <see cref="Animation"/> component and registers itself with it.
        /// </summary>
        private void UpdateParentAnimation()
        {
            SceneObject currentSO = SceneObject;
            while (currentSO != null)
            {
                Animation parent = currentSO.GetComponent<Animation>();
                if (parent != null)
                {
                    if (currentSO.Active)
                        SetParent(parent);
                    else
                        SetParent(null);

                    return;
                }

                currentSO = currentSO.Parent;
            }

            SetParent(null);
        }

        /// <summary>
        /// Changes the parent animation of this component.
        /// </summary>
        /// <param name="animation">New animation parent, can be null.</param>
        /// <param name="isInternal">If true the bone will just be changed internally, but parent animation will not be
        ///                          notified.</param>
        internal void SetParent(Animation animation, bool isInternal = false)
        {
            if (animation == parent)
                return;

            if (!isInternal)
            {
                if (parent != null)
                    parent.RemoveBone(this);

                if (animation != null)
                    animation.AddBone(this);
            }

            parent = animation;
        }
    }

    /** @} */
}