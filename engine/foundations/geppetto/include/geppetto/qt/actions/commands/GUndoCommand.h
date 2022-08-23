#pragma once

// Standard
#include <functional>

// Qt
#include <QUndoCommand>

// Internal
#include "fortress/types/GIdentifiable.h"

namespace rev {

class WidgetManager;

/// @brief Class representing an undo command
class UndoCommand : public QUndoCommand{
public:

    /// @todo Maybe auto-generate this, or just remove entirely and replace with dynamic casting
    enum ECommandType {
        eUnassigned = -1,
        eAddScenario,
        eAddSceneObject,
        eRemoveSceneObject,
        eChangeSceneRelatedName,
        eReparentSceneObject,
        eCopySceneObject,
        eAddComponent,
        eRemoveComponent,
        eLoadTexture,
        eAddMaterial,
        eCopyMaterial,
        eAddMesh,
        eAddModel,
        eLoadModel,
        eCopyModel,
        eLoadAudio,
        eAddShader,
        eDeleteMaterial,
        eDeleteModel,
        eDeleteShader
    };

    /// @name Constructors/Destructor
    /// @{
    UndoCommand(WidgetManager* wm, const QString &text, QUndoCommand *parent = nullptr);
    UndoCommand(WidgetManager* wm, QUndoCommand *parent = nullptr);
    ~UndoCommand();
    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Return the type of command;
    virtual ECommandType undoCommandType() const = 0;

    Uint32_t getId() const { return m_id; }

    /// @brief Convenience method to add this command to action manager
    void perform();

    /// @brief The description for this command (used in tooltips)
    virtual QString description() const {
        return QStringLiteral("No description");
    }

    /// @}

protected:

    Uint32_t m_id{ 0 }; ///< The ID of the command
    WidgetManager* m_widgetManager{ nullptr }; ///< Pointer to the widget manager

    static Uint32_t s_undoCommandCount; ///< Count for tracking IDs of undo commands
};


} // End namespaces
