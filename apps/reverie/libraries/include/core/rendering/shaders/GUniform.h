#pragma once

// Standard
#include <cstdlib>
#include <memory>
#include <vector>

// QT
#include <QString>

// Internal 
#include "fortress/containers/extern/tsl/robin_map.h"
#include "fortress/containers/GVariant.h"
#include "fortress/types/GStringView.h"
#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/GContainerExtensions.h"
#include "core/rendering/GGLFunctions.h"
#include <fortress/json/GJson.h>

namespace rev {

struct ShaderInputInfo;
class ShaderProgram;
class UniformContainer;

typedef std::vector<Vector3> Vec3List;
typedef std::vector<Vector4> Vec4List;

enum class ShaderVariableType {
    kCustomStruct = -2,
    kNone = -1,
    kBool = GL_BOOL,
    kInt = GL_INT,
    kFloat = GL_FLOAT,
    kDouble = GL_DOUBLE,
    kUVec2 = GL_UNSIGNED_INT_VEC2,
    kUVec3 = GL_UNSIGNED_INT_VEC3,
    kUVec4 = GL_UNSIGNED_INT_VEC4,
    kIVec2 = GL_INT_VEC2,
    kIVec3 = GL_INT_VEC3,
    kIVec4 = GL_INT_VEC4,
    kVec2 = GL_FLOAT_VEC2,
    kVec3 = GL_FLOAT_VEC3,
    kVec4 = GL_FLOAT_VEC4,
    kMat2 = GL_FLOAT_MAT2,
    kMat3 = GL_FLOAT_MAT3,
    kMat4 = GL_FLOAT_MAT4,
    kSamplerCube = GL_SAMPLER_CUBE,
    kSamplerCubeArray = GL_SAMPLER_CUBE_MAP_ARRAY,
    kSampler2D = GL_SAMPLER_2D,
    kSampler2DShadow = GL_SAMPLER_2D_SHADOW,
    kSampler2DArray = GL_SAMPLER_2D_ARRAY
};


/// @brief Uniform data to initialize a uniform
struct UniformData {
    static constexpr Uint32_t s_invalidStorageIndex = (Uint32_t)(-1);

    /// @brief Obtain the value of the uniform given its value container
    template<typename T>
    static const T& GetValue(const UniformData& data, const UniformContainer& uniformContainer) {
        return uniformContainer.getUniformValue<T>(data.m_storageIndex);
    }

    /// @brief Set a uniform's type
    template<typename UniformValueType>
    static void SetUniformType(UniformData& uniformData)
    {
        if constexpr (std::is_same_v<UniformValueType, bool>) {
            uniformData.m_uniformType = ShaderVariableType::kBool;
        }
        else if constexpr (std::is_same_v<UniformValueType, int>) {
uniformData.m_uniformType = ShaderVariableType::kInt;
        }
        else if constexpr (std::is_same_v<UniformValueType, float>) {
        uniformData.m_uniformType = ShaderVariableType::kFloat;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<float>>) {
        uniformData.m_uniformType = ShaderVariableType::kFloat;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2u>) {
        uniformData.m_uniformType = ShaderVariableType::kUVec2;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3u>) {
        uniformData.m_uniformType = ShaderVariableType::kUVec3;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4u>) {
        uniformData.m_uniformType = ShaderVariableType::kUVec4;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2i>) {
        uniformData.m_uniformType = ShaderVariableType::kIVec2;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3i>) {
        uniformData.m_uniformType = ShaderVariableType::kIVec3;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4i>) {
        uniformData.m_uniformType = ShaderVariableType::kIVec4;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2>) {
        uniformData.m_uniformType = ShaderVariableType::kVec2;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3>) {
        uniformData.m_uniformType = ShaderVariableType::kVec3;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Vector3>>) {
        uniformData.m_uniformType = ShaderVariableType::kVec3;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4>) {
        uniformData.m_uniformType = ShaderVariableType::kVec4;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Vector4>>) {
        uniformData.m_uniformType = ShaderVariableType::kVec4;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix2x2>) {
        uniformData.m_uniformType = ShaderVariableType::kMat2;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix3x3>) {
        uniformData.m_uniformType = ShaderVariableType::kMat3;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix4x4>) {
        uniformData.m_uniformType = ShaderVariableType::kMat4;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Matrix4x4>>) {
        uniformData.m_uniformType = ShaderVariableType::kMat4;
        }
        else {
        static_assert(false, "Unsupported uniform type");
        }
    }

