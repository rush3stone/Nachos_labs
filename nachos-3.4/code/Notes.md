### [The difference between source file(.c) and header file(.h)](https://stackoverflow.com/questions/3482948/any-fundamental-difference-between-source-and-header-files-in-c)

There is no *technical* difference. The compiler will happily let you include a `.c` file, or compile a `.h` file directly, if you want to.

There is, however, a huge *cultural* difference:

* *Declarations* (prototypes) go in `.h` files. The `.h` file is the *interface* to whatever is implemented in the corresponding `.c` file.

* *Definitions* go in `.c` files. They *implement* the interface specified in the `.h` file.

The difference is that a `.h` file can (and usually will) be `#include`d into multiple compilation units (`.c` files). If you *define* a function in a `.h` file, it will end up in multiple `.o` files, and the linker will complain about a multiply defined symbol. That's why definitions should not go in `.h` files. (Inline functions are the exception.)

If a function is defined in a `.c` file, and you want to use it from other `.c` files, a declaration of that function needs to be available in each of those other `.c` files. That's why you put the declaration in a `.h`, and `#include` that in each of them. You could also repeat the declaration in each `.c` file, but that leads to lots of code duplication and an unmantainable mess.

If a function is defined in a `.c` file, but you *don't* want to use it from other `.c` files, there's no need to declare it in the header. It's essentially an implementation detail of that `.c` file. In that case, make the function `static` as well, so it doesn't conflict with identically-named functions in other files.