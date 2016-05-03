# A Light-weight, Offscreen and Fast 3D Model Renderer

This code is designed for building previews of 3D models. So we aim at rendering many formats of 3D models robustly with acceptable quality. 

### Main Features
1. A light-weight offscreen renderer in C++. 
2. Easy to compile and fast to run. 
3. Run in command line. 
4. Support over 40+ common 3D formats such as OBJ, OFF and COLLADA. 
5. Support materials and textures. 

### Installation

#### Linux (tested on Ubunut 14.04)

First, install dependencies by apt-get:
		
	sudo apt-get install libglew-dev libdevil-dev libassimp-dev freeglut3-dev libpng3

Second, install Mesa3D (>=11.0.7):
	
	wget ftp://ftp.freedesktop.org/pub/mesa/11.0.7/mesa-11.0.7.tar.gz
	tar xvf mesa-11.0.7.tar.gz
	cd mesa-11.0.7
	./configure --enable-osmesa --disable-driglx-direct --disable-dri --with-gallium-drivers=swrast --disable-egl
	make -j20
	sudo make install
	
By default, this will install OSMesa to `/usr/local`.

Note that you may have other versions of Mesa3D installed before. Then you need to uninstall/remove/disable old versions, or set `LD_LIBRARY_PATH`. Old versions of OSMesa might be found in `/usr/lib/x86_64-linux-gnu`. 

Third, build the renderer by running

	git clone git@github.com:ShapeNet/LightweightRenderer.git
	cd LightweightRenderer
	make
  
This will create an executable named `render`

Lastly, test your build by running
	
	./render airplane.obj airplane.png
	
This should generate an image named `airplane.png` as below:
![example](https://github.com/ShapeNet/LightweightRenderer/blob/master/airplane.png)

### Usage
run `render` and help message appears.

### Note
Normal smoothing is not enabled. This is to avoid bad rendering when surface normals are incorrect. 
