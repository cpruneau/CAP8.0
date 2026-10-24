# CapShell: the CAP command line interpreter

`bin/CapShell` runs commands of a small language built into CAP. It has no dependency other than CAP's own libraries
(and the system's libedit, when present, for line editing). The language reaches C++ classes through CAP's reflection
registry (see [Reflection.md](Reflection.md)), so a class registered once can be used from the prompt.

```
CapShell                  interactive session; history in ~/.cap_history
CapShell file.caps ...    run files of commands, then stop
CapShell -i file.caps     run a file, then continue interactively
CapShell -e "commands"    run commands, then stop
```

## A session

```
cap [0] g = MersenneTwister(12345)
cap [1] h = Histogram("h", "gauss", 40, -4, 4)
cap [2] for k in 1..10000 {
    ...   h.fill(g.gauss(0, 1))
    ... }
cap [3] h.mean()
(real) 0.009300059743
cap [4] save("gauss.pdf", h)
cap [5] write("gauss.cap", h)
cap [6] help Histogram
```

A command runs once its parentheses, brackets and braces are closed. Until then, the prompt `...` asks for more
lines.

## The language

- **Values:**
  - numbers: integers `3` and reals `3.5` or `1e-3`;
  - texts: `"text"` or `'text'`;
  - `true`, `false`, `none`;
  - lists: `[1, "a", h]`;
  - objects of the registered classes.
- **Variables:** made by assignment, `x = 2`. The forms `x += 1`, `-=`, `*=` and `/=` also work. `pi` and `e` are
  defined.
- **Operators:**
  - arithmetic: `+ - * / % ^` (or `**`);
  - comparison: `== != < <= > >=`;
  - logic: `and or not` (or `&& || !`);
  - range: `a..b` is the integers from a to b, both included;
  - indexing: `l[0]`, and `l[-1]` for the last element.
- **Integers stay integers:** `+ - * %` and `^` (with a positive power) keep integers integer. `/` always gives a
  real. `+` also joins texts and lists.
- **Control flow:**
  ```
  if c { ... } else if d { ... } else { ... }
  while c { ... }
  for x in 1..1000000 { ... }    # a range is counted, not made into a list
  for x in [1, 2, 3] { ... }
  break  continue
  ```
- **Functions:**
  ```
  fun f(a, b) { return a + b }
  ```
  A function sees its parameters and the variables it creates. It also reads and assigns the global variables
  that already exist.
- **Objects:**
  - create one by calling its class: `Histogram("h", "title", 40, -4, 4)`;
  - call its member functions: `h.fill(1.5)`;
  - read and set its members: `h.name`, `h.name = "g"`.
  The functions of the base classes are found too.
- **Lists and texts:** `x.size()`. For a list held by a variable, `l.append(v)` and `l[k] = v`.
- **Comments:** from `#` or `//` to the end of the line. Statements are separated by new lines or `;`.

## Built-in functions

| | |
|---|---|
| Output, session | `print(a, ...)`, `help`, `help name`, `help(x)`, `type(x)`, `classes(base)`, `variables()`, `run(file)`, `clock()`, `exit()` |
| Conversions, lists | `int(x)`, `real(x)`, `text(x)`, `len(x)`, `range(n)`, `range(first, end, step)`, `sum`, `min`, `max` |
| Mathematics | `sqrt exp log log10 sin cos tan asin acos atan atan2 floor ceil round abs pow` |
| CAP files | `ls(file)`, `read(file, path)`, `write(file, h, ...)` (histograms and profiles, under their names) |
| Figures | `save(file, h, ...)`: one graph, 800 × 600 points, SVG or PDF by the extension |

## Errors

An error gives its line and stops the command; the session goes on. For example:

```
cap [7] h.fill("x")
error (line 1): No function fill accepts (text): Not a number: "x"; candidates:
  fill void (double)
  fill void (double, double)
  ...
```

## Making a class available

Describe it once in a `.cpp` file of its library (see `core/Math/MathReflection.cpp`):

```cpp
CAP_REFLECTION(MyClasses)
{
  CAP::REFLECTION::ClassBuilder<MyClass>("CAP::MyClass")
    .base<CAP::Object>()
    .constructor<const CAP::String &, double>()
    .method("compute", &MyClass::compute)
    .property("width", &MyClass::width, &MyClass::setWidth);
}
```

For C++ default arguments, add a short form, e.g. `.function("mean", +[](const Histogram & h) { return h.mean(); })`.
The classes available today are `CAP::Object`, `Partition`, `DataPoints`, `Histogram`, `RandomGenerator` and
`MersenneTwister`.
