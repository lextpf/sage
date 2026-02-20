# Contributing Guide

This guide defines contribution standards for the project's authors and co-authors, with or without AI assistance.

---

## Getting Started

1. **Fork the repository** on GitHub.
2. **Clone your fork** locally.
3. **Build the project** with `.\build.bat`


## Language & Build

|            Item | Standard                         |
|-----------------|----------------------------------|
|        Language | C++23 (`CMAKE_CXX_STANDARD 23`)  |
|        Compiler | MSVC 2022 (primary)              |
|    Build system | CMake 3.10+                      |
| Package manager | vcpkg                            |
|         Testing | Google Test with CTest discovery |

---

## Naming Conventions

|                Element |          Style          | Examples                                 |
|------------------------|-------------------------|------------------------------------------|
|                  Files |       PascalCase        | `Renderer.h`, `Settings.cpp`             |
|      Classes / Structs |       PascalCase        | `Game`, `TierDefinition`                 |
|                  Enums |       PascalCase        | `enum class Direction`                   |
|            Enum values |       PascalCase        | `Direction::Up`                          |
|    Functions / Methods |       PascalCase        | `Initialize()`, `LoadFromFile()`         |
|             Namespaces |       PascalCase        | `Settings`, `Renderer`                   |
|        Local variables |        camelCase        | `deltaTime`, `tileX`                     |
|             Parameters |        camelCase        | `float distance`                         |
|       Member variables |    `m_` + PascalCase    | `m_Window`, `m_Width`                    |
|       Static variables |       `s_` prefix       | `s_count`                                |
|       Global variables |       `g_` prefix       | `g_device`                               |
|     Constants / Macros |    UPPER_SNAKE_CASE     | `MAX_DISTANCE`, `TWO_PI`                 |
| Compile-time constants |   `static constexpr`    | `static constexpr int SIZE = 16;`        |
|           Type aliases | PascalCase with `using` | `using IndexMap = std::unordered_map<>;` |

### Struct members

Plain structs used as data holders use **camelCase** with no prefix:

```cpp
struct Particle
{
    glm::vec2 startPosition;
    glm::vec2 moveDirection;
    float maxLifetime;
};
```

### Prefer structs with named fields over pairs / tuples

Named fields are far easier to read than `.first` / `.second` or
`std::get<>()`. Use a small struct instead.

---

## Formatting

### Indentation

4 spaces. No tabs.

### Brace style

**Allman** &mdash; opening brace on its own line for all constructs
(namespaces, classes, functions, and control flow):

```cpp
namespace Occlusion
{
    bool IsBehindCamera(const RE::NiPoint3& worldPos)
    {
        if (distance < 0.001f)
        {
            return false;
        }

        return dot > 0.0f;
    }
}
```

### Always use braces

Even for single-statement bodies. This prevents bugs when lines are
added later:

```cpp
// Good
if (done)
{
    return;
}

// Bad
if (done)
    return;
```

### Constructor initializer lists

Leading comma, one member per line, aligned:

```cpp
Game::Game()
    : m_Window(nullptr)
    , m_ScreenWidth(1360)
    , m_CameraZoom(1.0f)
{
}
```

### Spacing

- Space after keywords: `if (`, `for (`, `while (`, `switch (`
- No space between function name and `(`: `Update(deltaTime)`
- Spaces around binary operators: `a + b`, `x = y`
- Space after commas: `Foo(a, b, c)`
- Space inside angle-bracket casts: `reinterpret_cast<T*>(ptr)`
- No space inside parentheses: `Foo(a, b)` not `Foo( a, b )`

### Boolean expressions

When a boolean expression spans multiple lines, break **before** the
operator:

```cpp
if (longConditionA
    && longConditionB
    && longConditionC)
{
    ...
}
```

### Return values

Do not wrap return values in parentheses:

```cpp
return result;      // Good
return (result);    // Bad
```

### Floating-point literals

Omit the leading zero for fractional literals:

```cpp
float x = .5f;     // Good
float x = 0.5f;    // Bad
```

### Pointer and reference declarations

Attach `*` and `&` to the **type**:

```cpp
int* ptr = nullptr;
const std::string& name = GetName();
```

---

## Header Files

Every header starts with `#pragma once`. Do not use `#ifndef` guards.

```cpp
#pragma once

// includes...
```

### Forward declarations

Avoid forward declarations when possible; prefer including the header.
Forward declarations hide dependencies and can silently change code
meaning. Use them only to break circular includes.

