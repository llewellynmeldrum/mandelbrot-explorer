# Mandelbrot Explorer V0.1
Mandelbrot set plotter and explorer using [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2) to grab a metal surface, and [metal-cpp](https://github.com/bkaradzic/metal-cpp) to plot the set with a fragment shader.

<img src="https://github.com/user-attachments/assets/c1324b78-8f59-482a-aa18-b9e460f601e6" width="400">

<br>
<img src="https://github.com/user-attachments/assets/7c4c5bad-d0a1-4de3-b3a0-cc5984e8d762" width="200">
<img src="https://github.com/user-attachments/assets/fbee3809-db32-4624-bc1e-29af7067b691" width="200">

_Quite obviously precision limited (no infinite zoom), since Metal only supports FP32, i'll have to use another arbitrary or fixed precision datatype._


# Dependencies
M1 macos or higher (i think)
### gnu make
```
brew install make
```
### sdl2
```bash
brew install sdl2
```
### xcode (for frameworks and stuff)
```
app store -> install xcode
```
> [!NOTE]  
> Current version probably has memory leaks, its quite hard to keep track of resources with the metal-cpp bindings. I would use the `NS::SharedPointer`  implementation but I dont know what a shared pointer is.



# why not just use xcode
Interacting with metal through the more standard swift/obj-c/xcode editor toolchain requires interacting with swift/obj-c/xcode so I did everything to avoid that. Thankfully, metal-cpp now has 100% coverage (as I understand it), so zero objective c required.

# References:
[https://metaltutorial.com/Setup/](https://metaltutorial.com/Setup/)
[apple metal docs](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)

### ps. commits:
The commit history on this is a bit mangled, this started as a [separate project](https://github.com/llewellynmeldrum/game-of-life/tree/mandelbrot-explorer), which I swapped to this separate repo. Cloned the commit history for convinience.
