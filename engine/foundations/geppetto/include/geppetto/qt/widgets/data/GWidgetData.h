#pragma once

#include "fortress/json/GJson.h"

namespace rev {

/// @brief Represents data contained by a widget
struct WidgetData {
    nlohmann::json m_json; ///< The JSON data
};

/// @brief Represents itemized data contained by a widget
struct ItemizedWidgetData {
    ItemizedWidgetData(Uint32_t id) :
        m_id(id)
    {
    }

    ItemizedWidgetData(Uint32_t id, const WidgetData& data) :
        m_id(id),
        m_data(data)
    {
    }

    ItemizedWidgetData(Uint32_t id, const json& data) :
        m_id(id)
    {
        m_data.m_json = data;
    }

    /// @brief Return a data field by name
    /// @tparam the data type
    /// @name the field name
    template<typename T>
    T get(const char* name) const {
        return m_data.m_json[name].get<T>();
    }

    /// @brief Return a data field by name
    /// @tparam the data type
    /// @name the field name
    template<typename T>
    const T& getRef(const char* name) const {
        return m_data.m_json[name].get_ref<const T&>();
    }
    template<typename T>
    T& getRef(const char* name) {
        return m_data.m_json[name].get_ref<T&>();
    }

    /// @brief Set a data field by name
    /// @tparam the data type
    /// @name the field name
    template<typename T>
    void set(const char* name, const T& val) {
        m_data.m_json[name] = val;
    }

    Uint32_t m_id{ 0 }; ///< The unique ID of the tree item
    WidgetData m_data; ///< The JSON data
};

} /// End namespace rev
