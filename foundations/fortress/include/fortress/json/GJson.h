#pragma once
/// @see On friend functions https://github.com/nlohmann/json/issues/511

/// Includes
#include <fstream>

#include <nlohmann/json.hpp>
#include "fortress/types/GSizedTypes.h"

namespace rev
{

using json = nlohmann::json;

/// @struct SJson
/// @brief Struct containing commonly used json keys
struct JsonKeys {
    static constexpr char s_filepath[] = "filePath"; ///< @note Requires C++17 and up to lack definition
    static constexpr char s_name[] = "name"; ///< @note Requires C++17 and up to lack definition
};

/// @brief Call to_json for the specified class
template<typename Casted, typename Uncasted>
void ToJson(json& orJson, const Uncasted& korObject) {
    static_assert(!std::is_pointer_v<Uncasted>, "Pointer types are invalid");
    to_json(orJson, static_cast<const Casted&>(korObject));
}

/// @brief Call from_json for the specified class
template<typename Casted, typename Uncasted>
void FromJson(const json& korJson, Uncasted& orObject) {
    static_assert(!std::is_pointer_v<Uncasted>, "Pointer types are invalid");
    from_json(korJson, static_cast<Casted&>(orObject));
}

/// @class GJson
/// @brief Class for wrapping convenience functions
class GJson {
public:

    /// @brief Convert an object to JSON bytes
    template<typename T>
    static std::vector<Uint8_t> ToBytes(const T& object) {
        static_assert(!std::is_pointer_v<T>, "Pointer types are invalid");
        json j;
        to_json(j, object);
        return json::to_cbor(j);
    }

    template<>
    static std::vector<Uint8_t> ToBytes(const json& object) {
        return json::to_cbor(object);
    }

    /// @brief Get Json from JSON bytes
    static json FromBytes(const std::vector<Uint8_t>& bytes) {
        if (0 == bytes.size()) {
            return json::object();
        }
        else {
            json j = json::from_cbor(bytes);
            return j;
        }
    }

    /// @brief Convert an object from JSON bytes
    template<typename T>
    static void FromBytes(const std::vector<Uint8_t>& bytes, T& object) {
        static_assert(!std::is_pointer_v<T>, "Pointer types are invalid");
        json j = FromBytes(bytes);
        from_json(j, object);
    }

    /// @brief Convert Json to a string
    /// @see https://stackoverflow.com/questions/47834320/prettify-a-json-string-in-c-from-a-txt-file
    template<typename StringType>
    static StringType ToString(const nlohmann::json& json, bool prettify = false) {
        /// Dump Json to string, with indents if prettified
        StringType jsonStr(json.dump(prettify ? 3 : -1).c_str());
        return jsonStr;
    }

    /// @brief Return the JSON contents of a file, given the absolute file path
    static void FromFile(const char* filePath, json& outJson);
};


} // End namespace