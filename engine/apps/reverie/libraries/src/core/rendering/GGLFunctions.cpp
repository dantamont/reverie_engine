#include "core/rendering/GGLFunctions.h"

// Internal
#include "logging/GLogger.h"
#include "fortress/string/GStringView.h"
#include "fortress/system/path/GFile.h"

namespace rev {   
namespace gl {


const std::shared_ptr<OpenGLFunctions>& OpenGLFunctions::Functions() {
    if (!s_glFunctions) {
        s_glFunctions = std::make_shared<OpenGLFunctions>(true);
    }   
    return s_glFunctions;
}

size_t OpenGLFunctions::MaxNumTextureUnitsPerShader()
{
    Functions()->glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &s_maxTextureUnits);
    return (size_t)s_maxTextureUnits;
}

OpenGLFunctions::OpenGLFunctions(bool initialize):
    QOpenGLExtraFunctions()
{
    if (initialize) {
        initializeOpenGLFunctions();
    }
}

OpenGLFunctions::~OpenGLFunctions()
{
}

bool OpenGLFunctions::printGLError(const char * errorMessage)
{
    return printGLError(GStringView(errorMessage));
}

bool OpenGLFunctions::printGLError(const GStringView & errorMessage)
{
    if (!s_glFunctions) {
        s_glFunctions = std::make_shared<OpenGLFunctions>(true);
    }
    OpenGLFunctions& gl = *s_glFunctions;
    unsigned int error = gl.glGetError();
    unsigned int count = 0;
    GString errorStr;
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

        auto& lg = rev::Logger::Instance();
        GString errorMsg = errorMessage + ": " + GString::FromNumber(error) + ", " + errorStr;
        lg.logMessage(LogLevel::Warning, "OpenGLFunctions", errorMsg.c_str());
        lg.AddErrorString(errorMsg);
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

void OpenGLFunctions::printMessageCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam)
{
    Q_UNUSED(source);
    Q_UNUSED(type);
    Q_UNUSED(id);
    Q_UNUSED(severity);
    Q_UNUSED(length);
    Q_UNUSED(userParam);
    auto& lg = rev::Logger::Instance();
    GString msg = GString("Debug Message: ") + message;
    lg.logMessage(LogLevel::Debug, "OpenGLFunctions", msg.c_str());
    const GString fileDir = _SOLUTION_DIR + GString("/logs/gl.txt");
    GFile myFile(fileDir);
    myFile.create(true);
    if (myFile.exists()) {
        myFile.writeLines(std::vector<GString>{msg});
    }
}

std::shared_ptr<OpenGLFunctions> OpenGLFunctions::s_glFunctions = nullptr;


GLint OpenGLFunctions::s_maxTextureUnits = -1;



// End namespaces
}
}