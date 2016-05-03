# LightweightRenderer
A light-weight offscreen renderer in C++. Run in command line. Easy to compile and fast to run. Support over 40+ common 3D formats such as OBJ, OFF and COLLADA. Support materials and textures. 

### Installation on Linux (tested on Ubunut 14.04)

First, install dependencies by apt-get:
		
	sudo apt-get install libosmesa6 libglew1.10 libdevil1c2 libassimp-dev
    
Then, build the code by running

  make
  
This will create an executable named `render`

#### Installation on MacOS
I haven't tried to compile it on MacOS yet, though it should not be hard. You need to take care of dependencies: OSMesa, GLEW, DevIL and Assimp. They all support MacOS.

Then, build the code by running
	
	make
  
This will create an executable named `render`

#### Usage:
run `render` and help message appears.
