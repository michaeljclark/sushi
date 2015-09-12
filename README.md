# sushi

Xcode, Visual Studio (and eventually) Ninja project generator

Work in progress...

## Status

* Generates Xcode and Visual Studio project files
* Established initial file format for maki file (.sushi project file)
* ```sushi.xcodeproj``` and ```sushi.vsproj``` are generated by ```maki```

## To do

* Implement cflags
* Implement define to allow compiler and sdk to be overridden
* Implement globbing to make project definition more concise
* Implement cross-project dependencies

## Building

```
git submodule update --init
make -j4
```

## Example

To create the Xcode project for Sushi:
```
./build/darwin_x86_64/bin/maki sushi.sushi xcode 
```

To create the Visual Studio solution for sushi:
```
./build/darwin_x86_64/bin/maki sushi.sushi vs2015
```
