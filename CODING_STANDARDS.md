# Coding Standards

## Formatting
This project uses ClangFormat for formatting.

## Code Style
By default, follow the [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines).

## Line feeds
Use LF in all text files. No CRLF.

## Naming

### Filenames
* Name files in `snake_case.cpp`
* Use the following extensions:
  * C++ source files: `.cpp`
  * C++ header files: `.hpp`
  * C source files: `.c`
  * C header files: `.h`

### Namespaces
* Name namespaces in `snake_case`.
* Match the top-level namespaces with the module name, if any.
* Namespaces in headers containing private declarations must be called `detail`.

### Types
* Name all types (classes, typedefs, enums, etc) in `PascalCase`.
* Do not prefix types with letters like  `C`, `I`, `T` or `E`.

### Methods
* Name both free and member methods in `snake_case`.
* Try to avoid getters and setters. If you have a container class and want a method to get its size, call it `size()`, not `get_size()`.
  * If you can modify the size as well, overloading will help you: `size()` and `size(std::size_t)`.

### Variables
* Name variables in `snake_case`. Constants, too.
* Prefer static variables with `s_`.
* Prefix (non-static) member variables with `m_`.
* Use descriptive names that are not too long.
* No "traditional" Hungarian notation (`pWindow`, `iSize`, etc).
* Loop indices can be `i`, `j`, etc, if their meaning is otherwise irrelevant.

### Macros
* Macros are all-caps `SNAKE_CASE`.
  * Note: this does NOT apply to constant variables.