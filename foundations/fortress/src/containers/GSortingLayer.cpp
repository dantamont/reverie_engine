#include "fortress/containers/GSortingLayer.h"

#include <atomic>

#include "fortress/containers/GContainerExtensions.h"
#include "fortress/numeric/GSizedTypes.h"
#include "fortress/json/GJson.h"
#include "fortress/string/GString.h"

namespace rev {


// Class Definitions

SortingLayer::SortingLayer()
{
    initialize();
    setOrder(m_order);
}

SortingLayer::SortingLayer(int order) {
    initialize();
    setOrder(order);
}

SortingLayer::SortingLayer(const SortingLayer& layer):
    m_id(layer.m_id),
    m_order(layer.m_order)
{

}

SortingLayer::~SortingLayer()
{
}

SortingLayer& SortingLayer::operator=(const SortingLayer& other)
{
    m_id = other.m_id;
    m_order = other.m_order;
    return *this;
}

void SortingLayer::setOrder(int order)
{
    m_order = order;
    s_minimumOrder = std::min(order, s_minimumOrder);
}

Uint32_t SortingLayer::getPositiveOrder() const
{
    int order = m_order - s_minimumOrder;
    return Uint32_t(order);
}

void SortingLayer::initialize()
{
    // Set unique ID for the sorting layer
    static Uint32_t layerCount = 0;
    m_id = layerCount++;
}

Int32_t SortingLayer::s_minimumOrder = std::numeric_limits<Int32_t>::max();




SortingLayers::SortingLayers()
{
}

SortingLayers::~SortingLayers()
{
}

void SortingLayers::clear()
{
    m_layers.clear();
}

void SortingLayers::setLayerOrder(Int32_t layerId, Int32_t newOrder) 
{
    SortingLayer& layer = getLayerFromId(layerId);
    layer.setOrder(newOrder);
    sort();
}

const SortingLayer& SortingLayers::getLayer(const GString& label) const {
    auto iter = std::find_if(m_names.begin(), m_names.end(),
        [label](const GString& name) { return name == label; });
    assert(iter != m_names.end() && "Sorting layer not found");
    return m_layers[iter - m_names.begin()];
}

const SortingLayer& SortingLayers::getLayerFromId(Uint32_t id) const {
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [id](const auto& layer) {return layer.id() == id; });
    if (iter == m_layers.end()) {
        assert(false && "Sorting layer not found");
    }
    return *iter;
}

SortingLayer& SortingLayers::getLayerFromId(Uint32_t id)
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [id](const auto& layer) {return layer.id() == id; });
    assert(iter != m_layers.end() && "Sorting layer not found");
    return *iter;
}

json SortingLayers::getLayerJsonFromId(Uint32_t id) const
{
    const SortingLayer& layer = getLayerFromId(id);
    json layerJson = { 
        {"order", layer.getOrder()},
        {"label", getLayerNameFromId(id)},
        {"id", layer.id()} ///< @todo Remove eventually, used only for widgets
    };
    return layerJson;
}

const GString& SortingLayers::getLayerNameFromId(Uint32_t id) const
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [id](const auto& layer) {return layer.id() == id; });
#ifdef DEBUG_MODE
    if (iter == m_layers.end()) {
        assert(false && "Layer with ID not found");
    }
#endif

    return m_names[iter - m_layers.begin()];
}

void SortingLayers::setLayerNameFromId(Uint32_t id, const GString& name)
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [id](const auto& layer) {return layer.id() == id; });
#ifdef DEBUG_MODE
    if (iter == m_layers.end()) {
        assert(false && "Layer with ID not found");
    }
#endif

    Uint32_t index = iter - m_layers.begin();
    m_names[index] = name;
}

bool SortingLayers::removeLayer(const GString & label, std::function<void(Uint32_t)> onRemoval)
{
    auto iter = std::find_if(m_names.begin(), m_names.end(),
        [&](const GString& name) {
        return name == label;
    });

#ifdef DEBUG_MODE
    if (iter == m_names.end()) {
        throw("Error, layer not found");
    }
#endif

    // Remove the layer and name from the internal vectors
    Uint32_t index = iter - m_names.begin();
    Uint32_t layerId = m_layers[index].id();
    SortingLayer& layer = m_layers[index];

    m_layers.erase(m_layers.begin() + index);
    m_names.erase(iter);

    // Invoke callback
    onRemoval(layerId);

    return true;
}

void SortingLayers::sort() {
    Vec::SortVectors(m_layers, SortRenderLayers, m_layers, m_names);
}

const SortingLayer& SortingLayers::addLayer() {
    static std::atomic<Uint32_t> nameCount = 0;
    GString uniqueName = GString::Format("layer_%d", nameCount.load()).c_str();
    nameCount.fetch_add(1);
    m_layers.emplace_back();
    m_names.push_back(uniqueName);
    sort();
    return m_layers.back();
}

const SortingLayer& SortingLayers::addLayer(const GString& name, int order) {
    m_layers.emplace_back(order);
    m_names.push_back(name);
    sort();
    return m_layers.back();
}


bool SortingLayers::removeLayer(Uint32_t layerId, std::function<void(Uint32_t)> onRemoval)
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [&](const SortingLayer& layer) {
        return layer.id() == layerId;
    });

#ifdef DEBUG_MODE
    if (iter == m_layers.end()) {
        throw("Error, layer not found");
    }
#endif

    Uint32_t index = iter - m_layers.begin();
    m_layers.erase(iter);
    m_names.erase(m_names.begin() + index);

    // Invoke callback
    onRemoval(layerId);

    return true;
}

void to_json(json& orJson, const SortingLayers& korObject)
{
    orJson = json::array();
    for (Uint32_t i = 0; i < korObject.m_layers.size(); i++) {
        const SortingLayer& layer = korObject.m_layers[i];
        const GString& name = korObject.m_names[i];
        if (name != SortingLayer::s_defaultSortingLayer) {
            // Don't save default layer
            json layerJson;
            layerJson["order"] = layer.getOrder();
            layerJson["label"] = name.c_str();
            layerJson["id"] = layer.id(); ///< @todo Eventually remove, used only for widgets
            orJson.push_back(layerJson);
        }
    }
}

void from_json(const json& korJson, SortingLayers& orObject)
{
    for (const auto& layerJson : korJson) {
        Int32_t order = layerJson.at("order").get<Int32_t>();
        orObject.m_layers.emplace_back(order);
        orObject.m_names.emplace_back(layerJson.at("label").get_ref<const std::string&>().c_str());
    }
}



} // End rev namespace
