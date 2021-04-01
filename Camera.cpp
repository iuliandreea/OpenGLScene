#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + this->cameraFrontDirection, glm::vec3(0.0, 1.0, 0.0));
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraFrontDirection * speed;
                break;
            case MOVE_BACKWARD:
                cameraPosition -= cameraFrontDirection * speed;
                break;
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
            case MOVE_LEFT: 
                cameraPosition -= cameraRightDirection * speed;
                break;
            case MOVE_UP: 
                cameraPosition += cameraUpDirection * speed;
                break;
            case MOVE_DOWN:
                cameraPosition -= cameraUpDirection * speed;
                break;
        }
    }

    void Camera::setStartingPosition() {
        this->cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f),
        this->cameraTarget = glm::vec3(0.0f, 0.0f, -10.0f),
        this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    void Camera::rotate(float pitch, float yaw) {

        cameraFrontDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection.y = sin(glm::radians(pitch));
        cameraFrontDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(cameraFrontDirection);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }
}