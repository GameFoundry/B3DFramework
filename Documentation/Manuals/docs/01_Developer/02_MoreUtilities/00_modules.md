Modules									{#modules}
===============
[TOC]

A @ref bs::Module<T> "Module<T>" is a specialized form of singleton used for many of bs::f systems. Unlike standard singletons it requires manual startup and shutdown. To use it for your own objects, simply inherit from it and provide your own class as its template parameter.

~~~~~~~~~~~~~{.cpp}
class MyModule : public Module<MyModule>
{ };
~~~~~~~~~~~~~

Use @ref bs::Module<T>::startUp "Module<T>::startUp()" to start it up. Once started use @ref bs::Module<T>::instance "Module<T>::instance()" to access its instance. Once done with it call @ref bs::Module<T>::shutDown "Module<T>::shutDown()" to release it.

~~~~~~~~~~~~~{.cpp}
MyModule::startUp();
MyModule::instance().doSomething();
MyModule::shutDown();
~~~~~~~~~~~~~