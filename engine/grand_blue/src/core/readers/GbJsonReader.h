/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_JSON_READER
#define GB_JSON_READER

// QT
#include <QVariantMap>
#include <QJsonObject>

// Internal
#include "GbFileReader.h"

namespace Gb { 
/////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
/////////////////////////////////////////////////////////////////////////////////////////////

/// @class JsonReader
/// @brief For loading in data from a JSON file
/// @detailed Stores positional data of a model in a VAO
// TODO: parallelize 
// https://stackoverflow.com/questions/34572043/how-can-i-asynchronously-load-data-from-large-files-in-qt
class JsonReader : public FileReader{
public:
    //--------------------------------------------------------------------------------------------
    /// @name Static
    /// @{   

    static QString getJsonValueAsQString(const QJsonValue& json, bool verbose = false);
    static QJsonDocument getQStringAsJsonDocument(const QString& str);

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Constructor/Destructor
    /// @{    

    JsonReader(const QString& filepath);
    ~JsonReader();

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{    

    /// @brief Return file contents as a variant map
    QVariantMap getContentsAsVariantMap();

    /// @brief Return file contents as a variant map
    QJsonObject getContentsAsJsonObject();

    /// @}
protected:

    //--------------------------------------------------------------------------------------------
    /// @name Protected Members
    /// @{    


    /// @}
};

        
/////////////////////////////////////////////////////////////////////////////////////////////////////////////      
} // End namespaces

#endif