    /// @brief Add the value of the uniform to the uniform container
    template<typename T>
    static inline void AddValue(UniformData& uniformData, const T& value, UniformContainer& uniformContainer) {
        uniformContainer.addUniformValue<T>(value, uniformData);
    }

    /// @brief Set the value of the uniform in the uniform container
    template<typename T>
    static inline void SetValue(UniformData& uniformData, const T& value, UniformContainer& uniformContainer) {
#ifdef DEBUG_MODE
        ShaderVariableType prevType = uniformData.m_uniformType;
        assert(prevType != ShaderVariableType::kNone && "Oopsies, no assigned type");
        Uint32_t prevCount = uniformData.m_count;
#endif

        // Set existing value
        uniformContainer.setUniformValue<T>(uniformData.m_storageIndex, value);

        // Check that type has not changed
#ifdef DEBUG_MODE
        SetUniformType<T>(uniformData);
        assert(prevCount == uniformData.m_count && "Error, count changed");
        assert(prevType == uniformData.m_uniformType && "Error, type changed");
#endif
    }

    /// @brief Constructor for data from a value type
    /// @tparam ValueType The type of value to initialize the uniform with
    /// @param value the value to initialize the uniform data with
    /// @param container the uniform container in which the value is stored
    template<typename ValueType>
    UniformData(const ValueType& value, UniformContainer& container)
    {
        setUniformType<ValueType>();
        container.addUniformValue<ValueType>(value, *this);
    }

    UniformData(ShaderVariableType uniformType, Uint32_t storageIndex, Int32_t count) :
        m_uniformType(uniformType),
        m_storageIndex(storageIndex),
        m_count(count)
    {
    }

    UniformData() = default;

    inline bool isEmpty() const noexcept{
        return m_storageIndex == UniformData::s_invalidStorageIndex;
    }

    /// @brief Set a uniform's type
    template<typename UniformValueType>
    inline void setUniformType()
    {
        UniformData::SetUniformType<UniformValueType>(*this);
    }

    /// @brief Obtain the value given its uniform container
    template<typename ValueType>
    inline const ValueType& getValue(const UniformContainer& uniformContainer) const {
        return UniformData::GetValue<ValueType>(*this, uniformContainer);
    }

    /// @brief Set the value of the uniform data in the uniform container
    template<typename ValueType>
    inline void setValue(const ValueType& value, UniformContainer& uniformContainer) {
        if (isEmpty()) {
            // Add new value
            UniformData::AddValue(*this, value, uniformContainer);
        }
        else {
            UniformData::SetValue(*this, value, uniformContainer);
        }
    }

    ShaderVariableType m_uniformType{ ShaderVariableType::kNone }; ///< The type of uniform
    Uint32_t m_storageIndex{ s_invalidStorageIndex }; ///< The index at which the uniform's value is stored in its uniform container
    Int32_t m_count{ -1 }; ///< If positive, the uniform is an array or vector of items, and the count is the number of items
};

template<typename ValueType>
class StrongTypeUniformData {
public:

    StrongTypeUniformData() {
        UniformData::SetUniformType<ValueType>(m_data);
    }

    StrongTypeUniformData(const ValueType& value, UniformContainer& uniformContainer):
        StrongTypeUniformData()
    {
        setValue(value, uniformContainer);
    }

    inline const UniformData& getData() const noexcept{
        return m_data;
    }

    /// @brief Obtain the value given its uniform container
    const ValueType& getValue(const UniformContainer& uniformContainer) const {
        return m_data.getValue<ValueType>(uniformContainer);
    }

    /// @brief Set the value of the uniform in the uniform container
    inline void setValue(const ValueType& value, UniformContainer& uniformContainer) {
        m_data.setValue(value, uniformContainer);
    }

private:

