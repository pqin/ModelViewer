#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include <GL/glew.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "ModelViewer.h"

int main(int argc, char *argv[]){
	std::freopen( "CON", "w", stdout );
	std::freopen( "CON", "w", stderr );
    if (argc < 2){
        std::cout << "Usage: " << argv[0] << " FILE.obj" << std::endl;
        return 0;
    }
    std::cout << "File: " << argv[1] << std::endl;
    // Parse OBJ file, load model
    ModelViewer modelviewer;
	bool status = modelviewer.load(argv[1]);
    if (!status){
        return 1;
    }

	int w = 640;
	int h = 480;
	std::string windowTitle = modelviewer.getFilename() + " (" + modelviewer.getDirectory() + ") - Model Viewer v2.0";
	// set up SDL and OpenGL
	if (modelviewer.setup(windowTitle, w, h) == false){
		std::cerr << "setup() failed." << std::endl;
		modelviewer.cleanup(1);
		return 1;
	}
    modelviewer.resize(w, h);
	modelviewer.runLoop();
	modelviewer.cleanup(0);
    return 0;
}
