#include "GSortingLayer.h"

namespace rev {

/////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////////////////////
SortingLayer::SortingLayer() :
    Nameable(DEFAULT_SORTING_LAYER)
{
    initialize();
    setOrder(m_order);
}

SortingLayer::SortingLayer(const QJsonValue & json)
{
    initialize();
    loadFromJson(json);
}

SortingLayer::SortingLayer(const GString & name, int order) :
    Nameable(name) {
    initialize();
    setOrder(order);
}
SortingLayer::~SortingLayer()
{
    s_deletedIndices.push_back(m_id);
}

void SortingLayer::setOrder(int order)
{
    m_order = order;
    s_minimumOrder = std::min(order, s_minimumOrder);
}

size_t SortingLayer::getPositiveOrder() const
{
    int order = m_order - s_minimumOrder;
    return size_t(order);
}

QJsonValue SortingLayer::asJson(const SerializationContext& context) const
{
    QJsonObject object;
    object.insert("order", m_order);
    object.insert("label", m_name.c_str());
    return object;
}

void SortingLayer::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context);

    const QJsonObject& object = json.toObject();
    setOrder(object.value("order").toInt());
    m_name = object.value("label").toString();
}

void SortingLayer::initialize()
{
    // Set unique ID for the sorting layer
    static size_t layerCount = 0;
    if (s_deletedIndices.size()) {
        m_id = s_deletedIndices.back();
        s_deletedIndices.pop_back();
    }
    else{
        m_id = layerCount++;
    }
}

int SortingLayer::s_minimumOrder = std::numeric_limits<int>::max();


std::vector<size_t> SortingLayer::s_deletedIndices{};




/////////////////////////////////////////////////////////////////////////////////////////////
// SortingLayers
/////////////////////////////////////////////////////////////////////////////////////////////
SortingLayers::SortingLayers()
{
}

SortingLayers::~SortingLayers()
{
}

bool SortingLayers::removeLayer(const GString & label, std::function<void(size_t)> onRemoval)
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [&](const std::unique_ptr<SortingLayer>& layer) {
        return layer->getName() == label;
    });

#ifdef DEBUG_MODE
    if (iter == m_layers.end()) {
        throw("Error, layer not found");
    }
#endif

    size_t layerId = (*iter)->id();
    m_layers.erase(iter);

    // Invoke callback
    onRemoval(layerId);

    return true;
}

bool SortingLayers::removeLayer(size_t layerId, std::function<void(size_t)> onRemoval)
{
    auto iter = std::find_if(m_layers.begin(), m_layers.end(),
        [&](const std::unique_ptr<SortingLayer>& layer) {
        return layer->id() == layerId;
    });

#ifdef DEBUG_MODE
    if (iter == m_layers.end()) {
        throw("Error, layer not found");
    }
#endif

    m_layers.erase(iter);

    // Invoke callback
    onRemoval(layerId);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// End namespaces
}