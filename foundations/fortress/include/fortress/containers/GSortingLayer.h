#pragma once

// std
#include <functional>
#include <memory>

// Internal
#include "fortress/types/GLoadable.h"
#include "fortress/types/GNameable.h"

namespace rev {  

/// Forward Declarations
class Scene;

/// @class SortingLayer
/// @brief Used to govern the processing order of  data
class SortingLayer {
public:

    static constexpr char* s_defaultSortingLayer = "default";
	
    /// @name Constructors/Destructor
    /// @{

    SortingLayer();
    SortingLayer(int order);
    SortingLayer(const SortingLayer& layer);
    ~SortingLayer();

    /// @}

    /// @name Properties
    /// @{

    Uint32_t id() const { return m_id; }

    int getOrder() const {
        return m_order;
    }
    void setOrder(int order);

    /// @}

    /// @name Public methods
    /// @{

    /// @brief Return order distance above minimum order
    Uint32_t getPositiveOrder() const;

    bool isValid() const {
        return m_id != std::numeric_limits<Uint32_t>::max();
    }

    /// @}

    /// @name Operators
    /// @{

    SortingLayer& SortingLayer::operator=(const SortingLayer& other);

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

    /// @details A lower number will occur first
    Int32_t m_order = 0; ///< the order of this layer in the queue
    Uint32_t m_id = std::numeric_limits<Uint32_t>::max(); ///<  Uuid is unnecessary

    static Int32_t s_minimumOrder;
};


/// @class SortingLayers
class SortingLayers {
protected:

    using SortingLayerVector = std::vector<SortingLayer>;
public:

    SortingLayers();
    ~SortingLayers();

    void setLayerOrder(Int32_t layerId, Int32_t newOrder);

    const SortingLayer& getLayer(const GString& label) const;
    const SortingLayer& getLayerFromId(Uint32_t id) const;

    void clear();

    json getLayerJsonFromId(Uint32_t id) const;

    const std::vector<GString>& getNames() const { return m_names; }

    const GString& getLayerNameFromId(Uint32_t id) const;
    void setLayerNameFromId(Uint32_t id, const GString& name);

    const SortingLayer& addLayer();
    const SortingLayer& addLayer(const GString& name, int order);

    const SortingLayerVector::const_iterator begin() const {
        return m_layers.begin();
    }

    const SortingLayerVector::const_iterator end() const {
        return m_layers.end();
    }

    /// @brief Remove a render layer, calling the specified callback on removal
    bool removeLayer(const GString& label, std::function<void(Uint32_t)> onRemoval);
    bool removeLayer(Uint32_t layerId, std::function<void(Uint32_t)> onRemoval);

    /// @brief Sort the vectors of render layers and names to reflect any modifications
    void sort();

    /// @brief Convert from the given class type to JSON
    /// @note Actually defined in namespace outside of class, so this should be recognized by nlohmann JSON
    /// @param orJson The output JSON
    /// @param korObject The object to convert to JSON
    friend void to_json(nlohmann::json& orJson, const SortingLayers& korObject);

    /// @brief Convert from JSON to the given class type
    /// @param korJson The input JSON
    /// @param orObject The object to be obtained from JSON
    friend void from_json(const nlohmann::json& korJson, SortingLayers& orObject);

protected:
    /// @name Protected Methods
    /// @{

    SortingLayer& getLayerFromId(Uint32_t id);

    static bool SortRenderLayers(const SortingLayer& l1, const SortingLayer& l2) {
        return l1 < l2;
    }

    /// @}

    SortingLayerVector m_layers; ///< The sorting layers. Stored as pointers because location in memory is otherwise not preserved when the vector size changes
    std::vector<GString> m_names; ///< The names corresponding to each sorting layer

};


} // End rev namespace
