#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include "fortress/templates/GTemplates.h"
#include "fortress/containers/math/GVector.h"

namespace rev {

TEST(TemplateTests, UniquePointerCheck) {
    static_assert(is_unique_ptr<std::unique_ptr<int>>(), "Should be a pointer");
    static_assert(!is_unique_ptr<int>(), "Better not be a pointer");
}

enum class TestEnum {
    eVal0 = 0,
    eVal1,
    eVal2,
    eVal3,
    eVal4,
    eVal5
};


/// @note Cannot use template-templates with functions, so need to wrap in a struct
static int s_count = 0;

template<TestEnum EnumValue>
void incrementCount() {
    s_count += (int)EnumValue;
}

template<TestEnum EnumValue>
struct IncrementCountWrapper {
    void operator()(void) const { 
        incrementCount<EnumValue>(); 
    }
};

/// Test template-template with an argument
template<TestEnum EnumValue>
void incrementCountMultiple(int factor) {
    s_count += (int)EnumValue * factor;
}

template<TestEnum EnumValue>
struct IncrementCountMultipleWrapper {
    void operator()(int factor) const {
        incrementCountMultiple<EnumValue>(factor);
    }
};



TEST(TemplateTests, ConstexprEnumForLoop) {
    IncrementCountWrapper<TestEnum::eVal1>()();
    EXPECT_EQ(s_count, 1);

    s_count = 0;
    for_each_enums<TestEnum::eVal0, TestEnum::eVal4, IncrementCountWrapper>();
    EXPECT_EQ(s_count, 6);

    s_count = 0;
    for_each_enums<TestEnum::eVal0, TestEnum::eVal5, IncrementCountWrapper>();
    EXPECT_EQ(s_count, 10);

    s_count = 0;
    for_each_enums<TestEnum::eVal0, TestEnum::eVal5, IncrementCountMultipleWrapper>(30);
    EXPECT_EQ(s_count, 300);
}

/// @brief Type data info
struct TupleDataInfo {
    int m_sizeOfType{ 0 };
    int m_valueCount{ 0 };
    bool m_isIntegral; ///< If false, is floating point
};

template<typename EnumType, template<typename> typename ContainerType, typename ...Types>
struct TupleData {
private:

    /// @brief Tuple gets an extra type for indices
    using Tuple = std::tuple<ContainerType<Types>...>;
public:

    static constexpr size_t s_tupleSize = std::tuple_size<Tuple>::value; ///< Determine size of tuple

