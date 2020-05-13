#include "GbGLFunctions.h"

// Internal
#include "../GbLogger.h"

namespace Gb {   namespace GL {

/////////////////////////////////////////////////////////////////////////////////////////////
OpenGLFunctions::OpenGLFunctions():
    QOpenGLExtraFunctions()
{
    initializeOpenGLFunctions();
}
/////////////////////////////////////////////////////////////////////////////////////////////
OpenGLFunctions::~OpenGLFunctions()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool OpenGLFunctions::printGLError(const QString & errorMessage)
{
    OpenGLFunctions gl;
    unsigned int error = gl.glGetError();
    unsigned int count = 0;
    QString errorStr;
    while (count < 10) {
        if (error == 0) break;

        switch (error) {
            case GL_INVALID_OPERATION:      
                errorStr = "INVALID_OPERATION";      
                break;
            case GL_INVALID_ENUM:           
                errorStr = "INVALID_ENUM";           
                break;
            case GL_INVALID_VALUE:          
                errorStr = "INVALID_VALUE";         
                break;
            case GL_OUT_OF_MEMORY:          
                errorStr = "OUT_OF_MEMORY";          
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  
                errorStr = "INVALID_FRAMEBUFFER_OPERATION"; 
                break;
        }

        auto& lg = Gb::Logger::getInstance();
        QString errorMsg = errorMessage + ": " + error + ", " + errorStr;
        lg.logMessage(LogLevel::Warning, "OpenGLFunctions", errorMsg.toStdString().c_str());
        error = gl.glGetError();

        count++;
    }
    if (count) {
        return true;
    }
    else {
        return false;
    }
}

// End namespaces
}
}