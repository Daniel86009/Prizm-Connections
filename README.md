# Casio fx-CG project template

This is a starter template for C/C++ projects targeting the Casio fx-CG series graphing calculators, using the [Prizm SDK](https://github.com/Jonimoose/libfxcg).

## Using this template

Source code is placed in `/src`. Resources used to build the `g3a` add-in package are placed in `resource`, at the moment this is only the home screen icons in unselected and selected versions.

### Prerequisites

You'll need to [build and install the Prizm SDK](https://prizm.cemetech.net/Tutorials/PrizmSDK_Setup_Guide/) for your system, as well as appropriate versions of the GCC toolchain targeting the SH4 CPU. Set the shell environment variable `FXCGSDK` to the directory where the SDK is installed (parent of the `lib`, `include` and `toolchain` directories).

### Building

To build your project, simply run `make` in the project root directory. Object files (`.o`) are placed in `/build`, and final artifacts are placed in the root directory.

The build produces an executable file (`.bin`) and a Casio add-in package (`.g3a`). Connect your calculator via USB and copy the `g3a` file to the root directory.

## Additional files

`compile_flags.txt` lets clangd know where to find the `libfxcg` headers for neovim users. Replace the path with your SDK install path.
