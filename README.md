GOTCHA v0.0.1 (alpha)
============

Gotcha is a library that wraps functions.  Tools can use gotcha to install hooks into other libraries, for example putting a wrapper function around libc's malloc.  
It is similar to LD_PRELOAD, but operates via a programable API.
This enables easy methods of accomplishing tasks like code instrumentation or wholesale replacement of mechanisms in programs
without disrupting their source code.

This release of Gotcha is alpha software.  It should not be used in production, and there are numerous known issues that
still need to be fixed:

  * "Stacking," or having several tools each use Gotcha to wrap the same function. We eventually want to allow this,
    and to have an API in which the order in which wrappers execute is configurable, but we aren't there
    yet.
  * Gotcha can be made to sabotage itself if it destructively replaces any functions that it relies on.
  * Gotcha is not yet performant on applications with many shared libraries.
  * Gotcha does not yet support dlopen, either wrapping symbols looked up by dlsym or modifying dlopen'd libraries.
  * Full user docs. Right now you have this README, my email (poliakoff1@llnl.gov) and Doxygen as options for understanding the software. This is meant to be productized software, in the future we hope to give you better documentation resources.
  * Automated wrapper generation. We have a pre-alpha Clang compiler plugin for generating libraries which use Gotcha
    to wrap all the functions in a header file. We hope in the future to automate some of the scut work in Gotcha use

Quick Start
-----------

*Building Gotcha* is trivial. In the root directory of the repo

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX = <where you want the sofware> ..
make install
```
*Usage* is fairly simple. For us to wrap a function, we need to know its name, what you want it wrapped with (the wrapper), and we need to give you some ability to call the function you wrapped (wrappee). Gotcha works on triplets containing this information. We have [small sample uses](src/example/autotee/autotee.c), but the standard workflow looks like


```
  #include <gotcha/gotcha.h>
  static int (*wrappee_puts)(const char*); //this lets you call the original
  static int puts_wrapper(const char* str); //this is the declaration of your wrapper
  static int (*wrappee_fputs)(const char*, FILE*);
  static int fputs_wrapper(const char* str, FILE* f);
  struct gotcha_binding_t wrap_actions [] = {
    { "puts", puts_wrapper, &wrappee_puts },
    { "fputs", fputs_wrapper, &wrappee_fputs },
  } 
  int init_mytool(){
    gotcha_wrap(wrap_actions, sizeof(wrap_actions)/sizeof(struct gotcha_binding_t), "my_tool_name");
  }
  static int fputs_wrapper(const char* str, FILE* f){
    // insert clever tool logic here
    return wrappee_fputs(str, f); //wrappee_fputs was directed to the original fputs by gotcha_wrap
  }

```

*Building your tool* changes little, you just need to add the prefix you installed Gotcha to your include directories, the location
the library was installed (default is <that_prefix>/lib) to your library search directories (-L...), and link
libgotcha.so (-lgotcha) with your tool. Very often this becomes "add -lgotcha to your link line," and nicer CMake integration is coming down the pipe.

That should represent all the work your application needs to do to use Gotcha

Contact/Legal
-----------

The license is [LGPL](LGPL).

Primary contact/Lead developer

David Poliakoff (poliakoff1@llnl.gov)

Other developers

Matt Legendre  (legendre1@llnl.gov)
