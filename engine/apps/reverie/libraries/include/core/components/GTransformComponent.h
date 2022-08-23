#pragma once

// Project
#include "GComponent.h"
#include "fortress/containers/math/GTransform.h"
#include "core/rendering/shaders/GUniform.h"

namespace rev {

class SceneObject;


/// @class TransformComponent
/// @brief A component representing a transform, consisting of a translation, a rotation, and a scaling
class TransformComponent: public Component, public IndexedTransform {
public:
    /// @name Static
    /// @{

    /// @brief Enum for which transforms to inherit from a parent state
    enum InheritanceType {
        kAll, // inherit entire parent world matrix
        kTranslation, // inherit only the translation of the world state (e.g. won't orbit parent)
        kPreserveOrientation // preserves the local orientation of this transform 
    };

    /// @}

    /// @name Constructors/Destructor
    /// @{
    
    /// @note Trivial constructor, only exists so scene object can be constructed before transform component is added
    TransformComponent();

    TransformComponent(const std::shared_ptr<SceneObject>& object, std::vector<Matrix4x4>& worldMatrixVec, Uint32_t index);

    ~TransformComponent();

    /// @}

    /// @name Public Methods
    /// @{    

    const UniformData& worldMatrixUniform() const { return m_uniforms.m_worldMatrix; }

    /// @brief Max number of allowed components per scene object
    virtual int maxAllowed() const override{ return 1; }

    /// @brief Set value from a transform
    void set(const IndexedTransform& transform);

    /// @brief Compute world matrix
    void computeWorldMatrix() override;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const TransformComponent& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, TransformComponent& orObject);


    /// @}

protected:
    /// @name Friends 
    /// @{

    friend class AbstractTransformComponent;
    friend class AffineComponent;
    friend class SceneObject;

    /// @}

    /// @name Protected methods
    /// @{  

    /// @brief Initialize this transform
    void initialize();

    /// @}

    struct TransformUniforms {
        UniformData m_worldMatrix; ///< the world matrix uniform
    };
    TransformUniforms m_uniforms; ///< The uniforms for the transform component

};


} // end namespacing

