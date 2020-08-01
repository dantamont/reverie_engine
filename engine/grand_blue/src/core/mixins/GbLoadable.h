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

// Internal

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class representing an object that can be generated via a script
class Persistable {
public:

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    Persistable() : 
        m_isPythonGenerated(false)
    {
    }
    virtual ~Persistable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    bool isPythonGenerated() { return m_isPythonGenerated; }
    void setIsPythonGenerated(bool gen) { m_isPythonGenerated = gen; }


    /// @}

protected:

    //--------------------------------------------------------------------------------------------
    /// @name Private Members
    /// @{

    /// @brief Whether or not this object was python generated
    /// @details Python-generated objects will not be saved as json, since they are
    /// auto-generated on script reruns
    bool m_isPythonGenerated;

    /// @}

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
    virtual QJsonValue asJson() const{
        return QJsonValue();
    }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) {
        Q_UNUSED(json);
        throw("Error, called unimplemented loadFromJson routine");
    }

    /// @brief Populates this data using a valid json string
    /// @details This option is useful for classes that don't otherwise need to store a pointer
    /// to the core engine, but need it for serialization
    virtual void loadFromJson(CoreEngine* engine, const QJsonValue& json) {
        Q_UNUSED(engine);
        Q_UNUSED(json);
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
        kJson
    };

    /// @}

	//--------------------------------------------------------------------------------------------
	/// @name Constructors/Destructor
	/// @{
    Loadable(const QString& filepath):
        Serializable(),
        m_path(filepath) {}
    Loadable() {}
    virtual ~Loadable() {}
	/// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    /// @property File or directory path
    const QString& getPath() const { return m_path; }
    void setPath(const QString& path) { m_path = path; }

    /// @}
	//--------------------------------------------------------------------------------------------
	/// @name Public Methods
	/// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override{
        QJsonObject jsonObject;
        jsonObject.insert("filePath", m_path);
        return jsonObject;
    }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override{
        m_path = json.toObject().value("filePath").toString();
    }

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
    QString m_path;

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

    /// @brief Formats that data can be loaded from
    enum Format {
        kJson
    };

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructors/Destructor
    /// @{
    DistributedLoadable(const QString& filepath): Loadable(filepath){
    }
    DistributedLoadable() {
    }
    virtual ~DistributedLoadable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    std::vector<QString>& additionalPaths() { return m_additionalPaths; }

    /// @}
    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override {
        QJsonObject object = Loadable::asJson().toObject();

        if (m_additionalPaths.size()) {
            QJsonArray paths;
            for (const QString& path : m_additionalPaths) {
                paths.push_back(path);
            }
            object.insert("additionalPaths", paths);
        }

        return object;
    }

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override {
        Loadable::loadFromJson(json);
        QJsonObject object = json.toObject();

        if (object.contains("additionalPaths")) {
            QJsonArray paths = object["additionalPaths"].toArray();
            for (const auto& pathJson : paths) {
                m_additionalPaths.push_back(pathJson.toString());
            }
        }
    }

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
    std::vector<QString> m_additionalPaths;

    /// @}

};


/////////////////////////////////////////////////////////////////////////////////////////////
} // End namespaces

#endif