# Windows Installation Notes

Tested to work with clang from LLVM 8.0, on a 64-bit Windows Server 10 OS, with Microsoft Visual Studio installed with C++ desktop support.

Steps:

```
> cmake . -Bbuild-windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
> cmake --build build-windows --target goldfish.x
```

This will spew out _many_ warnings, all of which seems to be irrelevant.


Also note that you might need to disable hardward support for `popcount` in `external/Fathom/src/tbconfig.h` by uncommenting the `TB_NO_HW_POPCNT` variable.
