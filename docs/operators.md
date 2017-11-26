Operator overloading
====================

assignment

```
a = b
a += b
a -= b
a *= b
a /= b
a %= b
a &= b
a |= b
a ^= b
a <<= b
a >>= b
```

increment/decrement

```
++a
--a
a++
a--
```

arithmetic

```
+a
-a
a + b
a - b
a * b
a / b
a % b
~a
a & b
a | b
a ^ b
a << b
a >> b
```

logical (the overloads of operators `&&` and `||` lose short-circuit evaluation)

```
!a
a && b
a || b
```

comparison

```
a == b
a != b
a < b
a > b
a <= b
a >= b
a <=> b
```

member access

```
a[b]
*a
&a
a->b
a.b
a->*b
a.*b
```

function call, comma, type conversion

```
a()
a,b
T()
```

allocation

```
new
new[]
delete
delete[]
```

Cannot be overloaded

```
a.b
a.*b
a?b:c
a::b
sizeof(a)
typeid(a)
```

Reference
=========

[Operator Overloading](https://en.wikibooks.org/wiki/C%2B%2B_Programming/Operators/Operator_Overloading)

[Operators](http://en.cppreference.com/w/cpp/language/operators)
