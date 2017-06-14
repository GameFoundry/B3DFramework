Surface & lighting shaders					{#surfaceShaders}
===============

So far we have shown how to create a BSL shader from scratch. But when working with Banshee's default renderer there is a lot going on, and your shaders would need to be quite complex and fully compliant with what the renderer expects. This is not something a normal user wants, or needs to deal with.

For this reason Banshee provides a couple of **mixin**s you can override. By overriding these mixins you can change the renderer's behaviour without having to deal with majority of its complexities.




TODO - Surface shaders
 - Refactor Transparent.bsl so lighting calc is in some mixin, and can just be called by the surface shader

TODO - Lighting shaders
 - Rename LightingCommon -> Lighting. Refactor it so only the minimum subset of code needs to be overriden
