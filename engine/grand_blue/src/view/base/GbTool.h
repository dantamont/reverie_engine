#ifndef GB_TOOL_H 
#define GB_TOOL_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

// External
#include <QtWidgets>

// Project
#include "../../core/service/GbService.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Gb {
namespace View {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Tool
/// @brief A QWidget-based service

class Tool : public QWidget, public AbstractService {
    Q_OBJECT
public:
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Class Name and Namespace
    /// @{
    virtual const char* className() const override { return "Tool"; }
    virtual const char* namespaceName() const override { return "Gb::View::Tool"; }
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Constructors and Destructors
    /// @{
    /** @brief Create a Tool with the given name and optional parent
        @note Name must be unique
    */
    Tool(const QString& name, QWidget* parent = nullptr);
    Tool(const char* name, QWidget* parent = nullptr) : Tool(QString(name), parent) {};

    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Returns false for a Tool and its subclasses.
    virtual bool isService() const override final { return false; }

    /// @brief Returns False for a Tool and its subclasses.
    virtual bool isTool() const override final { return true; }

    /// @brief Returns information about every defined Qt Signal and Slot
    QString signalSlotInfo() const;
    /// @}
    //-----------------------------------------------------------------------------------------------------------------
    /// @name QObject overrides
    /// @{
    /// @brief Overrides QObject::event
    virtual bool event(QEvent* event) override;
    /// @}
private:
    /// Private Methods
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // View
} // Gb

#endif // GB_TOOL_H 





