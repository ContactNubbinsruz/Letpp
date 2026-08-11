# let.hpp

A single-header C++17 "dynamically typed" value for JS/Python devs stuck
writing C++. Drop it in, `#include "let.hpp"`, and `let` behaves like a
loose, dynamically-typed variable — with arrays, objects, and all the
cursed implicit conversions that come with it.

```cpp
#include "let.hpp"

int main()
{
    let x = 5;
    x = "now I'm a string";
    x = true;

    let arr = {1, 2, 3, "hello", true};
    log(arr[2]);

    let person = {
        {"name", "Liam"},
        {"age", 15}
    };
    log(person["name"]);

    for (let e : person)
        log(e);
}
```

## Features

- **One type, any value** — `int`, `double`, `bool`, `std::string`, arrays,
  and objects all live inside a single `let`.
- **Arrays without a prefix** — `let arr = {1, 2, 3};` just works.
- **Objects, auto-detected** — `let obj = { {"key", "value"} };` is
  recognized as an object because every element is a 2-item array whose
  first item is a string. Use `obj{...}` if you want to be explicit.
- **Indexing by `let`** — `arr[i]`, `person["name"]`, `person[key]` all
  work, whether the index is a literal, an `int`, or another `let`.
- **Range-based `for`** — iterating an array yields elements; iterating
  an object yields `{key, value}` pairs (`e.key()` / `e.val()`).
- **Arithmetic overloads** — `+ - * / %` and their compound forms
  (`+= -= *= /= %=`), plus `++`/`--`, all coerce types the way you'd
  expect from a scripting language (`"abc" * 3`, `10 + " bananas"`, etc.)
- **`log(x)`** — prints the value and its inferred type.
- **`.str()`, `.size()`, `.empty()`, `.clear()`, `.push()`, `.contains()`,
  `.find()`** — basic container ergonomics.

## Why

Mostly to troll other devs into thinking C++ suddenly grew dynamic
typing. Use responsibly (or don't).

## Requirements

C++17 or newer. Tested with GCC 13 and Clang 18.

## Caveats

- `let x = {5};` (a single-element brace-init) becomes the **array**
  `[5]`, not the int `5` — brace-init always prefers the
  `initializer_list` constructor. Use `let x = 5;` for scalars.
- Mutable iteration over an **object** (`for (let& e : obj)`) reuses a
  single `thread_local` scratch `let` per thread to materialize each
  `{key, value}` pair. It's fine for read-only use (`log(e)`,
  `e.key()`, `e.val()`), but don't hold onto more than one `e` at a
  time expecting independent copies.
- `operator[]` throws `std::runtime_error` / `std::out_of_range` on
  type mismatches or bad indices — there's no silent `undefined`.

## License

Do whatever you want with it.
