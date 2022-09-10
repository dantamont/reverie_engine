#pragma once

#include <vector>

#include "fortress/containers/math/GMatrix.h"
#include "fortress/containers/GStrictGrowContainer.h"
#include "fortress/numeric/GSizedTypes.h"

namespace rev {

class UniformData;

/// @brief Class for managing a set of uniforms
class UniformContainer {
protected:

    /// @todo Make this a strict grow container
    /// @brief Encapsulate a vector of uniforms of type T
    /// @tparam T the type of uniform
    template<typename T>
    struct UniformVector {
        static constexpr Uint32_t s_defaultReservedSize{ 0 };

        UniformVector();

        void clear() {
            m_uniforms.clear();
        }

        constexpr Uint32_t stride() {
            return sizeof(T);
        }

        inline size_t size() const {
            return m_uniforms.size();
        }

        inline T& operator[](size_t index) {
            return m_uniforms[index];
        }
        inline const T& operator[](size_t index) const {
            return m_uniforms[index];
        }

        inline void push_back(const T& value) {
            m_uniforms.push_back(value);
        }

        std::vector<T> m_uniforms;
    };

public:
    UniformContainer();
    ~UniformContainer() = default;

    /// @brief Return the size of the vector of the given type
    template<typename UniformValueType>
    UniformVector<UniformValueType>& getUniformVector() {
        if constexpr (std::is_same_v<UniformValueType, bool>) {
            return m_bool;
        }
        else if constexpr (std::is_same_v<UniformValueType, int>) {
            return m_int;
        }
        else if constexpr (std::is_same_v<UniformValueType, float>) {
            return m_float;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2u>) {
            return m_vector2u;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3u>) {
            return m_vector3u;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4u>) {
            return m_vector4u;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2i>) {
            return m_vector2i;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3i>) {
            return m_vector3i;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4i>) {
            return m_vector4i;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2>) {
            return m_vector2;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3>) {
            return m_vector3;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4>) {
            return m_vector4;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix2x2>) {
            return m_matrix2x2;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix3x3>) {
            return m_matrix3x3;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix4x4>) {
            return m_matrix4x4;
        }
        else {
            static_assert(false, "Unsupported uniform type");
        }
    }

    /// @brief Clear all of the uniforms in the container
    void clear() {
        m_bool.clear(); 
        m_int.clear();
        m_float.clear();
        m_vector2i.clear();
        m_vector3i.clear();
        m_vector4i.clear();
        m_vector2u.clear();
        m_vector3u.clear();
        m_vector4u.clear();
        m_vector2.clear();
        m_vector3.clear();
        m_vector4.clear();
        m_matrix2x2.clear();
        m_matrix3x3.clear();
        m_matrix4x4.clear();
    }

protected:

