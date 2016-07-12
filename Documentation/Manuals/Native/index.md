Banshee Engine Documentation (Native)						{#mainpage}
===============

[TOC]

Welcome to the documentation for the native (C++) API of the Banshee Engine.

This documentation contains a set of [manuals](@ref manuals) and an API reference for all native types.

# Manuals # {#mainpage_a}
Manuals should be your primary entry point into learning about Banshee. They will provide you with a view of the general architecture of the engine, as well as the architecture of the more important systems. They will also teach you have to extend/modify various parts of the engine, including adding custom GUI types, script objects, resources, importers, renderers and many more.
 
The manuals generally do not cover user-facing functionality, and focus more on explaining the engine internals. To learn about user-facing functionality read the [scripting documentation](@ref TODO_LINK), the C++ interfaces are very similar and are usually just a direct reflection of their script counterparts.
 
[Visit](@ref manuals) 
 
# API Reference # {#mainpage_b}
API reference provides a categorized and hierarchical view of all the engine's classes. 

All classes are categorized into three primary groups:
 - <a class="el" href="group___layers.html">Core API</a> - Contains documentation for the user-facing API of the engine core, categorized per layer. Each layer is built directly on top of the previous one. This is what most users will be interested in. 
 - <a class="el" href="group___internals.html">Internals</a> - Reference documentation for internals of the engine core, categorized per layer. Primarily useful for those modifying the engine but of less relevance for normal users.
 - <a class="el" href="group___plugins.html">Plugins</a> - Reference documentation for all available plugins. Plugins are various interchangeable libraries that contain high level systems built on top of abstractions defined in the engine core. If you are modifying the engine you might be interested in this documentation, but it can be skipped for most normal users.
 
You should read the [architecture](@ref architecture) manual for a more detailed breakdown of the architecture.
