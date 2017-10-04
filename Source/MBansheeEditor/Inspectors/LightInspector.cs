//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="Light"/> component.
    /// </summary>
    [CustomInspector(typeof(Light))]
    internal class LightInspector : Inspector
    {
        private GUIEnumField lightTypeField = new GUIEnumField(typeof(LightType), new LocEdString("Light type"));
        private GUIColorField colorField = new GUIColorField(new LocEdString("Color"));
        private GUIFloatField attRadiusField = new GUIFloatField(new LocEdString("Attenuation radius"));
        private GUIFloatField sourceRadiusField = new GUIFloatField(new LocEdString("Source radius"));
        private GUIFloatField intensityField = new GUIFloatField(new LocEdString("Intensity"));
        private GUISliderField spotAngleField = new GUISliderField(1, 180, new LocEdString("Spot angle"));
        private GUISliderField spotFalloffAngleField = new GUISliderField(1, 180, new LocEdString("Spot falloff angle"));
        private GUIToggleField autoAttenuationField = new GUIToggleField(new LocEdString("Use auto. attenuation"));
        private GUIToggleField castShadowField = new GUIToggleField(new LocEdString("Cast shadow"));

        private InspectableState modifyState;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            if (InspectedObject != null)
            {
                Light light = (Light)InspectedObject;

                lightTypeField.OnSelectionChanged += x =>
                {
                    light.Type = (LightType)x;

                    ToggleTypeSpecificFields((LightType) x, light.UseAutoAttenuation);
                };

                colorField.OnChanged += x =>
                {
                    light.Color = x;
                    MarkAsModified();
                    ConfirmModify();
                };

                attRadiusField.OnChanged += x => { light.AttenuationRadius = x; MarkAsModified(); };
                attRadiusField.OnConfirmed += ConfirmModify;
                attRadiusField.OnFocusLost += ConfirmModify;

                sourceRadiusField.OnChanged += x => { light.SourceRadius = x; MarkAsModified(); };
                sourceRadiusField.OnConfirmed += ConfirmModify;
                sourceRadiusField.OnFocusLost += ConfirmModify;

                intensityField.OnChanged += x => { light.Intensity = x; MarkAsModified(); };
                intensityField.OnConfirmed += ConfirmModify;
                intensityField.OnFocusLost += ConfirmModify;

                spotAngleField.OnChanged += x => { light.SpotAngle = (Degree)x; MarkAsModified(); };
                spotAngleField.OnFocusLost += ConfirmModify;

                spotFalloffAngleField.OnChanged += x => { light.SpotAngleFalloff = (Degree)x; MarkAsModified(); };
                spotFalloffAngleField.OnFocusLost += ConfirmModify;

                castShadowField.OnChanged += x =>
                {
                    light.CastsShadow = x;
                    MarkAsModified();
                    ConfirmModify();
                };

                autoAttenuationField.OnChanged += x =>
                {
                    light.UseAutoAttenuation = x;
                    ToggleTypeSpecificFields(light.Type, x);
                    MarkAsModified();
                    ConfirmModify();
                };

                Layout.AddElement(lightTypeField);
                Layout.AddElement(colorField);
                Layout.AddElement(intensityField);
                Layout.AddElement(attRadiusField);
                Layout.AddElement(sourceRadiusField);
                Layout.AddElement(spotAngleField);
                Layout.AddElement(spotFalloffAngleField);
                Layout.AddElement(autoAttenuationField);
                Layout.AddElement(castShadowField);

                ToggleTypeSpecificFields(light.Type, light.UseAutoAttenuation);
            }
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Light light = InspectedObject as Light;
            if (light == null)
                return InspectableState.NotModified;

            LightType lightType = light.Type;
            if (lightTypeField.Value != (ulong)lightType || autoAttenuationField.Value != light.UseAutoAttenuation)
                ToggleTypeSpecificFields(lightType, light.UseAutoAttenuation);

            lightTypeField.Value = (ulong)lightType;
            colorField.Value = light.Color;
            intensityField.Value = light.Intensity;
            attRadiusField.Value = light.AttenuationRadius;
            sourceRadiusField.Value = light.SourceRadius;
            spotAngleField.Value = light.SpotAngle.Degrees;
            spotFalloffAngleField.Value = light.SpotAngleFalloff.Degrees;
            autoAttenuationField.Value = light.UseAutoAttenuation;
            castShadowField.Value = light.CastsShadow;

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Enables or disables different GUI elements depending on the light type.
        /// </summary>
        /// <param name="type">Light type to show GUI elements for.</param>
        /// <param name="physBasedAttenuation">Determines is physically based attenuation enabled.</param>
        private void ToggleTypeSpecificFields(LightType type, bool physBasedAttenuation)
        {
            if (type == LightType.Directional)
            {
                attRadiusField.Active = false;
                spotAngleField.Active = false;
                spotFalloffAngleField.Active = false;
                autoAttenuationField.Active = false;
            }
            else if (type == LightType.Radial)
            {
                attRadiusField.Active = !physBasedAttenuation;
                spotAngleField.Active = false;
                spotFalloffAngleField.Active = false;
                autoAttenuationField.Active = true;
            }
            else
            {
                attRadiusField.Active = !physBasedAttenuation;
                spotAngleField.Active = true;
                spotFalloffAngleField.Active = true;
                autoAttenuationField.Active = true;
            }
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        protected void MarkAsModified()
        {
            modifyState |= InspectableState.ModifyInProgress;
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        protected void ConfirmModify()
        {
            if (modifyState.HasFlag(InspectableState.ModifyInProgress))
                modifyState |= InspectableState.Modified;
        }
    }

    /** @} */
}
