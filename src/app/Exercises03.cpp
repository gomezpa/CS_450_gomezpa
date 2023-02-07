#include <iostream>
#include <cstring>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
using namespace std;

static void error_callback(int error, const char* description) {
    cerr << "ERROR" << error << ": " << description <<endl;
}

int main(int argc, char **argv) {
    cout << "Begin OpenGL adventure!" <<endl;
    
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) {
        cerr << "GLFW FAILED!" <<endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, true);

    int windowWidth = 640;
    int windowHeight = 480;
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight,
                                            "Amazing OpenGL Window",
                                            NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    int fwidth = 0;
    int fheight = 0;

    glClearColor(0.9f, 0.2f, 0.7f, 1.0f);
    
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cout << "GLEW ERROR: ";
        cout << glewGetErrorString(err) << endl;
        glfwTerminate();
        exit(1);
    }

    while(!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &fwidth, &fheight);
        glViewport(0, 0, fwidth, fheight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        this_thread::sleep_for(chrono::milliseconds(15));
    }
   
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}