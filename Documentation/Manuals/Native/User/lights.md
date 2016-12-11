Lights						{#lights}
===============

Lights control the rendering of the nearby **Renderable** objects, by attempting to simulate how a real light would affect them. They are represented by the @ref bs::CLight "Light" component.

# Creating a light
**Light** is created as any component, and requires no additional parameters.

~~~~~~~~~~~~~{.cpp}
HSceneObject lightSO = SceneObject::create("Light");
HLight light = lightSO->addComponent<CLight>();
~~~~~~~~~~~~~

Once created light can be positioned and oriented normally using its **SceneObject**.

# Light types
Lights have three variants that determine how light affects surrounding objects: point, spot and directional. You can change between light variants with @ref bs::CLight::setType "CLight::setType".

~~~~~~~~~~~~~{.cpp}
// Make a point light
light->setType(LightType::Point);
~~~~~~~~~~~~~

## Point light

Point light affects everything within a certain radius from the light (i.e. its sphere of influence). It is the most basic type of light.

@ref TODO_IMAGE

## Spot light

Spot lights only illuminate a certain direction (i.e. like a flash-light). The user can specify the angle of illumination.

@ref TODO_IMAGE

## Directional light

Directional lights are a special type of light primarily used for simulating sun-light, or otherwise far-away objects. Unlike point or spot lights they cannot be positioned. Only their orientation matters and they equally affect every object in the scene.

@ref TODO_IMAGE

# Light properties
Each light has a set of properties you can adjust (aside from position/orientation, which are handled by the **SceneObject**). Most of the properties are common for all light types but some are only relevant for specific types. We'll go over all of the properties below.

## Color
As the name implies, this controls what color light is being emitted from the source. It can be set by calling @ref bs::CLight::setColor "CLight::setColor" and it is valid for all light types.

## Range
Range determines how far away does the light's influence reach. For point lights this is the radius of the sphere of influence, and for spot lights this is the distance from the origin in the direction of the light. This property is not relevant for directional lights as their range is infinite. Use @ref bs::CLight::setRange "CLight::setRange" to set the range.

Note that the manually set range will only be used if physically based attenuation is disabled. You can toggle this by calling @ref bs::CLight::setPhysicallyBasedAttenuation "CLight::setPhysicallyBasedAttenuation". When physically based attenuation is enabled the maximum range is calculated automatically based on the light intensity (described below) and using a real-world light attenuation model. You might assume that this is always wanted, but sometimes the designer might want more control in which case setting the range manually might be preferable.

## Intensity
This controls how strong is the light. Although you could technically control light intensity using the color property (by using a lighter or darker color), using the intensity allows the engine to simulate high-dynamic range. 

In nature the range of light intensities varies highly - standing outside at a sunlit day may be hundreds or thousands of times brighter than standing indoors illuminated by a lightbulb. We won't perceive such a large difference because our eyes are able to adjust to different intensities.

Banshee uses a HDR algorithm to try to approximate this adjustment, which results in more realistic and higher quality lighting. Therefore it can be important for realism to set up the light intensities similar to what they would be in nature. 

Use @ref bs::CLight::setIntensity "CLight::setIntensity" to change the light intensity. 

The specific units used for intensity (in case you want to look them up for actual light sources) are *luminous flux* for point/spot lights, and *radiance* for directional lights.

## Shadows
Lights may or may not cast shadows. For realism all lights should cast shadows, but that is not feasible due to the high performance costs of using shadows. Therefore you should enable shadows only for one, or a few important lights. Use @ref bs::CLight::setCastsShadow "CLight::setCastsShadow" to enable or disable shadow casting.

## Spot angles
Spot light have a property that defines at how wide an angle do they cast light in. Narrower angle means a more focused light beam, while wider angle means a weaker light covering a larger area. 

You can set the spot light angle (also knows as total angle) with @ref bs::CLight::setSpotAngle "CLight::setSpotAngle". Note that light intensity will be spread out over the range, so when increasing the angle, you might also want to increase the intensity to keep the perceived brightness the same.

You can also call @ref bs::CLight::setSpotFalloffAngle "CLight::setSpotFalloffAngle" to set an angle that allows a smooth falloff at the edge of the lights radius. This must be an angle smaller than the total spot angle. Within this angle the light intensity will be uniform, but as the angle increases past the falloff angle and towards the total angle and intensity will slowly reduce, resulting in a smooth falloff curve. If you don't want the smooth falloff, set this angle to the same value as the total angle.

@ref TODO_IMAGE