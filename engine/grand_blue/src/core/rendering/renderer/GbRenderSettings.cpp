#include "GbRenderSettings.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<RenderSetting> RenderSetting::create(const QJsonValue & json)
{
    std::shared_ptr<RenderSetting> setting;
    QJsonObject object = json.toObject();
    SettingType type = SettingType(object.value("type").toInt());
    switch (type) {
    case kCullFace:
        setting = std::make_shared<CullFaceSetting>();
        break;
    case kBlend:
        setting = std::make_shared<BlendSetting>();
        break;
    case kDepth:
        setting = std::make_shared<DepthSetting>();
        break;
    default:
        throw("Error, type of setting not recognized");
    }
    setting->loadFromJson(json);

    return setting;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSetting::RenderSetting(SettingType type, int order):
    m_settingType(type),
    m_order(order)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSetting::~RenderSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::bind()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::release()
{
    if (m_prevSetting) {
        m_prevSetting->bind();
    }

    // Throw error if previous setting is the same as the current
#ifdef DEBUG_MODE
    if (m_prevSetting.get() == this) {
        throw("Error previous setting is the same as the current one");
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderSetting::isSet() const
{
    return RenderSettings::CURRENT_SETTINGS.count(m_settingType);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RenderSetting::asJson() const
{
    QJsonObject object;
    object.insert("type", (int)m_settingType);
    object.insert("order", (int)m_order);

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::loadFromJson(const QJsonValue & json)
{
    QJsonObject object = json.toObject();
    m_settingType = (SettingType)object.value("type").toInt();
    m_order = object.value("order").toInt();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::makeCurrent()
{
    RenderSettings::CURRENT_SETTINGS[m_settingType] = shared_from_this();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::cachePreviousSettings()
{
    if (!isSet()) cacheSettings();
    m_prevSetting = RenderSettings::CURRENT_SETTINGS[m_settingType];
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cull Face
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::CullFaceSetting():
    RenderSetting(kCullFace, 0),
    m_cullFace(true),
    m_culledFace(GL_BACK)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::CullFaceSetting(bool cull, int culledFace) :
    RenderSetting(kCullFace, 0),
    m_cullFace(cull),
    m_culledFace(culledFace)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::~CullFaceSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CullFaceSetting::bind()
{
    cachePreviousSettings();

    if (m_cullFace) {
        // Enable face culling and set face
        gl()->glEnable(GL_CULL_FACE);
        gl()->glCullFace(m_culledFace);
    }
    else {
        // Disable face culling
        gl()->glDisable(GL_CULL_FACE);
    }

    makeCurrent();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue CullFaceSetting::asJson() const
{
    QJsonObject object = RenderSetting::asJson().toObject();
    object.insert("cullFace", m_cullFace);
    if(m_cullFace)
        object.insert("culledFace", int(m_culledFace));
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CullFaceSetting::loadFromJson(const QJsonValue & json)
{
    RenderSetting::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_cullFace = object["isCullFace"].toBool();
    if(m_cullFace)
        m_culledFace = object["culledFace"].toInt();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CullFaceSetting::cacheSettings()
{
    bool isCulled = gl()->glIsEnabled(GL_CULL_FACE);
    int culledFace;
    gl()->glGetIntegerv(GL_CULL_FACE_MODE, &culledFace);

    auto setting = std::make_shared<CullFaceSetting>(isCulled, culledFace);
    RenderSettings::CURRENT_SETTINGS[m_settingType] = setting;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Blend
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlendSetting::BlendSetting() :
    RenderSetting(kBlend, 0),
    m_blendEnabled(false),
    m_sourceAlpha(GL_SRC_ALPHA),
    m_sourceRGB(GL_ONE),
    m_destinationAlpha(GL_ONE_MINUS_SRC_ALPHA),
    m_destinationRGB(GL_ZERO),
    m_blendEquationAlpha(GL_FUNC_ADD),
    m_blendEquationRGB(GL_FUNC_ADD)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlendSetting::BlendSetting(bool blendEnabled, 
    int sourceAlphaFactor, int destAlphaFactor,
    int sourceRGBFactor, int destRGBFactor,
    int blendEquationAlpha,
    int blendEquationRGB):
    RenderSetting(kBlend, 0),
    m_blendEnabled(blendEnabled),
    m_sourceAlpha(sourceAlphaFactor),
    m_sourceRGB(sourceRGBFactor),
    m_destinationAlpha(destAlphaFactor),
    m_destinationRGB(destRGBFactor),
    m_blendEquationAlpha(blendEquationAlpha),
    m_blendEquationRGB(blendEquationRGB)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlendSetting::~BlendSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlendSetting::bind()
{
    cachePreviousSettings();

    if (m_blendEnabled) {
        // Enable blending
        gl()->glEnable(GL_BLEND);
        gl()->glBlendFuncSeparate(m_sourceAlpha, m_destinationAlpha,
            m_sourceRGB, m_destinationRGB);
        gl()->glBlendEquationSeparate(m_blendEquationRGB, m_blendEquationAlpha);
        gl()->glBlendColor(m_blendColor.x(), m_blendColor.y(), m_blendColor.z(),
            m_blendColor.w());
    }
    else {
        // Disable face culling
        gl()->glDisable(GL_BLEND);
    }

    makeCurrent();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue BlendSetting::asJson() const
{
    QJsonObject object = RenderSetting::asJson().toObject();
    object.insert("blendEnabled", m_blendEnabled);
    if (m_blendEnabled) {
        object.insert("sourceRGB", m_sourceRGB);
        object.insert("sourceAlpha", m_sourceAlpha);
        object.insert("destAlpha", m_destinationAlpha);
        object.insert("destRGB", m_destinationRGB);
        object.insert("beAlpha", m_blendEquationAlpha);
        object.insert("beRGB", m_blendEquationRGB);
        object.insert("blendColor", m_blendColor.asJson());
    }
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlendSetting::loadFromJson(const QJsonValue & json)
{
    RenderSetting::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_blendEnabled = object["blendEnabled"].toBool();
    if (m_blendEnabled) {
        m_sourceRGB = object["sourceRGB"].toInt(GL_ONE);
        m_sourceAlpha = object["sourceAlpha"].toInt(GL_SRC_ALPHA);
        m_destinationAlpha = object["destAlpha"].toInt(GL_ONE_MINUS_SRC_ALPHA);
        m_destinationRGB = object["destRGB"].toInt(GL_ZERO);
        m_blendEquationAlpha = object["beAlpha"].toInt(GL_FUNC_ADD);
        m_blendEquationRGB = object["beRGB"].toInt(GL_FUNC_ADD);
        m_blendColor = Vector4g(object["blendColor"]);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlendSetting::cacheSettings()
{
    bool blendEnabled = gl()->glIsEnabled(GL_BLEND);
    int sourceAlpha;
    int sourceRGB;
    int destAlpha;
    int destRGB;
    int blendEquationAlpha;
    int blendEquationRGB;
    float blendColor[4];

    glGetIntegerv(GL_BLEND_SRC_ALPHA, &sourceAlpha);
    glGetIntegerv(GL_BLEND_SRC_RGB, &sourceRGB);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &destAlpha);
    glGetIntegerv(GL_BLEND_DST_RGB, &destRGB);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRGB);
    glGetFloatv(GL_BLEND_COLOR, &blendColor[0]);
    std::vector<float> blendVec(blendColor, blendColor + 4);


    auto setting = std::make_shared<BlendSetting>(blendEnabled, 
        sourceAlpha, destAlpha, sourceRGB, destRGB);
    setting->setBlendColor(blendVec);
    RenderSettings::CURRENT_SETTINGS[m_settingType] = setting;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Depth Test
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::DepthSetting() :
    RenderSetting(kDepth, 0),
    m_testEnabled(true),
    m_mode(GL_LESS)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::DepthSetting(bool enable, int mode) :
    RenderSetting(kDepth, 0),
    m_testEnabled(enable),
    m_mode(mode)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::~DepthSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DepthSetting::bind()
{
    cachePreviousSettings();

    if (m_testEnabled) {
        // Enable face culling and set face
        gl()->glEnable(GL_DEPTH_TEST);
        gl()->glDepthFunc(m_mode);
    }
    else {
        // Disable face culling
        gl()->glDisable(GL_DEPTH_TEST);
    }

    makeCurrent();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue DepthSetting::asJson() const
{
    QJsonObject object = RenderSetting::asJson().toObject();
    object.insert("test", m_testEnabled);
    object.insert("mode", m_mode);
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DepthSetting::loadFromJson(const QJsonValue & json)
{
    RenderSetting::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_testEnabled = object["test"].toBool();
    m_mode = object["mode"].toInt();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DepthSetting::cacheSettings()
{
    bool enabled = gl()->glIsEnabled(GL_DEPTH_TEST);
    int mode;
    gl()->glGetIntegerv(GL_DEPTH_FUNC, &mode);

    auto setting = std::make_shared<DepthSetting>(enabled, mode);
    RenderSettings::CURRENT_SETTINGS[m_settingType] = setting;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Settings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_map<RenderSetting::SettingType, std::shared_ptr<RenderSetting>> RenderSettings::CURRENT_SETTINGS = {};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<RenderSetting> RenderSettings::current(RenderSetting::SettingType type)
{
    if (CURRENT_SETTINGS.count(type)) {
        return CURRENT_SETTINGS.at(type);
    }
    return nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::cacheSettings()
{
    CullFaceSetting cullFace;
    BlendSetting blend;
    DepthSetting depth;

    if (!cullFace.isSet()) {
        // Cache current GL settings for face culling
        cullFace.cacheSettings();
    }
    if (!blend.isSet()) {
        // Cache current GL settings for blending
        blend.cacheSettings();
    }
    if (!depth.isSet()) {
        // Cache current GL settings for depth testing
        depth.cacheSettings();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettings::RenderSettings(const QJsonValue & json):
    RenderSettings()
{
    loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettings::RenderSettings():
    m_shapeMode(GL_TRIANGLES)
{
    // Start conservatively, can always remove these flags in render loop
    setMaterialBind(true);
    setMaterialRelease(true);
    setShaderBind(true);
    //setShaderRelease(true); // Releasing is expensive, can just bind new shader directly
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettings::~RenderSettings()
{
    clearSettings();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::addDefaultBlend()
{
    auto blend = std::make_shared<BlendSetting>(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    addSetting(blend);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::addSetting(std::shared_ptr<RenderSetting> setting)
{
    if (!hasSetting(setting)) {
        m_settings.insert(setting);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::bind()
{
    for (const std::shared_ptr<RenderSetting>& setting : m_settings) {
        setting->bind();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::release()
{
    for (const std::shared_ptr<RenderSetting>& setting : m_settings) {
        setting->release();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::clearSettings()
{    
    //// Delete all settings
    //for (RenderSetting* setting : m_settings) {
    //    delete setting;
    //}
    m_settings.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RenderSettings::asJson() const
{
    QJsonObject object;

    // Cache settings
    QJsonArray settings;
    for(auto setting : m_settings) {
        settings.append(setting->asJson());
    }
    object.insert("settings", settings);

    // Rendering mode
    object.insert("shapeMode", int(m_shapeMode));

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::loadFromJson(const QJsonValue & json)
{
    // Clear current settings
    clearSettings();

    const QJsonObject& object = json.toObject();

    // Load which settings to toggle
    const QJsonArray& settings = object.value("settings").toArray();
    for (const auto& settingJson : settings) {
        // Preserve backwards compatability
        QJsonObject settingObject = settingJson.toObject();
        if (!settingObject.contains("type")) continue;

        auto setting = RenderSetting::create(settingJson);
        m_settings.insert(setting);
    }

    // Cache primitive mode, triangles by default
    m_shapeMode = object["shapeMode"].toInt(GL_TRIANGLES);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing