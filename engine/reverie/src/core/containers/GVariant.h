/**
    @file GbVariant.h
    @note See: https://gist.github.com/tibordp/6909880
*/

#ifndef GB_VARIANT_H
#define GB_VARIANT_H

/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////
// Standard
#include <variant>

#include <typeindex>
#include <typeinfo>
#include <optional>

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
template <typename... Types>
class Variant
{
public:
    typedef std::variant<std::monostate, Types...> V;

    constexpr std::size_t typeIndex() const {
        return m_variant.index();
    }

    //const std::type_info& typeInfo(std::optional<std::size_t> idx = {}) const {
    //    if (!idx) {
    //        idx = m_variant.index();
    //    }
    //    if (*idx == std::variant_npos) {
    //        return typeid(void);
    //    }
    //    const std::array<std::type_info const*, sizeof...(Types) + 1> infos[] = { &typeid(std::monostate), &typeid(Types)... };
    //    return *(infos[*idx]);
    //}

    std::type_info const& typeInfo() const {
        return std::visit([](auto&& x)->decltype(auto) { return typeid(x); }, m_variant);
    }

    template<typename T>
    bool is() const {
        //return std::get_if<T>(&m_variant) == nullptr;
        return std::holds_alternative<T>(m_variant);
    }

    bool isEmpty() const {
        return is<std::monostate>();
    }

    template<typename T>
    constexpr T& get() {
        return std::get<T>(m_variant);
    }

    template<typename T>
    constexpr const T& get() const {
        return std::get<T>(m_variant);
    }

    template<typename T>
    void set() {
        m_variant = T();
    }

    template<typename T>
    void set(const T& val) {
        m_variant = val;
    }

    template<typename T>
    void set(T&& val) {
        m_variant = std::move(val);
    }

private:
    /// @brief Create internal variant with a monostate to allow to be empty
    V m_variant = std::monostate{};
};

/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespace

#endif