### Inline functions

Only define functions inline when they are short (roughly 10 lines or
fewer). Longer definitions belong in `.cpp` files.

---

## Include Order

Group includes in this order, separated by a blank line between groups.
Alphabetize within each group.

1. **Corresponding header** (`.cpp` files only)
2. **Project headers**
3. **External / third-party library headers**
4. **Standard library headers**

```cpp
#include "Occlusion.h"          // 1. corresponding header

#include "Settings.h"           // 2. project headers
#include "TextEffects.h"

#include <RE/A/Actor.h>         // 3. external libraries
#include <glm/glm.hpp>

#include <atomic>               // 4. standard library
#include <string>
#include <vector>
```

---

## Scoping

### Namespaces

- **Never** use `using namespace` in header files.
- `using namespace` is acceptable in `.cpp` files only at function scope
  or within an unnamed namespace, and only for well-known libraries
  (e.g. `std::literals`).

### Local variables

- Declare variables in the **narrowest scope** possible, as close to
  first use as practical.
- **Initialize at declaration**; never declare first and assign later.
- Prefer declaring loop variables inside the `for` / `while` statement.

### Internal linkage

Functions and variables used only within a single `.cpp` file should
have internal linkage. Use an unnamed namespace or `static`:

```cpp
namespace
{
    float ComputeWeight(float x)
    {
        return x * x;
    }
}  // namespace
```

### Static and global variables

- Objects with static storage duration must be **trivially destructible**
  unless they are function-local statics.
- Prefer `constexpr` or `constinit` for static/global initialization.
- Global strings should be `constexpr std::string_view` or `const char*`
  pointing to a literal, not `std::string`.

---

## Classes

### Constructors

- Avoid virtual method calls in constructors.
- If initialization can fail, use a factory function or a separate
  `Initialize()` method that returns `bool`.

### Implicit conversions

Mark single-argument constructors and conversion operators **`explicit`**
to prevent unintended type conversions:

```cpp
explicit Texture(const std::string& path);
```

Copy and move constructors are exempt from this rule.

### Copyable and movable types

Be explicit about copy/move semantics:

- **Copyable**: Default or define both copy constructor and assignment.
- **Move-only**: Define move operations; delete copy operations.
- **Neither**: Delete both.

```cpp
// Move-only resource
Texture(Texture&& other) noexcept;
Texture& operator=(Texture&& other) noexcept;
Texture(const Texture&) = delete;             // Deleted
Texture& operator=(const Texture&) = delete;  // Deleted
```

### Structs vs. classes

- Use `struct` for passive data containers with all-public fields and no
  invariants.
- Use `class` for types with methods, invariants, or private state.

### Operator overloading

- Overload only when semantics are obvious and unsurprising.
- **Never** overload `&&`, `||`, `,` (comma), or unary `&`.

---

## Functions

### Inputs and outputs

- Prefer **return values** over output parameters.
- Parameter order: inputs first, then outputs.
- Non-optional input: `const T&` (or `T` for cheap-to-copy types).
- Non-optional output / in-out: `T&`.
- Optional input: `const T*` (nullable) or `std::optional<T>`.
- Optional output: `T*` (nullable).

### Trailing return type

Use `auto Foo() -> ReturnType` only when the ordinary syntax is
impractical (e.g. complex template return types). Lambdas may always use
trailing return types.

---

## Ownership & Smart Pointers

- Prefer **single, fixed ownership**. Transfer ownership explicitly via
  smart pointers.
- `std::unique_ptr` for exclusive ownership (factory returns, member
  resources).
- `std::shared_ptr` only with strong justification (e.g. expensive
  immutable objects shared across subsystems). Beware of cyclic
  references.
- Raw pointers for **non-owning** references only.
- Never use `std::auto_ptr`.

---

## Documentation

### Where documentation lives

- **Header files** (`.h`): Doxide comments for every public class,
  method, enum, and non-trivial member.
- **Implementation files** (`.cpp`): Only `//` comments for non-obvious
  logic. No Doxide blocks.

### Comment styles

In **header files**, the following styles are supported before a
declaration:

```cpp
/** ... */
/*! ... */
/// ...
//! ...
```

For end-of-line documentation placed after a declaration:

```cpp
/**< ... */
/*!< ... */
///< ...
//!< ...
```

In **`.cpp` files**, use `//` exclusively.

### Class / namespace documentation

