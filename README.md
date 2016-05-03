# A Light-weight, Offscreen and Fast 3D Model Renderer

### Features
This code is designed for building previews of 3D models. So efficiency and 3D format compatibility outweigh quality.

1. A light-weight offscreen renderer in C++. 
2. Easy to compile and fast to run. 
3. Run in command line. 
4. Support over 40+ common 3D formats such as OBJ, OFF and COLLADA. 
5. Support materials and textures. 

### Installation

#### Linux (tested on Ubunut 14.04)

First, install dependencies by apt-get:
		
	sudo apt-get install libosmesa6 libglew1.10 libdevil1c2 libassimp-dev
    
Then, build the code by running

	make
  
This will create an executable named `render`

#### MacOS
I haven't tried to compile it on MacOS yet, though it should not be hard. You need to take care of dependencies: OSMesa, GLEW, DevIL and Assimp. They all support MacOS.

Then, build the code by running
	
	make
  
This will create an executable named `render`

### Usage:
run `render` and help message appears.

### Note:
Normal smoothing is not enabled. 
