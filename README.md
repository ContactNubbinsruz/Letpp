# let.hpp

Single-header dynamically-typed value for C++17.

```cpp
#include "let.hpp"

int main()
{
    let x = 5;
    x = "now a string";
    x = true;

    let arr = {1, 2, 3, "hello", true};
    log(arr[2]);

    let person = {
        {"name", "Amelia"},
        {"age", 15}
    };
    log(person["name"]);

    for (let e : person)
        log(e);
}
```

## Features

- One type (`let`) holds `int`, `double`, `bool`, `std::string`, arrays, or objects
- Arrays: `let arr = {1, 2, 3};`
- Objects: `let obj = { {"key", "value"} };` — auto-detected when every
  element is a 2-item array with a string first item. `obj{...}` also works.
- Indexing by literal, variable, or another `let`: `arr[i]`, `person["name"]`
- Range-based `for` — arrays yield elements, objects yield `{key, value}`
  pairs (`e.key()`, `e.val()`)
- `+ - * / %`, compound assignment, `++`/`--`, with JS-style type coercion
- `log(x)` prints value and type
- `.str() .size() .empty() .clear() .push() .contains() .find()`

## Requirements

C++17 or newer. Tested with GCC 13 and Clang 18.

## Notes

- `let x = {5};` becomes array `[5]`, not int `5` — use `let x = 5;` for scalars
- Mutable object iteration (`for (let& e : obj)`) shares one `thread_local`
  scratch value per thread; fine for read-only use, don't hold multiple
  `e` references expecting independent copies
- Type mismatches and bad indices throw `std::runtime_error` /
  `std::out_of_range`

## License

Do whatever you want with it.