```cpp
/**
 * @class PlayerCharacter
 * @brief Handles player movement, animation, and collision.
 * @author Alex (https://github.com/lextpf)
 * @ingroup Player
 *
 * Detailed description of responsibilities.
 *
 * @see NonPlayerCharacter, Tilemap
 */
class PlayerCharacter { ... };
```

### Method documentation

```cpp
/**
 * Load a tilemap from a JSON file.
 *
 * @param path Filesystem path to the JSON map file.
 * @return `true` if the map loaded successfully.
 *
 * @pre The renderer must be initialized before calling this.
 */
bool LoadFromFile(const std::string& path);
```

### Doxide commands

Use Markdown wherever possible. Doxide provides a small set of commands
for organizing and annotating documentation:

| Command | Purpose |
|---------|---------|
|    `@param name` | Document a parameter and its direction |
|   `@tparam name` | Document a template parameter |
|        `@return` | Document the return value |
| `@pre` / `@post` | Document pre- or post-conditions |
|    `@throw name` | Document a thrown exception |
|           `@see` | Add "see also" references (format with Markdown links) |
|   `@anchor name` | Insert a link target for `[text](#name)` |
|  `@ingroup name` | Add the entity to a named group |
|             `@@` | Escape: produces a literal `@` |
|             `@/` | Escape: produces a literal `/` |

### Legacy commands supported by Doxide

Doxide replaces these with their Markdown equivalents during processing.
Both forms are accepted; use whichever reads best in context.

|                                                   Command(s) | Doxide behaviour       | Markdown equivalent        |
|--------------------------------------------------------------|------------------------|----------------------------|
|                             `@e word`, `@em word`, `@a word` | Replaced with Markdown | `*word*`                   |
|                                                    `@b word` | Replaced with Markdown | `**word**`                 |
|                                         `@c word`, `@p word` | Replaced with Markdown | `` `word` ``               |
|                                                `@f$ ... @f$` | Replaced with Markdown | `$ ... $` (inline math)    |
|                                                `@f[ ... @f]` | Replaced with Markdown | `$$ ... $$` (display math) |
|                                                `@li`, `@arg` | Replaced with Markdown | `- ` (list item)           |
|           `@code ... @endcode`, `@verbatim ... @endverbatim` | Replaced with Markdown | ```` ``` ... ``` ````      |
| `@bug`, `@example`, `@note`, `@todo`, `@warning`, `@remarks` | Replaced with Markdown | `!!! type` (admonition)    |
|                                             `@ref name text` | Replaced with Markdown | `[text](#name)`            |
|                                     `@image format file alt` | Ignored                | `![alt](file)`             |
|                                        `@returns`, `@result` | Treated as `@return`   | Use `@return`              |
|                                      `@throws`, `@exception` | Treated as `@throw`    | Use `@throw`               |
|                                                        `@sa` | Treated as `@see`      | Use `@see`                 |

We still use `@brief`, `@class`, `@struct`, `@union`, `@enum`, and `@namespace` for readability and
forward-compatibility.

Commands we do **not** use: `@short`, `@file`, `@defgroup`, `@def`,
`@fn`, `@var` and `@internal`.

### Enum and member documentation

Use trailing doc comments for brief inline docs:

```cpp
enum class ParticleStyle
{
    Stars,    ///< Twinkling star points
    Sparks,   ///< Fast, erratic fire-like sparks
    Wisps,    ///< Slow, flowing ethereal wisps
};
```

### Grouped members

Use `/// @ingroup` or `/// @name` / `/// @{` ... `/// @}` to group
related members:

```cpp
/// @name Lifecycle
/// @{
bool Initialize();
void Run();
void Shutdown();
/// @}
```

### Rich documentation

Use Markdown freely in documentation comments: tables, links, images,
admonitions, and math.

**Material Design Icons** can be used in headings and text.

**Mermaid diagrams** are supported for architecture and flow
visualizations:

````cpp
/**
 * Manages the game's day/night cycle.
 *
 * ```mermaid
 * stateDiagram-v2
 *     Dawn --> Midday
 *     Midday --> Dusk
 *     Dusk --> Night
 *     Night --> Dawn
 * ```
 */
````

### Inline comments

- Explain *why*, not *what*.
- Keep them short and on the same line when possible.
- Use `//` style exclusively in `.cpp` files.

### TODO comments

Mark incomplete or temporary work with a `TODO` and attribution:

```cpp
// TODO: Replace with spatial hash once tile count exceeds 10k.
```

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE).