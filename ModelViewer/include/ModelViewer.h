#ifndef HX_UTL_MODEL_VIEWER_H
#define HX_UTL_MODEL_VIEWER_H

#include "Model.h"
#include "Quaternion.h"
#include "Camera.h"

#include <SDL2/SDL.h>
#include <windows.h>
#include <GL/GL.h>

class ModelViewer {
private:
    Camera camera;
    Model model;
	SDL_Window *window;
    GLuint *textureID;
    GLsizei texNum;

    Vector_f center;
    Vector_f rotation0;    // previous rotation
    Vector_f rotation1;    // current rotation
    Quaternion orientation, rotation;
    float rotationAngle;
    Vector_f rotationAxis;

    GLfloat lightPosition[4];

    bool showSolid;
    bool showTexture;
    bool run;

    void genTexture(SDL_Surface *image, GLuint id);
    void keyInput(SDL_Keycode key);
    void process_events();
    void update();

    void drawScene();
    void orientLocal();

public:
    ModelViewer();
    bool load(char *filename);
    std::string getFilename();
    std::string getDirectory();
    void runLoop();
    bool setup(std::string title, int w, int h);
    void cleanup(int code);
    void resize(int w, int h);
};

#endif
