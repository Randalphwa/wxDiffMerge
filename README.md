
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

## Builds and Releases

From time to time, we may publish builds here as a GitHub release.

Currently, the platforms included are:

- macOS arm64 and x86_64 (dmg)
- Windows x64 and x86 (msi and zip)
- Ubuntu amd64 (deb)

Please note that macOS builds are neither signed nor notarized.

