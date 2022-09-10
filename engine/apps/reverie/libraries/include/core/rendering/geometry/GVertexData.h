#pragma once

#include <array>
#include <vector>

// Internal
#include "fortress/containers/math/GVector.h"
#include "fortress/templates/GTemplates.h"

namespace rev {

/// @brief Metadata for indexing a set of geometry's vertex data in a VAO
//struct VertexAttributeStorageData {
//    Int32_t m_indicesIndex; ///< The index of the indices to use in the VAO
//    Int32_t m_
//};


/// @brief Enum of valid buffer data types
/// @details Enum value represents position in shader
enum class MeshVertexAttributeType {
    kNone = -1,
    kPosition, ///< 'v'(xyz)
    kColor,  ///< extension: vertex colors
    kTextureCoordinates, ///< 'vt'(uv)
    kNormal, ///< 'vn'
    kTangent, ///< tangents for normal mapping
    kMiscInt,  ///< (Skeletal animation) used for bone weights for skeletal animation
    kMiscReal, ///< (Skeletal animation) used for IDs of the bones corresponding to the bone weights
    kIndices, ///< Used for the indices of the vertex attributes. Must be bound last when populating VAO
    kMAX_ATTRIBUTE_TYPE
};

/// @struct VertexAttributes
/// @brief Struct holding vertex attributes
/// @detail This is a template-template, fun!
/// @see https://stackoverflow.com/questions/68878706/passing-in-unspecialized-class-as-a-template-parameter
template<typename EnumType, template<typename> typename ContainerType, typename ...Types>
struct VertexAttributes {
private:

    /// @brief Tuple gets an extra type for indices
    typedef Uint32_t IndexType; ///< Type used to store index data
    typedef std::tuple<ContainerType<Types>..., ContainerType<IndexType>> TupleType;
    static constexpr size_t s_tupleSize = std::tuple_size<TupleType>::value; ///< Determine size of tuple

public:

    /// @brief Type data info
    struct TupleDataInfo {
        int m_sizeOfType{ 0 }; ///< The size of the most fundamental value type stored
        int m_valueCount{ 0 }; ///< The number of values stored in each stride of data, e.g. Vector2, Vector3, etc
        bool m_isIntegral{ false }; ///< If false, is floating point
    };

    typedef EnumType EnumType;
    typedef std::array<VertexAttributes::TupleDataInfo, VertexAttributes::s_tupleSize> TupleDataInfoArray;

    /// @brief Get metadata of the contained tuple types, including size and integral type
    /// @todo See if this can be made into a constexpr
    static const std::array<TupleDataInfo, s_tupleSize>& GetTupleData() {
        static std::array<TupleDataInfo, s_tupleSize> s_entryArray;
        static int s_count = 0;
        if (s_count) {
            return s_entryArray;
        }
        for_each_tuple_type<TupleType>(
            [&](auto& arg)
            {
                constexpr size_t sizeOfValueType = sizeof(std::decay_t<decltype(arg)>::value_type);
                if constexpr (std::is_arithmetic_v<std::decay_t<decltype(arg)>::value_type>) {
                    // Allow for use of simple arithmetic types
                    using MyFundamentalType = typename std::decay_t<decltype(arg)>::value_type;
                    constexpr size_t sizeOfFundamentalType = sizeof(MyFundamentalType);
                    s_entryArray[s_count] = {
                        sizeOfFundamentalType,
                        sizeOfValueType / sizeOfFundamentalType,
                        std::is_integral_v<MyFundamentalType>
                    };
                }
                else {
                    // Specifically handle a Vector type specialization
                    using MyFundamentalType = typename std::decay_t<decltype(arg)>::value_type::ValueType;
                    constexpr size_t sizeOfFundamentalType = sizeof(MyFundamentalType);
                    s_entryArray[s_count] = {
                        sizeOfFundamentalType,
                        sizeOfValueType / sizeOfFundamentalType,
                        std::is_integral_v<MyFundamentalType>
                    };
                }
                s_count++;
            });
        return s_entryArray;
    }

    /// @brief Return the attribute at the given index
    template<EnumType Index>
    typename std::tuple_element_t<static_cast<size_t>(Index), TupleType>& get() {
        return std::get<static_cast<size_t>(Index)>(m_data);
    }
    template<EnumType Index>
    typename std::add_const_t<std::tuple_element_t<static_cast<size_t>(Index), TupleType>>& get() const {
        return std::get<static_cast<size_t>(Index)>(m_data);
    }

    /// @brief Get the size of the tuple data
    Int64_t getSizeInBytes() const {
        Int64_t len = 0;
        for_each_tuple(m_data,
            [&](auto& arg)
            {
                len += sizeof(std::decay_t<decltype(arg)>::value_type) * arg.size();
            });
        return len;
    }

    /// @brief Get the size in bytes of the specified attribute
    template<EnumType Index>
    Int64_t getAttributeSizeInBytes() const {
        const auto& attributeBuffer = get<Index>();
        return sizeof(std::decay_t<decltype(attributeBuffer)>::value_type) * attributeBuffer.size();
    }

    /// @brief Whether or not the vertex attributes are empty
    bool empty() const {
        return get<MeshAttributes::kIndices>().empty();
    }

    /// @brief  Whether or not the 
    void clear() {
        for_each_tuple(m_data,
            [&](auto& arg)
            {
                arg.clear();
            });
    }

    TupleType m_data; ///< The vertex attribute data
};
typedef VertexAttributes<MeshVertexAttributeType, std::vector, Vector3, Vector4, Vector2, Vector3, Vector3, Vector4i, Vector4> MeshVertexAttributes;

// End namespaces
}
