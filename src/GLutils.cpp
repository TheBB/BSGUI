#include <iostream>
#include <QGLFormat>

#include "GLutils.h"

void checkErrors(const std::string &prefix)
{
    while (GLenum err = glGetError())
    {
        switch (err)
        {
        case GL_NO_ERROR:
            std::cout << prefix << ": no error" << std::endl;
            break;

        case GL_INVALID_ENUM:
            std::cout << prefix << ": invalid enum" << std::endl;
            break;

        case GL_INVALID_VALUE:
            std::cout << prefix << ": invalid value" << std::endl;
            break;

        case GL_INVALID_OPERATION:
            std::cout << prefix << ": invalid operation" << std::endl;
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cout << prefix << ": invalid framebuffer operation" << std::endl;
            break;

        case GL_OUT_OF_MEMORY:
            std::cout << prefix << ": out of memory" << std::endl;
            break;

        case GL_STACK_UNDERFLOW:
            std::cout << prefix << ": stack underflow" << std::endl;
            break;

        case GL_STACK_OVERFLOW:
            std::cout << prefix << ": stack overflow" << std::endl;
            break;

        default:
            std::cout << prefix << ": something else" << std::endl;
            break;
        }

    }
}
