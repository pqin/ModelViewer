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

ModelViewer::ModelViewer(){
	textureID = NULL;
    texNum = 0;
	window = NULL;

    center = Vector_f::Zero;
    rotation0 = Vector_f::Zero;
    rotation1 = Vector_f::Zero;
    rotationAngle = 0.f;
    rotationAxis = Vector_f::Zero;

    lightPosition[0] = 0.1f;
	lightPosition[1] = 1.f;
	lightPosition[2] = 0.f;
	lightPosition[3] = 1.f;

    showSolid = true;
    showTexture = true;
    run = false;
}
bool ModelViewer::load(char *filename){
	bool status = model.load(filename);
	float scale = 1.f;
	if (status){
		std::cout << " - No errors." << std::endl;
		scale = model.getScale();
		center = model.getCenter() * (-1.f/scale);
	} else {
		std::cerr << " - Errors parsing file." << std::endl;
		std::cerr << " - Abort." << std::endl;
	}
	return status;
}
std::string ModelViewer::getFilename(){
	return model.getFilename();
}
std::string ModelViewer::getDirectory(){
	return model.getDirectory();
}

// rotate in model-space
void ModelViewer::orientLocal(){
    static Vector_f deltaRotation;

    deltaRotation = rotation1 - rotation0;

    // x-axis rotation
    if (deltaRotation.x != 0.f){
        rotation.rotate(deltaRotation.x, 1, 0, 0);
        orientation = rotation * orientation;
    }
    // y-axis rotation
    if (deltaRotation.y != 0.f){
        rotation.rotate(deltaRotation.y, 0, 1, 0);
        orientation = rotation * orientation;
    }
    // z-axis rotation
    if (deltaRotation.z != 0.f){
        rotation.rotate(deltaRotation.z, 0, 0, 1);
        orientation = rotation * orientation;
    }
    orientation.normalize();
	rotationAngle = orientation.getAngle() * (180.f/PI);
	rotationAxis = orientation.getAxis();
}
// Drawing (display) routine.
void ModelViewer::drawScene(void){
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);   // Clear screen to background color.

    glMatrixMode(GL_MODELVIEW);     // Set matrix mode to modelview.
    glLoadIdentity();               // Clear current modelview matrix to identity.

    camera.setView();

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glTranslatef(0.f, 0.f, -2.f);

    // rotate with quaternion
	glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

    glPushMatrix();
    // set solid or wireframe mode
    if (showSolid){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
    }
	glColor3f(1.f, 1.f, 1.f);
    glTranslatef(center.x, center.y, center.z);

    model.draw(showTexture);
    glPopMatrix();

    SDL_GL_SwapWindow(window);	// Render to screen.
}

// Initialization routine.
bool ModelViewer::setup(std::string winTitle, const int winWidth, const int winHeight){
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE,     5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE,   5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE,    5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE,  16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// create SDL window
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        std::cerr << "Video initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
	window = SDL_CreateWindow(
		winTitle.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		winWidth,
		winHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	SDL_GL_CreateContext(window);
	// initialize GLEW
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK ){
		std::cerr << "Error initializing GLEW: " << glewGetErrorString( glewError ) << std::endl;
		return false;
	}
	// get version of OpenGL supported
	std::cout << "Using OpenGL v" << glewGetString(GLEW_VERSION) << std::endl;
	
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);   // Set background (or clearing) color.
    glEnable(GL_DEPTH_TEST);
	if (model.isSmooth()){
		glShadeModel(GL_SMOOTH);
	} else {
		glShadeModel(GL_FLAT);
	}
	glFrontFace(GL_CCW);

    // lighting for solid
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // get texture
	SDL_Surface *image = NULL;
	std::cout << "Retrieving texture files." << std::endl;
	std::vector<std::string> textureFile = model.getTextureFiles();
	texNum = (GLsizei)textureFile.size();
	std::cout << "Number of texture files: " << texNum << std::endl;
	if (texNum > 0){
		textureID = new (std::nothrow) GLuint[texNum];
		if (textureID == NULL){
			std::cerr << "Failed to allocate textureID[] of size " << texNum << std::endl;
			return false;
		}
		glGenTextures(texNum, textureID);
	}
	bool status = true;
	for (size_t i = 0; i < textureFile.size(); ++i){
		std::cout << " - file:" << textureFile[i] << std::endl;
		image = IMG_Load(textureFile[i].c_str());
		if (image != NULL){
			std::cout << "Generating texture." << std::endl;
			genTexture(image, textureID[i]);
			model.setTexture(textureFile[i], textureID[i]);
			// free surface
			SDL_FreeSurface(image);
			image = NULL;
		} else {
			std::cerr << "Failed to open/load texture file: " << textureFile[i] << std::endl;
			status = false;
			break;
		}
	}
	if (status){
		// Specify how texture values combine with current surface color values.
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

    rotation0.set(0, 0, 0);
    rotation1.set(0, 0, 0);
    orientation.set(1, 0, 0, 0);
    rotation.set(1, 0, 0, 0);

	return status;
}
void ModelViewer::genTexture(SDL_Surface *image, GLuint id){
	glBindTexture(GL_TEXTURE_2D, id);
    // Set texture parameters for wrapping.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture parameters for filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Specify an image as the texture to be bound with the currently active texture index.
	// pixel colour format
	static int mode;
    mode = GL_RGB;
	if(image->format->BytesPerPixel == 4) {
		mode = GL_RGBA;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, image->format->BytesPerPixel,
				 image->w, image->h,
				 0, mode, GL_UNSIGNED_BYTE, image->pixels);
}
void ModelViewer::runLoop(){
	run = true;
	while( run ) {
        process_events();
        update();
		drawScene();
    }
}

