# starfield
Reminiscence to the screensaver that shipped until WinXP

Requires libGL and SDL.

Building with
    gcc -o starfield starfield.c -O2 -lglut -lGL -lGLU -lSDL -lSDL_image

starfield expects the star_[0-3].jpg images in PWD (ie in doubt "next to it")

Supported parameters (and defaults):
* stars=512
* spaceDepth=1500
* speed=2.0
* width=768
* height=480
* forward
* backward (the default)
* fullscreen
* fps=0 (try 30, zero is uncapped)

Keyboard actions:
* F1        : toggle fullscreen
* d         : toggle direction
* Up        : double up speed
* Down      : half speed
* left/right: rotate
* Space     : break / choke
* Escape    : quit


---
There's really no xscreensaver hack like this??
