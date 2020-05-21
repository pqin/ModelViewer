#ifndef HX_UTL_CAMERA_H
#define HX_UTL_CAMERA_H

#include "Vector.h"
#include "Quaternion.h"

class Camera {
private:
    int w, h;   // window width and height
    float zNear, zFar;  // near and far clipping planes
    float vFoV; // vertical (y-axis) field of view angle 
    Vector_f eye, center, up;   // eye position, point looked at, camera up direction
    Quaternion orientation; // camera rotation orientation
    float zoomSensitivity;  // zoom scale factor per scrollwheel click
public:
    Camera();

    void reset();
    void resize(int width, int height);
    void setView(void);
    void setCenter(Vector_f center);

    void translate(int mouseDeltaX, int mouseDeltaY);
    void zoom(int mouseDeltaZ);
    float rotate(int delta, Vector_f axis);
    void roll(float degDeltaZ);
};
#endif
