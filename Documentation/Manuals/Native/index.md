Banshee Framework Documentation (Native)						{#mainpage}
===============

[TOC]

Welcome to the documentation for the Banshee Framework.

This documentation contains a set of [manuals](@ref manuals) and an API reference for all native types.

# Manuals # {#mainpage_a}
Manuals should be your primary entry point into learning about Banshee. They can be split into two main categories:
 - User manuals - They explain how to use the provided API in order to create an application of your own, guiding you from basics through all the important systems. This is what most people will be interested in.
 - Developer manuals - They will provide you with a view of the general architecture of the framework, as well as the architecture of the more important systems. They will also teach you how to extend/modify various parts of the framework, including adding custom GUI types, script objects, resources, importers, renderers and many more. They are intended for those planning to extend or modify the framework in some way.

[Visit](@ref manuals) 
 
# API Reference # {#mainpage_b}
API reference provides a categorized and hierarchical view of all the framework's classes. 

All classes are categorized into three primary groups:
 - <a class="el" href="group___layers.html">Core API</a> - Contains documentation for the user-facing API of the framework core, categorized per layer. Each layer is built directly on top of the previous one. This is what most users will be interested in. 
 - <a class="el" href="group___internals.html">Internals</a> - Reference documentation for internals of the framework core, categorized per layer. Primarily useful for those modifying the framework but of less relevance for normal users.
 - <a class="el" href="group___plugins.html">Plugins</a> - Reference documentation for all available plugins. Plugins are various interchangeable libraries that contain high level systems built on top of abstractions defined in the framework core. If you are modifying the framework you might be interested in this documentation, but it can be skipped for most normal users.
 
You should read the [architecture](@ref architecture) manual for a more detailed breakdown of the architecture.
