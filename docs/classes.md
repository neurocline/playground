C++ Classes
===========

Construction and destruction
----------------------------

### Default and deleted special member functions

There are 6 special member functions that have default behavior if not
defined. If defined, the signatures are as follows:

```
class SpecialMembers
{
public:
    // Default constructor
    SpecialMembers();

    // Destructor
    ~SpecialMembers() noexcept;

    // Copy constructor
    SpecialMembers(const SpecialMembers& rhs) noexcept;

    // Move constructor
    SpecialMembers(SpecialMembers&& rhs) noexcept;

    // Copy assignment operator
    SpecialMembers& operator=(const SpecialMembers& rhs) noexcept;

    // Move assignment operator
    SpecialMembers& operator=(SpecialMember&& rhs) noexcept;
};
```

All except the constructor and destructor should be declared as `noexcept` in 
C++11 and beyond; this is important to enable optimizations in containers. The
destructor should almost always be declared as `noexcept` as well. The implicitly
defined versions of these are usually `noexcept`, although there is a moderately
complex set of rules to define this.

You can explicitly get the default behavior. This has no effect except as
documentation, with one exception; a class with user-defined constructors
will not automatically define a default constructor. In this case, using the
`default` keyword to bring it into existence is needed if the default constructor
is needed.

```
class DefaultMembers
{
public:
    DefaultMembers() = default;
    ~DefaultMembers() = default;
    DefaultMembers(const DefaultMembers& rhs) = default;
    DefaultMembers(DefaultMembers&& rhs) = default;
    DefaultMembers& operator=(const DefaultMembers& rhs) = default;
    DefaultMembers& operator=(DefaultMembers&& rhs) = default;
};
```

You can delete one or more special members. This prevents them from being
called (specifically, a compile error will be generated if one of the special
members is called). Uses for this include objects that cannot be copied but
can be moved, or objects that cannot be default-constructed.

A class with a deleted destructor is an interesting case; if there is no
accessible destructor, the compiler refuses to compile code that allocates
an instance with automatic storage (stack). These can be created on the heap,
because there is no automatic deletion, because there is no scope to leave.

```
class DeletedMembers
{
public:
    DeletedMembers() = delete;
    ~DeletedMembers() = delete;
    DeletedMembers(const DeletedMembers& rhs) = delete;
    DeletedMembers(DeletedMembers&& rhs) = delete;
    DeletedMembers& operator=(const DeletedMembers& rhs) = delete;
    DeletedMembers& operator=(DeletedMembers&& rhs) = delete;
};
```

### Copy constructor

The implicitly-declared copy constructor always has the form `T(const T&)`.

Multiple copy constructor operators can be defined, as variants of `const`
and `volatile`.

```
class CopyConstruct
{
public:
    CopyConstruct(CopyConstruct rhs) noexcept;
    CopyConstruct(CopyConstruct& rhs) noexcept;
    CopyConstruct(const CopyConstruct& rhs) noexcept;
    CopyConstruct(volatile CopyConstruct& rhs) noexcept;
    CopyConstruct(const volatile CopyConstruct& rhs) noexcept;
};
```

### Copy assignment operator

The implicitly-declared copy assignment operator always has the form
`T& T::operator=(const T&)`.

Multiple copy assignment operators can be defined, as variants of `const`
and `volatile`.

```
class CopyAssign
{
public:
    CopyAssign& operator=(CopyAssign rhs) noexcept;
    CopyAssign& operator=(CopyAssign& rhs) noexcept;
    CopyAssign& operator=(const CopyAssign& rhs) noexcept;
    CopyAssign& operator=(volatile CopyAssign& rhs) noexcept;
    CopyAssign& operator=(const volatile CopyAssign& rhs) noexcept;
};
```

The copy assignment operator almost always returns a reference (or sometimes
a const reference). If you return by value, you will incur an extra constructor
and destructor call, and you cannot then use operator chaining (`a = b += c`),
and certain rarely-used sentences (`(a = b) = 666`).

### Move constructor

The implicitly-declared copy constructor always has the form `T(T&&)`.

Multiple move constructor operators can be defined, as variants of `const`
and `volatile`.

