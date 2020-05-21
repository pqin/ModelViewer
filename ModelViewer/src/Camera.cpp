#include "Camera.h"

#include <cmath>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

Camera::Camera(){
    w = 400;
    h = 300;

    reset();
}

void Camera::reset(){
    zNear = 1.f;
    zFar = 100.f;

    vFoV = 45;

    // eye at origin
    eye = Vector_f::Zero;
    // look in -z direction
    center = Vector_f::Z_Axis;
    center = center * -1.f;
    // camera z-axis rotation
    up = Vector_f::Y_Axis;

    orientation.set(1, 0, 0, 0);

    zoomSensitivity = 0.1f;
}
void Camera::resize(int width, int height){
    w = width;
    h = height;
    float ratio = (float)w / (float)h;
    gluPerspective(vFoV, ratio, zNear, zFar);
}
void Camera::setView(void){
    Vector_f focus = center + eye;
    gluLookAt(
        eye.x, eye.y, eye.z,
        focus.x, focus.y, focus.z,
        up.x, up.y, up.z
    );
}
void Camera::setCenter(Vector_f center){
    this->center = center;
}

void Camera::translate(int mouseDeltaX, int mouseDeltaY){
    static float deltaX = 0.f;
    static float deltaY = 0.f;
    deltaX = (float)mouseDeltaX / (float)w;
    deltaY = (float)mouseDeltaY / (float)h;
    eye.x += deltaX;
    eye.y += deltaY;
}
void Camera::zoom(int mouseDeltaZ){
    static float deltaZ = 0.f;
    deltaZ = (float)mouseDeltaZ * zoomSensitivity;
    eye.z += deltaZ;
}

float Camera::rotate(int delta, Vector_f axis){
    float angle;
    if (axis == Vector_f::X_Axis){
        angle = asin( (float)delta / (float)h ) * (180.f/PI);
    } else if (axis == Vector_f::Y_Axis){
        angle = asin( (float)delta / (float)w ) * (180.f/PI);
    } else {
        angle = 0.f;
    }
    return angle;
}
void Camera::roll(float degDeltaZ){
    float radDeltaZ = degDeltaZ * (PI/180.f);
    float sine = sin(radDeltaZ);
    float cosine = cos(radDeltaZ);
    float x = up.x;
    float y = up.y;
    up.x = (x * cosine) - (y * sine);
    up.y = (y * cosine) + (x * sine);
}

