
This is an open source (GPL3) release of SourceGear DiffMerge.

Example command lines to build:

```
cd sgdm3

# Mac
ARCH=arm64 make -f Makefile.Apple
ARCH=x86_64 make -f Makefile.Apple

# Windows
TARGET_PLATFORM=x64 make -f Makefile.Windows
TARGET_PLATFORM=x86 make -f Makefile.Windows

# Linux
make -f Makefile.Linux
```


