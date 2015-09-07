# sushi

Xcode (and eventually) Visual Studio and Ninja project generator

Work in progress...

## Status

* Generates conformant Xcode project files
* Established initial file format for Makifile (Sushi project file)
* Visual Studio solution and project file parsing and output complete
* ```sushi.xcodeproj/project.pbxproj``` is generated by ```maki```

## Building

```
git submodule update --init
make -j4
```

## Example

To create the Xcode project for Sushi:
```
./build/darwin_x86_64/bin/maki sushi.sushi > sushi.xcodeproj/project.pbxproj
```
