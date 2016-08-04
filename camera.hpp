/* Author: Brett A. Blashko
 * ID: V00759982
*/

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/ext.hpp"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

class Camera
{
public:
    Camera(GLFWwindow *window){
        _position = glm::vec3(-35.0f, 20.0f, 35.0f);
        _horizontal_angle = 2.35f;
        _vertical_angle = -0.5f;
        _initial_FOV = 45.0f;
        _speed = 5.0f;
        _mouse_speed = 0.005;
        glfwGetWindowSize(window, &_window_width, &_window_height);
        glfwSetCursorPos(window, _window_width/2, _window_height/2);
    };

    const glm::vec3& position() const
    {
        return _position;
    };

    glm::vec3& position()
    {
        return _position;
    };

    void update_camera_from_inputs(GLFWwindow *window)
    {
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - _lastTime);
        _lastTime = currentTime;

        //update from mouse
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwSetCursorPos(window, _window_width/2, _window_height/2);

        _horizontal_angle += _mouse_speed * float(_window_width/2 - xpos);
        _vertical_angle += _mouse_speed * float(_window_height/2 - ypos);

        _direction = glm::vec3(
            cos(_vertical_angle) * sin(_horizontal_angle),
            sin(_vertical_angle),
            cos(_vertical_angle) * cos(_horizontal_angle)
        );

        _right = glm::vec3(
            sin(_horizontal_angle - 3.14f/2.0f),
            0,
            cos(_horizontal_angle - 3.14f/2.0f)
        );

        _up = glm::cross(_right, _direction);

        //udate from keyboard
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            _position += _direction * deltaTime * _speed;
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            _position -= _direction * deltaTime * _speed;
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            _position -= _right * deltaTime * _speed;
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            _position += _right * deltaTime * _speed;
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            _position += _up * deltaTime * _speed;
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            _position -= _up * deltaTime * _speed;
        }
        // _FOV = _initial_FOV - 5 * scroll_callback();
        // std::cout << "position = " << to_string(_position) << std::endl;
        _FOV = _initial_FOV;
    };

    glm::mat4 getPerspectiveMatrix()
    {
        return glm::perspective(_FOV, 4.0f / 3.0f, 0.1f, 256.0f);
    };

    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(
            _position,
            _position + _direction,
            _up
        );
    };
private:

    glm::vec3 _position;
    float _horizontal_angle;
    float _vertical_angle;
    float _initial_FOV;
    float _speed;
    float _mouse_speed;

    int _window_width;
    int _window_height;

    float _lastTime = 0;
    float _FOV;
    glm::vec3 _direction;
    glm::vec3 _up;
    glm::vec3 _right;
};