    /// @brief Get the size of the tuple data
    static const std::array<TupleDataInfo, s_tupleSize>& GetTupleData() {
        static std::array<TupleDataInfo, s_tupleSize> s_entryArray;
        static int s_count = 0;
        if (s_count) {
            return s_entryArray;
        }
        for_each_tuple_type<Tuple>(
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
    template<TestEnum Index>
    typename std::tuple_element_t<static_cast<size_t>(Index), Tuple>& get() {
        return std::get<static_cast<size_t>(Index)>(m_data);
    }
    template<TestEnum Index>
    typename std::add_const_t<std::tuple_element_t<static_cast<size_t>(Index), Tuple>>& get() const {
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

    template<EnumType Index>
    Int64_t getAttributeSizeInBytes() const {
        const auto& attributeBuffer = get<Index>();
        return sizeof(std::decay_t<decltype(attributeBuffer)>::value_type) * attributeBuffer.size();
    }

    /// @brief  Whether or not the 
    void clear() {
        for_each_tuple(m_data,
            [&](auto& arg)
            {
                arg.clear();
            });
    }

    Tuple m_data; ///< The tuple data
};

typedef TupleData<TestEnum, std::vector, Vector<int, 1>> TestTupleType;
typedef TupleData<TestEnum, std::vector, Vector<int, 1>, Vector<float, 4>> AnotherTupleType;
typedef TupleData<TestEnum, std::vector, Vector<int, 1>, float> YetAnotherTupleType;
TEST(TemplateTests, ConstexprEnumForTuple) {
    TestTupleType tupleData;

    Int64_t sizeInBytes = tupleData.getSizeInBytes();
    EXPECT_EQ(sizeInBytes, 0);

    sizeInBytes = tupleData.getAttributeSizeInBytes<TestEnum::eVal0>();
    EXPECT_EQ(sizeInBytes, 0);

    tupleData.get<TestEnum::eVal0>().resize(1);
    tupleData.get<TestEnum::eVal0>()[0] = 7;

    sizeInBytes = tupleData.getAttributeSizeInBytes<TestEnum::eVal0>();
    EXPECT_EQ(sizeInBytes, 4);

    int entry = 0;
    for_each_tuple(
        tupleData.m_data,
        [&](auto& arg) 
        {
            entry = (int)arg[0];
        });

    EXPECT_EQ(entry, 7);

    auto f = [&](auto& value) {
        entry++;
    };

    for_each_tuple(tupleData.m_data, f);
    EXPECT_EQ(entry, 8);
    
    tupleData.get<TestEnum::eVal0>().push_back(100);
    sizeInBytes = tupleData.getSizeInBytes();
    EXPECT_EQ(sizeInBytes, 2 * sizeof(int));

    tupleData.clear();
    sizeInBytes = tupleData.getSizeInBytes();
    EXPECT_EQ(sizeInBytes, 0);

    const std::array<TupleDataInfo, TestTupleType::s_tupleSize>& data = TestTupleType::GetTupleData();
    EXPECT_EQ(data[0].m_sizeOfType, 4);
    EXPECT_EQ(data[0].m_valueCount, 1);
    EXPECT_EQ(data[0].m_isIntegral, true);

    AnotherTupleType tuple2;
    const std::array<TupleDataInfo, AnotherTupleType::s_tupleSize>& data2 = AnotherTupleType::GetTupleData();
    EXPECT_EQ(data2[0].m_sizeOfType, 4);
    EXPECT_EQ(data2[0].m_valueCount, 1);
    EXPECT_EQ(data2[0].m_isIntegral, true);
    EXPECT_EQ(data2[1].m_sizeOfType, 4);
    EXPECT_EQ(data2[1].m_valueCount, 4);
    EXPECT_EQ(data2[1].m_isIntegral, false);

    YetAnotherTupleType tuple3;
    const std::array<TupleDataInfo, YetAnotherTupleType::s_tupleSize>& data3 = YetAnotherTupleType::GetTupleData();
    EXPECT_EQ(data3[0].m_sizeOfType, 4);
    EXPECT_EQ(data3[0].m_valueCount, 1);
    EXPECT_EQ(data3[0].m_isIntegral, true);
    EXPECT_EQ(data3[1].m_sizeOfType, 4);
    EXPECT_EQ(data3[1].m_valueCount, 1);
    EXPECT_EQ(data3[1].m_isIntegral, false);
}

typedef TupleData<TestEnum, std::vector, Vector<int, 1>, Vector<float, 1>> MultiFunctionTupleType;
TEST(TemplateTests, ConstexprEnumForTupleMultipleFunctions) {
    MultiFunctionTupleType tupleData;
    tupleData.get<TestEnum::eVal0>().resize(1);
    tupleData.get<TestEnum::eVal0>()[0] = 7;
    tupleData.get<TestEnum::eVal1>().resize(1);
    tupleData.get<TestEnum::eVal1>()[0] = (float)3.14;

    int entry = 0;
    float entry2 = 0;
    for_each_tuple(
        tupleData.m_data,
        [&](auto& arg)
        {
            entry = (int)arg[0];
        },
        [&](auto& arg)
        {
            entry2 = 2.0 * arg[0][0]; // Vector<type, 1> has no multiplication
        });

    EXPECT_EQ(entry, 7);
    EXPECT_NEAR(entry2, 6.28, 1e-6);
}

} /// End rev namespace
