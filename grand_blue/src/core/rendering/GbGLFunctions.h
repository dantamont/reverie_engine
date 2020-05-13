/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_GL_EXTRA_FUNCTIONS_H
#define GB_GL_EXTRA_FUNCTIONS_H

#include <QOpenGLExtraFunctions>
#include <QDebug>

namespace Gb {  
namespace GL {


/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Statics
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief For loading 3D geometry (models) into memory
/// @detailed Stores positional data of a model in a VAO
class OpenGLFunctions: public QOpenGLExtraFunctions{
     
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{
    /** @property className
        @brief The name of this class
        @details Every subclass should redefine/override this property to
            return its name
    */
    virtual const char* className() const { return "OpenGLFunctions"; }

    /** @property namespaceName
        @brief The full namespace for this class
        @details Every subclass should redefine/override this property to
            return its full namespace.  The built in logging methods will
            use this value for the message category
    */
    virtual const char* namespaceName() const { return "Gb::GL::OpenGLFunctions"; }
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    OpenGLFunctions();
    ~OpenGLFunctions();
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /** @name Public Methods
        @{
    */
    /// @brief Print all errors that have been raised in GL
    static bool printGLError(const QString& errorMessage);

    /// @}
protected:

};

        
        
}} // End namespaces

#endif