    /// @brief Obtain a uniform value
    template<typename UniformValueType>
    const UniformValueType& getUniformValue(Uint32_t storageIndex) const {
        if constexpr (std::is_same_v<UniformValueType, bool>) {
            return m_bool[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, int>) {
            return m_int[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, float>) {
            return m_float[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2u>) {
            return m_vector2u[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3u>) {
            return m_vector3u[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4u>) {
            return m_vector4u[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2i>) {
            return m_vector2i[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3i>) {
            return m_vector3i[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4i>) {
            return m_vector4i[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2>) {
            return m_vector2[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3>) {
            return m_vector3[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4>) {
            return m_vector4[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix2x2>) {
            return m_matrix2x2[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix3x3>) {
            return m_matrix3x3[storageIndex];
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix4x4>) {
            return m_matrix4x4[storageIndex];
        }
        else {
            static_assert(false, "Unsupported uniform type");
        }
    }

    /// @brief Add a uniform's value, and return the index/count of the new values
    template<typename UniformValueType>
    void addUniformValue(const UniformValueType& value, UniformData& outUniform) {
        UniformData::SetUniformType<UniformValueType>(outUniform);
        if constexpr (std::is_same_v<UniformValueType, bool>) {
            outUniform.m_storageIndex = m_bool.m_uniforms.size();
            outUniform.m_count = -1;
            m_bool.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, int>) {
            outUniform.m_storageIndex = m_int.m_uniforms.size();
            outUniform.m_count = -1;
            m_int.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, float>) {
            outUniform.m_storageIndex = m_float.m_uniforms.size();
            outUniform.m_count = -1;
            m_float.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<float>>) {
            m_float.m_uniforms.reserve(value.size());
            outUniform.m_storageIndex = m_float.m_uniforms.size();
            outUniform.m_count = value.size();
            for (size_t i = 0; i < outUniform.m_count; i++) {
                m_float.push_back(value[i]);
            }
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2u>) {
            outUniform.m_storageIndex = m_vector2u.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector2u.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3u>) {
            outUniform.m_storageIndex = m_vector3u.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector3u.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4u>) {
            outUniform.m_storageIndex = m_vector4u.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector4u.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2i>) {
            outUniform.m_storageIndex = m_vector2i.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector2i.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3i>) {
            outUniform.m_storageIndex = m_vector3i.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector3i.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4i>) {
            outUniform.m_storageIndex = m_vector4i.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector4i.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2>) {
            outUniform.m_storageIndex = m_vector2.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector2.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3>) {
            outUniform.m_storageIndex = m_vector3.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector3.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Vector3>>) {
            m_vector3.m_uniforms.reserve(value.size());
            outUniform.m_storageIndex = m_vector3.m_uniforms.size();
            outUniform.m_count = value.size();
            for (size_t i = 0; i < outUniform.m_count; i++) {
                m_vector3.push_back(value[i]);
            }
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4>) {
            outUniform.m_storageIndex = m_vector4.m_uniforms.size();
            outUniform.m_count = -1;
            m_vector4.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Vector4>>) {
            m_vector4.m_uniforms.reserve(value.size());
            outUniform.m_storageIndex = m_vector4.m_uniforms.size();
            outUniform.m_count = value.size();
            for (size_t i = 0; i < outUniform.m_count; i++) {
                m_vector4.push_back(value[i]);
            }
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix2x2>) {
            outUniform.m_storageIndex = m_matrix2x2.m_uniforms.size();
            outUniform.m_count = -1;
            m_matrix2x2.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix3x3>) {
            outUniform.m_storageIndex = m_matrix3x3.m_uniforms.size();
            outUniform.m_count = -1;
            m_matrix3x3.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix4x4>) {
            outUniform.m_storageIndex = m_matrix4x4.m_uniforms.size();
            outUniform.m_count = -1;
            m_matrix4x4.push_back(value);
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Matrix4x4>>) {
            m_matrix4x4.m_uniforms.reserve(value.size());
            outUniform.m_storageIndex = m_matrix4x4.m_uniforms.size();
            outUniform.m_count = value.size();
            for (size_t i = 0; i < outUniform.m_count; i++) {
                m_matrix4x4.push_back(value[i]);
            }
        }
        else {
            static_assert(false, "Unsupported uniform type");
        }
    }

    /// @brief Set a uniform's value at the given index
    template<typename UniformValueType>
    void setUniformValue(Uint32_t storageIndex, const UniformValueType& value) {
        if constexpr (std::is_same_v<UniformValueType, bool>) {
            m_bool[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, int>) {
            m_int[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, float>) {
            m_float[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<float>>) {
            memcpy(&m_float[storageIndex], value.data(), sizeof(float) * value.size());
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2u>) {
            m_vector2u[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3u>) {
            m_vector3u[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4u>) {
            m_vector4u[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2i>) {
            m_vector2i[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3i>) {
            m_vector3i[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4i>) {
            m_vector4i[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector2>) {
            m_vector2[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector3>) {
            m_vector3[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Vector3>>) {
            memcpy(&m_vector3[storageIndex], value.data(), sizeof(Vector3) * value.size());
        }
        else if constexpr (std::is_same_v<UniformValueType, Vector4>) {
            m_vector4[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Vector4>>) {
            memcpy(&m_vector4[storageIndex], value.data(), sizeof(Vector4) * value.size());
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix2x2>) {
            m_matrix2x2[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix3x3>) {
            m_matrix3x3[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, Matrix4x4>) {
            m_matrix4x4[storageIndex] = value;
        }
        else if constexpr (std::is_same_v<UniformValueType, std::vector<Matrix4x4>>) {
            memcpy(&m_matrix4x4[storageIndex], value.data(), sizeof(Matrix4x4) * value.size());
        }
        else {
            static_assert(false, "Unsupported uniform type");
        }
    }

private:
    friend class Uniform;
    friend class UniformData;

    UniformVector<Uint8_t> m_bool; ///< Because std::vector<bool> is a monster
    UniformVector<Int32_t> m_int;
    UniformVector<Float32_t> m_float;
    UniformVector<Vector2i> m_vector2i; 
    UniformVector<Vector3i> m_vector3i; 
    UniformVector<Vector4i> m_vector4i;
    UniformVector<Vector2u> m_vector2u; 
    UniformVector<Vector3u> m_vector3u; 
    UniformVector<Vector4u> m_vector4u;
    UniformVector<Vector2> m_vector2;
    UniformVector<Vector3> m_vector3;
    UniformVector<Vector4> m_vector4;
    UniformVector<Matrix2x2> m_matrix2x2;
    UniformVector<Matrix3x3> m_matrix3x3; 
    UniformVector<Matrix4x4> m_matrix4x4;
};

template<typename T>
inline UniformContainer::UniformVector<T>::UniformVector()
{
    m_uniforms.reserve(s_defaultReservedSize);
}

} // End namespaces
