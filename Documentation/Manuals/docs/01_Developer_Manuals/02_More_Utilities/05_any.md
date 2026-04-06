---
title: Any
---

@b3d::Any is a specialized data type that allows you to store any kind of object in it. For example:
~~~~~~~~~~~~~{.cpp}
Any var1 = Vector<String>();

struct MyStruct { int a; };
Any var2 = MyStruct();
~~~~~~~~~~~~~

Use @b3d::AnyCast and @b3d::AnyCastRef to retrieve valid types from an **Any** variable.
~~~~~~~~~~~~~{.cpp}
Vector<String> val1 = AnyCast<Vector<String>>(var1);
MyStruct& val2 = AnyCastRef<MyStruct>(var2);
~~~~~~~~~~~~~
