#ifndef GB_WIDGET_COMMANDS_H
#define GB_WIDGET_COMMANDS_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Qt
#include <QtWidgets>
#include <QString>

// Internal
#include "../GUndoCommand.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////

namespace View {
}


/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @class EditLineEditCommand
/// @brief Command representing a change to a line edit
class EditLineEditCommand : public UndoCommand {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static methods
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    EditLineEditCommand(CoreEngine* core, QLineEdit* lineEdit, const QString& newText);
    ~EditLineEditCommand();
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Redoes the add scenario command
    virtual void redo() override;

    /// @brief Undoes the add scenario command
    virtual void undo() override;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "EditLineEditCommand"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::EditLineEditCommand"; }
    /// @}

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected members
    /// @{

    /// @brief Pointer to the line edit
    QLineEdit* m_lineEdit;

    /// @brief New text
    QString m_newText;

    /// @brief Original text
    QString m_oldText;

    /// @}

};



/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif