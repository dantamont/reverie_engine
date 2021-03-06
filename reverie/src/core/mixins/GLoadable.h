/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LOADABLE_H
#define GB_LOADABLE_H

// Standard
#include <vector>

// QT
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QJsonDocument>

// Internal
#include "../containers/GString.h"

namespace rev {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @struct SerializationContext
/// @brief Contains applicationinformation required to serialize an object
/// @note Exists for flexibility with dependency injection
struct SerializationContext {
    static const SerializationContext& Empty() { return s_emptyContext; }

    CoreEngine* m_engine = nullptr;

    /// @brief Generic data
    char* m_data = nullptr;
private:
    static SerializationContext s_emptyContext;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an object that can be serialized
class Serializable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Formats that data can be loaded from
    enum Format {
        kJson
    };

    static GString ToString(const QJsonObject& obj, QJsonDocument::JsonFormat fmt = QJsonDocument::Compact);
    static QByteArray ToByteData(const QJsonObject& obj, QJsonDocument::JsonFormat fmt = QJsonDocument::Compact);
    static QJsonObject ObjectFromString(const GString& str);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Serializable(){}
    virtual ~Serializable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const{
        Q_UNUSED(context);
        return QJsonValue();
    }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) {
        Q_UNUSED(json);
        Q_UNUSED(context);
        throw("Error, called unimplemented loadFromJson routine");
    }

    /// @}:

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an object that can be loaded and saved from a file
class Loadable: public Serializable{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @brief Formats that data can be loaded from
    enum Format {
        kJson,
        kBinary // Load directly into memory
    };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Loadable(const GString& filepath):
        Serializable(),
        m_path(filepath) {}
    Loadable() {}
    virtual ~Loadable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property File or directory path
    const GString& getPath() const { return m_path; }
    void setPath(const GString& path) { m_path = path; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

	/// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Filepath to the object
    GString m_path;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an object that can be loaded and saved from/to multiple files
class DistributedLoadable : public Loadable {
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DistributedLoadable(const GString& filepath): Loadable(filepath){
    }
    DistributedLoadable() {
    }
    virtual ~DistributedLoadable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::vector<GString>& additionalPaths() { return m_additionalPaths; }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override {
        QJsonObject object = Loadable::asJson().toObject();

        if (m_additionalPaths.size()) {
            QJsonArray paths;
            for (const GString& path : m_additionalPaths) {
                paths.push_back(path.c_str());
            }
            object.insert("additionalPaths", paths);
        }

        return object;
    }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{


    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Additional paths for the loadable
    std::vector<GString> m_additionalPaths;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif