Flags									{#flags}
===============
[TOC]

@ref bs::Flags<Enum, Storage> "Flags<Enum, Storage>" provide a wrapper around an `enum` and allow you to easily perform bitwise operations on them without having to cast to integers. For example when using raw C++ you must do something like this:
~~~~~~~~~~~~~{.cpp}
enum class MyFlag
{
	Flag1 = 1<<0,
	Flag2 = 1<<1,
	Flag3 = 1<<2
};

MyFlag combined = (MyFlag)((UINT32)MyFlag::Flag1 | (UINT32)MyFlag::Flag2);
~~~~~~~~~~~~~

Which is cumbersome. Flags require an additional step to define the enum, but after that allow you to manipulate values much more nicely. 

To create a **Flags<Enum, Storage>** type for an enum simply define a `typedef` with your enum type provided as the template parameter. You must also follow that definition with a @ref BS_FLAGS_OPERATORS macro in order to ensure all operators are properly defined.
~~~~~~~~~~~~~{.cpp}
typedef Flags<MyFlag> MyFlags;
BS_FLAGS_OPERATORS(MyFlag)
~~~~~~~~~~~~~

Now you can do something like this:
~~~~~~~~~~~~~{.cpp}
MyFlags combined = MyFlag::Flag1 | MyFlag::Flag2;
~~~~~~~~~~~~~