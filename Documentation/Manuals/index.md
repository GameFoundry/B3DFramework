bs::framework Documentation						{#mainpage}
===============

Welcome to the documentation for bs::framework. The documentation can be split into two sections:
 - **Manuals** guiding you through a variety of systems with code examples
 - **API reference** documenting all the methods and classes in the framework

# Manuals # {#mainpage_a}
Manuals should be your primary entry point into learning about the framework. They can be split into two main categories:
 - [User manuals](@ref manuals) - An ordered set of manuals guiding you from basics through all the important systems and teaching you how to create an application using bs::f. This is where you should start.
 - [Developer manuals](@ref devManuals) - An advanced set of manuals intended for those that are extending or modifying the framework. They provide information about the general architecture of the framework, as well as internal workings of the framework core systems.
 
# API Reference # {#mainpage_b}
API reference provides a documented list of all methods and classes in the framework. The entries are neatly categorized so you can browse the API even if you are not exactly sure what you are looking for. 

All entries are categorized into three primary groups:
 - <a class="el" href="group___layers.html">Core API</a> - Contains documentation for the user-facing API. This represents the main API that normal users will work with. This is the API referenced in "User manuals".
 - <a class="el" href="group___internals.html">Internals</a> - Reference documentation for internals of the framework core. Primarily useful for those modifying the framework but of less relevance for normal users.
 - <a class="el" href="group___plugins.html">Plugins</a> - Reference documentation for all available plugins. Plugins are various interchangeable libraries that contain high level systems built on top of abstractions defined in the framework core. If you are modifying the framework you might be interested in this documentation, but it can be skipped for normal users.