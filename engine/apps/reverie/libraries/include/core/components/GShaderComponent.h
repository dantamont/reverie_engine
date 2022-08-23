#pragma once

// Qt
#include <QString>
#include <QJsonValue>

// Project
#include "GComponent.h"
#include "core/rendering/renderer/GRenderSettings.h"
#include "core/mixins/GRenderable.h"
#include "core/rendering/shaders/GShaderPreset.h"

namespace rev {

class ShaderPreset;

/// @class ShaderComponent
/// @brief  A renderer component to be attached to a scene object
class ShaderComponent: public Component {
public:
    /// @name Constructors/Destructor
    /// @{
    ShaderComponent();
    ShaderComponent(const std::shared_ptr<SceneObject>& object);
    ~ShaderComponent();

    /// @}

    /// @name Public Methods
    /// @{

    /// @brief Enable the behavior of this script component
    virtual void enable() override;

    /// @brief Disable the behavior of this script component
    virtual void disable() override;

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override { return 1; }

    /// @}

    /// @name Properties
    /// @{

    const std::shared_ptr<const ShaderPreset> shaderPreset() const;
    void setShaderPreset(const std::shared_ptr<ShaderPreset>& preset);

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const ShaderComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, ShaderComponent& orObject);


    /// @}

protected:

    /// @brief Recreate draw commands on the change of a shader preset
    void onShaderPresetChanged();

    /// @name Protected Members
    /// @{

    std::weak_ptr<ShaderPreset> m_shaderPreset;

    /// @}


};


} // end namespacing
