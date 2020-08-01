#ifndef GB_UNDO_COMMAND_H
#define GB_UNDO_COMMAND_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <functional>

// Qt
#include <QUndoCommand>

// Internal
#include "../../core/GbObject.h"

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Class representing an undo command
class UndoCommand : public QUndoCommand, public Object {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    UndoCommand(CoreEngine* core, const QString &text, QUndoCommand *parent = nullptr);
    UndoCommand(CoreEngine* core, QUndoCommand *parent = nullptr);
    ~UndoCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Convenience method to add this command to action manager
    void perform();


    /// @brief The description for this command (used in tooltips)
    virtual QString description() const {
        return QStringLiteral("No description");
    }

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "UndoCommand"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::UndoCommand"; }
    /// @}

protected:

    /// @brief Pointer to the core engine
    CoreEngine* m_engine;
};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif