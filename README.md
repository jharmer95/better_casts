# Better Casts

A header-only library providing improved type-checked versions of `static_cast`.

## License

This project is donated to the public domain under the Creative Commons CC0 license and is free to use for any purpose, including commercial applications.
See the [LICENSE](LICENSE) file for more details.

magic_enum (optional dependency) is licensed under the MIT license. See the [magic_enum LICENSE](https://github.com/Neargye/magic_enum/blob/master/LICENSE) file for more details.

## Benefits

- Provides compile-time checks for type safety.
- Reduces the risk of undefined behavior from invalid casts.
- Improves code readability and maintainability by making the intent of the cast clear.
- Makes it easier to search for casts in codebases by providing a consistent naming scheme.
- Limits the scope of casts to specific types, reducing the risk of accidental misuse or performing multiple operations on the same cast.

## Requirements

- C++14 or later
- MSVC++ 2015 or later
- GCC 5 or later
- Clang 3.5 or later

## Features

- `constexpr` compatible casts performing most checks at compile time.
- `_checked` and `_unchecked` variants for runtime checks.
  - Checked casts throw exceptions on failure.
  - By default, the generic version of casts (`enum_cast`, `float_cast`, etc.) are checked in debug builds and unchecked in release builds.
  - Can use the specific `_checked` or `_unchecked` versions to override this behavior (ex. `enum_cast_checked`, `float_cast_unchecked`).
  - Can override by defining either `ALWAYS_CHECK_CASTS` or `NEVER_CHECK_CASTS` to use the checked or unchecked versions, respectively.

## Provided Casts

### `enum_cast`

- Converts between enum types and their underlying types.
- Only allows conversion between an enum and an integer of the same or larger size and the same signedness.
- May optionally utilize [magic_enum](https://github.com/Neargye/magic_enum) for more powerful enum checking.
  - Ex. Ensuring an integer is within an enum's range
  - magic_enum requires C++17

Example:

```cpp
enum class MyEnum : int8_t { A = 1, B = 2, C = 3 };

// auto bad_cast1 = casts::enum_cast<MyEnum>(int16_t{128}); // Compile Error: cannot enum_cast to a smaller type
// auto bad_cast2 = casts::enum_cast<MyEnum>(uint8_t{128}); // Compile Error: cannot enum_cast between signed and unsigned

auto casted1 = casts::enum_cast<MyEnum>(int8_t{1}); // OK (MyEnum::A)
auto casted2 = casts::enum_cast<MyEnum>(int8_t{2}); // OK (MyEnum::B)

auto bad_cast3 = casts::enum_cast_checked<MyEnum>(int8_t{4}); // Error (if USE_MAGIC_ENUM): throws casts::enum_cast_error
```

### `float_cast`

- Converts floating-point types to integer types.
- Ensures that the value being cast is within the range of the target type.
- Provides several different methods of conversion:
  - `truncate`: Truncates the decimal portion of the float (same behavior as `static_cast` in most implementations).
  - `round`: Rounds the float to the nearest integer.
  - `ceiling`: Rounds the float up (away from zero) to the nearest integer.
  - `floor`: Rounds the float down (towards zero) to the nearest integer.
- By default, `float_cast` uses `truncate`, but you can specify a different method by providing a tag as the second argument.

Example:

```cpp
// auto bad_cast1 = casts::float_cast<double>(int8_t{128}); // Compile Error: cannot float_cast to a float

auto casted1 = casts::float_cast<int8_t>(float{27.5}); // OK (truncates to 27)
auto casted2 = casts::float_cast<int8_t>(float{27.5}, float_cast_op::round); // OK (rounds to 28)
auto casted3 = casts::float_cast<int8_t>(float{27.5}, float_cast_op::ceiling); // OK (rounds to 28)
auto casted4 = casts::float_cast<int8_t>(float{27.5}, float_cast_op::floor); // OK (rounds to 27)

auto bad_cast2 = casts::float_cast<int8_t>(float{128.5}); // Error: throws casts::float_cast_error
```

### `narrow_cast`

- Inspired by the version found in [Guideline Support Library](https://github.com/Microsoft/GSL).
- Casts from a larger to a smaller integer type.
- Types must have the same signedness.
- Ensures that the value being cast is within the range of the target type.
- Prevents narrowing conversions that could lead to data loss or undefined behavior.

Example:

```cpp
// auto bad_cast1 = casts::narrow_cast<int8_t>(uint16_t{128}); // Compile Error: cannot narrow_cast between signed and unsigned
// auto bad_cast2 = casts::narrow_cast<int16_t>(int8_t{128}); // Compile Error: cannot narrow_cast to a larger type

auto casted1 = casts::narrow_cast_checked<int8_t>(int16_t{127}); // OK
auto bad_cast3 = casts::narrow_cast_checked<int8_t>(int16_t{128}); // Error: throws casts::narrow_cast_error
```

### `sign_cast`

- Casts between signed and unsigned integer types.
- Ensures that the value being cast is within the range of the target type.
- Prevents narrowing conversions that could lead to data loss or undefined behavior.

Example:

```cpp
// auto bad_cast1 = casts::sign_cast<int8_t>(uint16_t{128}); // Compile Error: cannot sign_cast to a smaller type
// auto bad_cast2 = casts::sign_cast<int16_t>(int8_t{128}); // Compile Error: cannot sign_cast to the same signedness

auto casted1 = casts::sign_cast<int8_t>(uint8_t{127}); // OK
auto casted2 = casts::sign_cast<uint8_t>(int8_t{127}); // OK
auto casted3 = casts::sign_cast<int16_t>(uint8_t{129}); // OK (larger type)

auto bad_cast3 = casts::sign_cast<int8_t>(uint8_t{128}); // Error: throws casts::sign_cast_error
auto bad_cast4 = casts::sign_cast<uint8_t>(int8_t{-1}); // Error: throws casts::sign_cast_error
```

### `up_cast`

- Casts from a derived class to a base class.
- Ensures that the object being cast is of the correct type.
- Only allows casting pointers or references to avoid slicing.
- All checks are performed at compile time (`up_cast_checked()` is provided for consistency but provides no additional checks).

Example:

```cpp
class Base {};
class Derived : public Base {};
class Unrelated {};

Base b;
Derived d;
Unrelated u;

// auto bad_cast1 = casts::up_cast<Derived*>(&b); // Compile Error: cannot up_cast to a derived type
// auto bad_cast2 = casts::up_cast<Derived*>(&u); // Compile Error: cannot up_cast to an unrelated type

auto* casted1 = casts::up_cast<Base*>(&d); // OK
auto& casted2 = casts::up_cast<Base&>(d); // OK
```

### `void_cast`

- Casts to/from `void*`.
- All checks are performed at compile time (`void_cast_checked()` is provided for consistency but provides no additional checks).

Example:

```cpp
// auto bad_cast1 = casts::void_cast<int*>(float*); // Compile Error: cannot void_cast between non-void pointers
// auto bad_cast2 = casts::void_cast<void*>(size_t{}); // Compile Error: cannot void_cast to/from a non-void pointer

auto* casted1 = casts::void_cast<void*>(nullptr); // OK
auto* casted2 = casts::void_cast<int*>(casted1); // OK
```

## Future Improvements

- Allow customization of how errors are reported (replace exceptions with abort, assert, utilize `std::optional`/`std::expected`, etc.).
- Provide more compile-time checks where possible, utilizing newer C++ standards.
- Utilize C++26 reflection (as available) to provide more powerful type checking.

### Enhanced Version

The primary intention of this library is to make the operations supported by `static_cast` more specific and type-safe.
While more powerful casts (such as a `string_cast` or `variant_cast`) could be added, they are not included in this library.
In the future, there may be an optional extension to this library that provides these more powerful casts.