    UniformData m_data;
};

/// @class Uniform
/// @brief struct representing a uniform for an openGLShader
/// @see https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/
class Uniform 
{
public:
    /// @name Static
    /// @{

    static tsl::robin_map<ShaderVariableType, std::type_index> s_uniformGLTypeMap;
    static tsl::robin_map<QString, ShaderVariableType> s_uniformTypeStrMap;
    static tsl::robin_map<ShaderVariableType, QString> s_uniformStrTypeMap;

    static Uniform FromJson(const nlohmann::json& json, ShaderProgram& shaderProgram, UniformContainer& container);

    /// @}

    /// @name Constructors/Destructor
    /// @{
    /// @}

    Uniform() = default;
    Uniform(const Uniform& other);
    Uniform(Uniform&& other);

    /// @brief 
    /// @tparam ValueType The value to initialize the uniform with
    /// @param id 
    /// @param value 
    /// @param container 
    template<typename ValueType>
    Uniform(Int32_t id, const ValueType& value, UniformContainer& container):
        m_id(id),
        m_data(value, container)
    {
    }

    explicit Uniform(Int32_t id, const UniformData& data);

    explicit Uniform(Int32_t id, UniformData&& data);

    ~Uniform();
    /// @}


    /// @name Operators
    /// @{

    /// @brief Conversion to QString
    QString asString(const UniformContainer& uc) const;

    Uniform& operator=(const Uniform& rhs);
    Uniform& operator=(Uniform&& rhs);

    /// @}

    /// @name Public methods
    /// @{

    bool isArrayType() const {
        return m_data.m_count != -1;
    }

    inline bool isEmpty() const {
        return m_data.isEmpty();
    }

    const UniformData& getData() const { return m_data; }

    Int32_t getId() const { return m_id; }
    void setId(Int32_t id) { m_id = id; }

    ShaderVariableType getType() const { return m_data.m_uniformType; }
    Uint32_t getStorageIndex() const { return m_data.m_storageIndex; }
    Int32_t getCount() const { return m_data.m_count; }

    const GString& getName(const ShaderProgram& shaderProgram) const;

    /// @brief Obtain the value of the uniform given its value container
    template<typename T>
    const T& getValue(const UniformContainer& uniformContainer) const {
        return UniformData::GetValue<T>(m_data, uniformContainer);
    }

    /// @brief Set a uniform's type
    template<typename UniformValueType>
    void setUniformType() 
    {
        m_data.setUniformType<UniformValueType>();
    }

    /// @brief Add the value of the uniform to the uniform container
    template<typename T>
    inline void addValue(const T& value, UniformContainer& uniformContainer) {
        UniformData::AddValue(m_data, value, uniformContainer);
    }

    /// @brief Set the value of the uniform in the uniform container
    template<typename T>
    void setValue(const T& value, UniformContainer& uniformContainer) {
        if (isEmpty()) {
            // Add new value
            UniformData::AddValue(m_data, value, uniformContainer);
        }
        else {
            UniformData::SetValue(m_data, value, uniformContainer);
        }
    }

    /// @brief Whether or not the uniform's type ID matches the given uniform info struct
    bool matchesInfo(const ShaderInputInfo& typeInfo) const;

    /// @}

    /// @name Friend Functions
    /// @{

    /// @brief Convert from the given class type to JSON
    json asJson(const ShaderProgram& shaderProgram, const UniformContainer& uc) const;

    /// @brief Convert from JSON to the given class type
    void fromJson(const nlohmann::json& korJson, ShaderProgram& shaderProgram, UniformContainer& container);

    /// @}

protected:
    /// @name Static
    /// @{
    friend class ShaderProgram;
    friend class UniformContainer;

    /// @}

    /// @name Protected methods
    /// @{

    /// @brief Load from JSON, setting storage index and count
    void loadValue(UniformContainer& uc, const nlohmann::json& json);

    /// @brief Get value as JSON
    json valueAsJson(const UniformContainer& uc) const;

    /// @}

    /// @name Protected members
    /// @{

    Int32_t m_id{ -1 }; ///< The uniform ID
    UniformData m_data; ///< The uniform data

    /// @}

};


} // End namespaces
