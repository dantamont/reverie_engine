/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_LOADABLE_H
#define GB_LOADABLE_H

// QT
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

// Internal

namespace Gb {

//////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////
class CoreEngine;

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
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
    Serializable():
        m_isPythonGenerated(false)
    {}
    virtual ~Serializable() {}
    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    bool isPythonGenerated() { return m_isPythonGenerated; }
    void setIsPythonGenerated(bool gen) { m_isPythonGenerated = gen; }


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

    /// @}
protected:
    //--------------------------------------------------------------------------------------------
    /// @name Private Methods
    /// @{
    /// @}

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
} // End namespaces

#endif