#pragma once

// Public
#include <memory>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>

#include "fortress/json/GJson.h"
#include "fortress/templates/GTemplates.h"
#include "fortress/types/GLoadable.h"
#include "fortress/layer/framework/GSingleton.h"

// Qt
#include <QSurfaceFormat>

namespace rev { 

class GString;

/// @brief Valid rendering modes for graphics backend
enum class RenderingBackend {
    kGL_ES,
    kGL,
    kVulkan //TODO: Implement
};

/// @class GApplicationSettings
/// @brief Provides an abstraction layer over a JSON settings file
/// @todo Split out render settings into a separate class
/// @todo Create a templated singleton base class
class GApplicationSettings: public LoadableInterface, public SingletonInterface<GApplicationSettings>{
public:

    /// @name Destructor
    /// @{
	~GApplicationSettings();

    /// @}

    /// @name Public Methods
    /// @{
    /// @}

    /// @brief Apply a default set of settings for Open GL ES 3.1.0 configuration to a format
    QSurfaceFormat configOpenGLES();

	/// @brief Apply a default set of settings for Open GL ES 3.1.0 configuration to a format, using ANGLE
	QSurfaceFormat configOpenGLESAngle();

    /// @brief Apply a default set of settings for Open GL 4.3.0 configuration to a format
    QSurfaceFormat configOpenGL();

    /// @brief Set rendering mode
    void setRenderingBackend(RenderingBackend mode, int major, int minor);

    /// @brief Get a setting value
    template<typename ValueType>
    ValueType value(const std::vector<const char*>& keys) const {
        std::shared_lock lock(s_mutex);

        // Iterate through keys to obtain JSON value
        json jsonValue = s_settingsJson;
        for (const char* key : keys) {
            jsonValue = jsonValue.at(key);
        }
        return jsonValue.get<ValueType>();
    }

    /// @brief Get a setting value, returning a default if not found
    template<typename ValueType>
    ValueType value(const std::vector<const char*>& keys, const ValueType& defaultVal) const {
        std::shared_lock lock(s_mutex);

        // Iterate through keys to obtain JSON value
        json jsonValue = s_settingsJson;
        for (const char* key : keys) {
            if (!jsonValue.contains(key)) {
                return defaultVal;
            }
            jsonValue = jsonValue.at(key);
        }
        return jsonValue.get<ValueType>();
    }

    /// @brief Set a setting value, both within the application and in the settings file
    template<typename ValueType>
    void setValue(const std::vector<const char*>& keys, const ValueType& value) {
        std::unique_lock lock(s_mutex);

        // Iterate through keys to set JSON value
        json* jsonValue = &s_settingsJson;
        Int32_t allButLast = keys.size() - 1;
        for (Int32_t i = 0; i < allButLast; i++) {
            const char* key = keys[i];

            // Create object at key if it does not exist
            if (!jsonValue->contains(key)) {
                (*jsonValue)[key] = json::object();
            }
            jsonValue = &jsonValue->at(key);
        }
        assert(jsonValue->is_object() && "JSON must be an object to insert key");
        (*jsonValue)[keys.back()] = value;
        write();
    }

    /// @}

    /// @name Properties
    /// @{

    /// @brief The application working (root) directory
    GString getWorkingDirectory() const;

    /// @brief Most recent project
    GString getRecentProject();
    void setRecentProject(const GString& filepath);

    /// @brief Get the application gateway port
    Uint32_t applicationGatewayReceivePort() const;

    /// @brief Get the widget gateway port
    Uint32_t widgetGatewayReceivePort() const;

    /// @brief Get major and minor version of the current rendering backend
    /// @details Currently, GL is the only supported backend
    int getMajorVersion();
    int getMinorVersion();

    /// @brief get rendering mode from settings
    RenderingBackend getRenderingBackend();

    /// @}

protected:

    GApplicationSettings();

private:

    /// @brief Write Json settings to the file
    void write();

    static json s_settingsJson; ///< Json containing application settings

    static std::shared_mutex s_mutex; ///< Mutex so that JSON/file access to be thread-safe

};



} /// End rev namespace
