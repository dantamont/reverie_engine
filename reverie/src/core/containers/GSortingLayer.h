/////////////////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GB_SORTING_LAYER_H
#define GB_SORTING_LAYER_H

// QT
#include <QString>
#include <functional>
#include <memory>

// Internal
#include "../mixins/GLoadable.h"
#include "../GObject.h"

namespace rev {  
//////////////////////////////////////////////////////////////////////////////////////////////////
// Defs
//////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_SORTING_LAYER "default"

//////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////
class Scene;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @class SortingLayer
/// @brief Used to govern the processing order of  data
struct SortingLayer: public Serializable, public Object, public Nameable{
	
    //--------------------------------------------------------------------------------------------
    /// @name Members
    /// @{
    SortingLayer();
    SortingLayer(const QJsonValue& json);
    SortingLayer(const GString& name, int order);
    ~SortingLayer();

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Properties
    /// @{

    size_t id() const { return m_id; }

    int getOrder() const {
        return m_order;
    }
    void setOrder(int order);

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name Public methods
    /// @{

    /// @brief Return order distance above minimum order
    size_t getPositiveOrder() const;

    /// @}

    //---------------------------------------------------------------------------------------
    /// @name GB Object Properties 
    /// @{

    /// @property className
    virtual const char* className() const { return "SortingLayer"; }

    /// @property namespaceName
    virtual const char* namespaceName() const { return "rev::SortingLayer"; }
    /// @}


    //-----------------------------------------------------------------------------------------------------------------
    /// @name Serializable overrides
    /// @{

    /// @brief Outputs this data as a valid json string
    virtual QJsonValue asJson(const SerializationContext& context = SerializationContext::Empty()) const override;

    /// @brief Populates this data using a valid json string
    virtual void loadFromJson(const QJsonValue& json, const SerializationContext& context = SerializationContext::Empty()) override;

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

private:

    void initialize();

    /// @brief the order of this layer in the queue
    /// @details A lower number will occur first
    int m_order = 0;

    /// @brief Uuid is unnecessary
    size_t m_id;

    static int s_minimumOrder;

    static std::vector<size_t> s_deletedIndices;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
/// @struct SortingLayers
struct SortingLayers {

    SortingLayers();
    ~SortingLayers();

    SortingLayer* getLayer(const GString& label) const {
        auto iter = std::find_if(m_layers.begin(), m_layers.end(),
            [label](const auto& layer) { return layer->getName() == label; });
        if (iter != m_layers.end()) {
            return (*iter).get();
        }
        else {
            return nullptr;
        }
    }

    SortingLayer* getLayerFromId(size_t id) const {
        auto iter = std::find_if(m_layers.begin(), m_layers.end(),
            [id](const auto& layer) {return layer->id() == id; });
        if (iter == m_layers.end()) {
            return nullptr;
        }
        else {
            return (*iter).get();
        }
    }

    SortingLayer* addLayer() {
        static std::atomic<size_t> nameCount = 0;
        GString uniqueName = GString::Format("layer_%d", nameCount.load()).c_str();
        nameCount.fetch_add(1);
        m_layers.push_back(std::make_unique<SortingLayer>());
        m_layers.back()->setName(uniqueName);
        sort();
        return m_layers.back().get();
    }
    SortingLayer* addLayer(const GString& name, int order) {
        m_layers.push_back(std::make_unique<SortingLayer>(name, order));
        sort();
        return m_layers.back().get();
    }

    /// @brief Remove a render layer, calling the specified callback on removal
    bool removeLayer(const GString& label, std::function<void(size_t)> onRemoval);
    bool removeLayer(size_t layerId, std::function<void(size_t)> onRemoval);

    /// @brief Sort the vector of render layers to reflect any modifications
    void sort() {
        std::sort(m_layers.begin(), m_layers.end(), SortRenderLayers);
    }



    std::vector<std::unique_ptr<SortingLayer>> m_layers;

protected:
    //--------------------------------------------------------------------------------------------
    /// @name Protected Methods
    /// @{

    static bool SortRenderLayers(const std::unique_ptr<SortingLayer>& l1,
        const std::unique_ptr<SortingLayer>& l2) {
        return *l1 < *l2;
    }

    /// @}

};

//////////////////////////////////////////////////////////////////////////////////////////////////    
// End namespaces
} 

#endif