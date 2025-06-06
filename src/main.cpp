#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#include <array>

#include "ShaderHandler.h"
#include "Input.h"
#include "PickingTexture.h"
#include "CollisionPlane.h"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // These 2 lines are just so I can set the window size to be the size of the monitor in glfwCreateWindow
    GLFWmonitor* monitor { glfwGetPrimaryMonitor() };
    const GLFWvidmode* mode { glfwGetVideoMode(monitor) };
    GLFWwindow* window { glfwCreateWindow(mode->width, mode->height, "Rigid-body", monitor, NULL) };
    if (window == NULL)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // So the cursor won't hit the edge of the screen and stop
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // Query and load all OpenGL extensions allowed by your drivers
    // Allows us to access features/extensions not in the core OpenGL specification
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Glew initialization failed");
    }
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    compileShaders();

    // Handle objects
    //MassSpringPlane massSpringPlane{ 5.0f, 100 };
    glm::mat4 rotation{ glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }) };
    rotation = glm::rotate(rotation, glm::radians(-90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
    //TetraObject massSpringObject{argv[1], 10.0f, rotation, false };
    const CollisionPlane collisionPlane{ 10.0f, -10.0f };
    PickingTexture pickingTexture{ mode->width, mode->height };

    // ****************
    // Scene properties
    // ****************
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    const glm::mat4 projection { glm::perspective(glm::radians(45.0f), (float)mode->width / mode->height, 0.1f, 500.0f) };
    constexpr glm::vec3 viewDir { 0.0f, 0.0f, 1.0f };

    GLfloat xCameraRotateAmountObject{ 0.0f };
    GLfloat zCameraRotateAmountObject{ 0.0f };
    GLfloat viewDistance{-30.0f};

    // Parameters to change light rotation
	GLfloat zLightRotateAmount{ 73.0f };
	GLfloat yLightRotateAmount{ -45.0f };

    // Set uniform variables in shaders that won't change
    glUseProgram(mainShader);
    glUniformMatrix4fv(glGetUniformLocation(mainShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    constexpr float fixedDeltaTime{ 1.0f / 60.0f };
    float accumulator{ 0.0f };
    GLfloat lastUpdateTime{ static_cast<GLfloat>(glfwGetTime()) };

    GLuint selectedTriangle{ 0xFFFFFFFFu };
    while (!glfwWindowShouldClose(window)) 
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Input
        GLfloat xRotateAmountChange;
        GLfloat zRotateAmountChange;
        processMouseInputObjectRotation(window, xRotateAmountChange, zRotateAmountChange);

        GLfloat cameraDistanceChange;
        processMouseInputObjectDistance(window, cameraDistanceChange);

        GLfloat zLightRotateChange;
        GLfloat yLightRotateChange;
        processMouseInputLightControls(window, yLightRotateChange, zLightRotateChange);

        processKeyboardInputExit(window);

        // Apply changes from input
        viewDistance += cameraDistanceChange * 0.05;

        zCameraRotateAmountObject += xRotateAmountChange;
        xCameraRotateAmountObject += zRotateAmountChange;
        zCameraRotateAmountObject = fmod(zCameraRotateAmountObject, 360.0f);
        xCameraRotateAmountObject = fmod(xCameraRotateAmountObject, 360.0f);

        yLightRotateAmount += yLightRotateChange;
        zLightRotateAmount += zLightRotateChange;
		yLightRotateAmount = fmod(yLightRotateAmount, 360.0f);
		zLightRotateAmount = fmod(zLightRotateAmount, 360.0f);

        // Setup transforms
        glm::mat4 view { glm::translate(glm::mat4{1.0f}, viewDir * viewDistance) };
        view = glm::rotate(view, glm::radians(-xCameraRotateAmountObject), glm::vec3{1.0f, 0.0f,0.0f});
        view = glm::rotate(view, glm::radians(-zCameraRotateAmountObject), glm::vec3{0.0f, 1.0f, 0.0f});

		glm::mat4 lightRotateMatrix { glm::rotate(glm::mat4{ 1.0f }, glm::radians(zLightRotateAmount), glm::vec3(0.0f, 0.0f, 1.0f)) };
		lightRotateMatrix = glm::rotate(lightRotateMatrix, glm::radians(yLightRotateAmount), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 lightDir { glm::vec3{lightRotateMatrix * glm::vec4{1.0f, 0.0f, 0.0f, 0.0f}} };
        const glm::vec3 lightDirInViewSpace { glm::normalize(view * glm::vec4(lightDir, 0.0f)) };

        // Get selected triangle with mouse input
        bool isTryingToPickTriangle{ processMouseInputIsTryingToPick(window, selectedTriangle) };
        if (isTryingToPickTriangle)
        {
            pickingTexture.bind();
            glViewport(0, 0, mode->width, mode->height);
            glClear(GL_DEPTH_BUFFER_BIT);
            GLuint clearColor[] { 0, 0, 0xFFFFFFFFu }; // Doing this so that I can tell if I've selected the background instead of the object
            glClearBufferuiv(GL_COLOR, 0, clearColor);

            // Render info about the object to a framebuffer so we can see which triangle we're clicking on
            glUseProgram(pickingShader);
            glUniformMatrix4fv(glGetUniformLocation(pickingShader, "mvp"), 1, GL_FALSE, glm::value_ptr(projection * view));
            glUniform1ui(glGetUniformLocation(pickingShader, "objectIndex"), 1);
            //massSpringObject.draw();

            pickingTexture.unbind();
            glViewport(0, 0, mode->width, mode->height);

            // Figure out which triangle we're clicking
            int xCursorPosPicking;
            int yCursorPosPicking;
            processMouseInputPickingControls(window, xCursorPosPicking, yCursorPosPicking);
            PickingTexture::PixelInfo pixel{ pickingTexture.readPixel(xCursorPosPicking, mode->height - yCursorPosPicking - 1) };
            selectedTriangle = pixel.primitiveID;
        }

        const GLfloat currentTime{ static_cast<GLfloat>(glfwGetTime()) };
        const GLfloat deltaTime{ currentTime - lastUpdateTime };
        lastUpdateTime = currentTime;
        accumulator += deltaTime;
        accumulator = std::min(accumulator, 0.35f);

        while (accumulator >= fixedDeltaTime)
        {
            //massSpringObject.setForceToZero();

            if (selectedTriangle != 0xFFFFFFFFu)
            {
                constexpr GLfloat forceMagnitude{ 10000.0f };
                const glm::vec3 forceWorldSpace{ processKeyboardInputForceVec(window) * forceMagnitude };
                //massSpringObject.applyForceFromMouse(selectedTriangle, forceWorldSpace);
            }

            //massSpringObject.updatePhysics(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }

        // Render selected triangle
        if (selectedTriangle != 0xFFFFFFFFu)
        {
            // Render selected triangle a different color
            glEnable(GL_POLYGON_OFFSET_FILL); // This basically pushes it ahead of the triangle that will be rendered in the normal render pass
            glPolygonOffset(-1.0f, -1.0f);
            glUseProgram(highlightShader);
            glUniform1ui(glGetUniformLocation(highlightShader, "selectedTriangle"), selectedTriangle);
            glUniformMatrix4fv(glGetUniformLocation(highlightShader, "mvp"), 1, GL_FALSE, glm::value_ptr(projection * view));
            //massSpringObject.draw();
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // Render object to screen
        glUseProgram(mainShader);
        glUniformMatrix4fv(glGetUniformLocation(mainShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(mainShader, "lightDir"), 1, glm::value_ptr(lightDirInViewSpace));

        // Render collision plane
        glUniform3fv(glGetUniformLocation(mainShader, "diffuseMaterialColor"), 1, glm::value_ptr(glm::vec3{ 0.0f, 0.5f, 0.0f }));
        collisionPlane.draw();

        // Render mass-spring objct
        glUniform3fv(glGetUniformLocation(mainShader, "diffuseMaterialColor"), 1, glm::value_ptr(glm::vec3{ 0.0f, 0.7f, 1.0f }));
        //massSpringPlane.draw();
        //massSpringObject.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