void ModelViewer::update(){
	if (!showSolid){
		showTexture = false;
	}
	orientLocal();
    rotation0 = rotation1;
}

// OpenGL window reshape routine.
void ModelViewer::resize(int w, int h){
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);   // Set viewport size to be entire OpenGL window.
    glMatrixMode(GL_PROJECTION);                // Set matrix mode to projection.
    glLoadIdentity();                           // Clear current projection matrix to identity.

    // Specify projection
    camera.resize(w, h);
    /*
    glFrustum(-0.1f, 0.1f,
			  -0.1f*ratio, 0.1f*ratio,
			   0.5f, 10.f);
    */

    glMatrixMode(GL_MODELVIEW);                 // Set matrix mode to modelview.
    glLoadIdentity();                           // Clear current modelview matrix to identity.
}
void ModelViewer::process_events(void){
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    SDL_Event event;
    while( SDL_PollEvent( &event ) ) {
        switch( event.type ) {
        case SDL_KEYDOWN:
			keyInput( event.key.keysym.sym );
            break;
        case SDL_KEYUP:
            break;
        case SDL_MOUSEMOTION:
            // dragging motion with left mouse button
            if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)){
                if (keys[SDL_SCANCODE_LSHIFT] != keys[SDL_SCANCODE_RSHIFT]){
					camera.translate(-1*event.motion.xrel, event.motion.yrel);
                } else {
                    rotation1.x += camera.rotate(event.motion.yrel, Vector_f::X_Axis);
                    rotation1.y += camera.rotate(event.motion.xrel, Vector_f::Y_Axis);
                }
            }
            break;
		case SDL_MOUSEWHEEL:
			camera.zoom(event.wheel.y);
			break;
        case SDL_MOUSEBUTTONDOWN:
			switch(event.button.button){
				case SDL_BUTTON_LEFT:
					break;
				case SDL_BUTTON_RIGHT:
					break;
				default:
					break;
			}
            break;
        case SDL_MOUSEBUTTONUP:
			switch(event.button.button){
				case SDL_BUTTON_LEFT:
					break;
				case SDL_BUTTON_RIGHT:
					break;
				default:
					break;
			}
            break;
		case SDL_WINDOWEVENT:
				switch(event.window.event){
					case SDL_WINDOWEVENT_RESIZED:
						resize(event.window.data1, event.window.data2);
						break;
				}
			break;
        case SDL_QUIT:
            run = false;
            break;
        default:
            break;
        }
    }
}

// Keyboard input processing routine.
void ModelViewer::keyInput(SDL_Keycode key){
    switch(key){
	    // Press ESC to quit.
        case SDLK_ESCAPE:
            run = false;
            break;
		case 'x':
            rotation0.set(0, 0, 0);
            rotation1.set(0, 0, 0);
            orientation.set(1, 0, 0, 0);
            rotation.set(1, 0, 0, 0);
            camera.reset();
            break;
		case 'z':
			showSolid = !showSolid;
			break;
		case 'c':
            showTexture = !showTexture;
            break;
        default:
            break;
    }
}

// Free allocated memory and other cleanup tasks on quit.
void ModelViewer::cleanup(int code){
    std::cout << "cleanup(" << code << "):" << std::endl;
	std::cout << " - delete textures(" << texNum << ')' << std::endl;
	if (texNum > 0){
		std::cout << "    - OpenGL: glDeleteTextures()" << std::endl;
		glDeleteTextures(texNum, textureID);
	}
	if (textureID != NULL){
		std::cout << "    - C++: delete[]" << std::endl;
		delete[] textureID;
		textureID = NULL;
	}
	if (window != NULL){
		SDL_DestroyWindow(window);
		window = NULL;
	}
	SDL_Quit();
}
