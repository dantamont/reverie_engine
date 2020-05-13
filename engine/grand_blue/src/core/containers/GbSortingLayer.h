/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SORTING_LAYER_H
#define GB_SORTING_LAYER_H

// QT
#include <QString>

// Internal
#include "../mixins/GbLoadable.h"
#include "../GbObject.h"

namespace Gb {  
//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SortingLayer
/// @brief Used to govern the processing order of  data
struct SortingLayer: public Serializable, public Object {
	
    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{
    SortingLayer() {
    }
    SortingLayer(const QJsonValue& json);
    SortingLayer(const QString& name, int order):
        m_order(order),
        m_label(name){
    }
    ~SortingLayer(){}

    /// @brief the order of this layer in the queue
    /// @details A lower number will occur first
	int m_order = 0;
    QString m_label = "default";

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public Methods
    /// @{

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "SortingLayer"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "Gb::SortingLayer"; }
    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson() const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json) override;

    /// @}

    //--------------------------------------------------------------------------------------------
    /// @name Operators
    /// @{

    inline bool SortingLayer::operator==(const SortingLayer &b) const {
        return m_order == b.m_order;
    }
    inline bool SortingLayer::operator!=(const SortingLayer &b) const {
        return m_order != b.m_order;
    }
    inline bool SortingLayer::operator<(const SortingLayer &b) const {
        return m_order < b.m_order;
    }
    inline bool SortingLayer::operator>(const SortingLayer &b) const {
        return m_order > b.m_order;
    }
    inline bool SortingLayer::operator>=(const SortingLayer &b) const {
        return m_order >= b.m_order;
    }
    inline bool SortingLayer::operator<=(const SortingLayer &b) const {
        return m_order <= b.m_order;
    }

    /// @}
};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
} 

#endif