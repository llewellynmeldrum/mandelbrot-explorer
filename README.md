# Mandelbrot Explorer V0.1
Mandelbrot set plotter and 'explorer' using [Metal](https://en.wikipedia.org/wiki/Metal_(API)) shaders. Zero objective-C/obj-C++, via [metal-cpp](https://github.com/bkaradzic/metal-cpp) api.

<img src="https://github.com/user-attachments/assets/c1324b78-8f59-482a-aa18-b9e460f601e6" width="400">

<br>
<img src="https://github.com/user-attachments/assets/7c4c5bad-d0a1-4de3-b3a0-cc5984e8d762" width="200">
<img src="https://github.com/user-attachments/assets/fbee3809-db32-4624-bc1e-29af7067b691" width="200">

_ Badly precision limited (no infinite zoom), since Metal only supports FP32. Intend to change to some sort of arbitrary precision floating point reccomendation._


# Dependencies
M1 macos or higher.

### [Gnu Make](https://www.gnu.org/software/make/)
- building
```
brew install make
```
### [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2)
- For window creation and handling
```bash
brew install sdl2
```
### [Xcode](https://github.com/llewellynmeldrum/mandelbrot-explorer/edit/mandelbrot-explorer/README.md) 
- For metal api, darwin frameworks
```
app store -> install xcode
```
> [!NOTE]  
> Im almost certain the current build has memory leaks, metal-cpp has to use a hybrid of RAII and objective-c style ref counts which is really confusing as someone who barely knows how memory management works in c++. 



# why not just use xcode editor and swift or something
Interacting with metal through the more standard swift/obj-c/xcode editor toolchain requires interacting with swift/obj-c/xcode so I did everything to avoid that. Thankfully, metal-cpp now has 100% coverage (as I understand it), so zero objective c is required. Theres basically zero up to date resources I could find that used zero objective-C, so the core of the pipeline is adapted from the following:

# References:
[https://metaltutorial.com/Setup/](https://metaltutorial.com/Setup/)

[apple metal docs](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)


### ps. commits:
The commit history on this is a bit mangled, this started as a branch on a [separate project](https://github.com/llewellynmeldrum/game-of-life/tree/mandelbrot-explorer). I dont really know how to use git so I just cloned the commit history onto this repo