```
class MoveConstruct
{
public:
    MoveConstruct(MoveConstruct rhs) noexcept;
    MoveConstruct(MoveConstruct& rhs) noexcept;
    MoveConstruct(const MoveConstruct& rhs) noexcept;
    MoveConstruct(volatile MoveConstruct& rhs) noexcept;
    MoveConstruct(const volatile MoveConstruct& rhs) noexcept;
};
```

### Move assignment operator

The implicitly-declared move assignment operator always has the form
`T& T::operator=(T&&)`.

Multiple move constructor operators can be defined, as variants of `const`
and `volatile`. Obviously, a move assignment operator does not take a
parameter by value; this would go against the mechanics of move.

```
class MoveAssign
{
public:
    MoveAssign& operator=(MoveAssign&& rhs) noexcept;
    MoveAssign& operator=(const MoveAssign&& rhs) noexcept;
    MoveAssign& operator=(volatile MoveAssign&& rhs) noexcept;
    MoveAssign& operator=(const volatile MoveAssign&& rhs) noexcept;
};
```

If both copy and move assignment operators are provided, then overload resolution
selects the move assignment operator if the argument is an rvalue, and selects
the copy assignment operator if the argument is an lvalue.

An rvalue is either a prvalue (such as a nameless temporary) or an xvalue
(such as the result of `std::move`).

An lvalue is a named object or a function/operator returning an lvalue reference.

Copy assignment is the fallback for move assignment, when move is unavailable,
because rvalues can bind to const references.

### Rule of Three

Rule of Three: if a class defines one or more of a destructor, copy constructor,
or copy assignment operator, it should define all three. This is a pre-C++11 rule.

```
class RuleOfThree
{
public:
    // Destructor
    ~RuleOfThree();

    // Copy constructor
    RuleOfThree(const RuleOfThree& rhs) noexcept;

    // Copy assignment operator
    RuleOfThree& operator=(const RuleOfThree& rhs) noexcept;
}
```

The copy constructor and copy assignment operator are always `noexcept` in
conforming C++11 code.

Only declare destructor, copy constructor or copy assignment operator if the
default behavior isn't appropriate. Default behavior:

- destructor - Call the destructors of the object's class-type members
- copy constructor - Construct all the object's members from the corresponding members of the copy constructor's argument, calling the copy constructors of the object's class-type members, and doing a plain assignment of all non-class type (e.g., int or pointer) data members
- copy assignment operator - Assign all the object's members from the corresponding members of the assignment operator's argument, calling the copy assignment operators of the object's class-type members, and doing a plain assignment of all non-class type (e.g. int or pointer) data members

The rule of three comes from the observation that if one of the default behaviors
isn't appropriate for one of the three of destructor, copy constructor, or
copy assignment operator, then it's probably not appropriate for all of them.

There may be an exception when it comes to the destructor, if the destructor is
doing more than just destroying each member of the object. If this is true,
though, it could mean that your class isn't designed properly.

### Rule of Two

If the class is designed following the RAII pattern, then the default destructor
is probably sufficient, even though you have a custom copy constructor and
copy assignment operator.

Classes with smart pointer members still need custom copy constructors and
copy assignment operators, so that forwarding of constructor calls can be
done.

### Rule of Five

With C++11, move constructor and move assignment operator are added to the list.
Subsets are possible; you can have objects that can't be moved or can be moved
but not copied.

```
class RuleOfFive
{
public:
    // Destructor
    ~RuleOfFive();

    // Copy constructor
    RuleOfFive(const RuleOfFive& rhs) noexcept;

    // Move constructor
    RuleOfFive(RuleOfFive&& rhs) noexcept;

    // Copy assignment operator
    RuleOfFive& operator=(const RuleOfFive& rhs) noexcept;

    // Move assignment operator
    RuleOfFive& operator=(RuleOfFive&& rhs) noexcept;
}
```

# Reference

[How does =delete on destructor prevents allocation?](https://stackoverflow.com/questions/18847739/how-does-delete-on-destructor-prevents-allocation)

[Classes (cppreference.com)](http://en.cppreference.com/w/cpp/language/classes)
