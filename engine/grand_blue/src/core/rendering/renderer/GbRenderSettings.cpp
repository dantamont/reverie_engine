#include "GbRenderSettings.h"
#include "GbRenderContext.h"

namespace Gb {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<RenderSetting> RenderSetting::Create(const QJsonValue & json)
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
RenderSetting::RenderSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSetting::~RenderSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::bind(RenderContext& context)
{
    if (!context.isCurrent()) {
        throw("Error context is not current");
    }

    // If no settings in render context, obtain from OpenGL
    if (!isSet(context)) {
        //cacheSettings(context);
        throw("Error, settings not cached from OpenGL");
    }

    // Cache previous setting in render context
    //context.previousRenderSettings().addSetting(
    //    *context.renderSettings().setting(settingType()));

    // Set settings
    set(context);
    
    // Make current setting in render context
    //makeCurrent(context);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::release(RenderContext& context)
{
    if (!context.isCurrent()) {
        throw("Error context is not current");
    }

    // Set previous settings back in OpenGL
    const RenderSetting* prevSetting = context.renderSettings().setting(settingType());
    if (!prevSetting) {
        throw("No previous setting");
    }
    prevSetting->set(context);

    // Add previous setting back as setting in render context
    //prevSetting->makeCurrent(context);

    // Throw error if previous setting is the same as the current
#ifdef DEBUG_MODE
    if (prevSetting == this) {
        throw("Error previous setting is the same as the current one");
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderSetting::isSet(RenderContext& context) const
{
    if (!context.isCurrent()) {
        throw("Error context is not current");
    }
    return context.renderSettings().setting(settingType()) != nullptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RenderSetting::asJson() const
{
    QJsonObject object;
    object.insert("type", (int)settingType());

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(json)
    Q_UNUSED(context)

    //QJsonObject object = json.toObject();
    //m_settingType = (SettingType)object.value("type").toInt();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSetting::makeCurrent(RenderContext& context) const
{
    context.renderSettings().addSetting(*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cull Face
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting CullFaceSetting::Current(RenderContext & context)
{
    if (!context.isCurrent()) {
        throw("Error context is not current");
    }

    bool isCulled = context.functions()->glIsEnabled(GL_CULL_FACE);
    int culledFace;
    context.functions()->glGetIntegerv(GL_CULL_FACE_MODE, &culledFace);

    return CullFaceSetting(isCulled, (CulledFace)culledFace);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::CullFaceSetting():
    RenderSetting(),
    m_cullFace(true),
    m_culledFace(CulledFace::kBack),
    m_cullFlags(0)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::CullFaceSetting(CulledFace culledFace):
    RenderSetting(),
    m_culledFace(culledFace),
    m_cullFlags(kModifyFace)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::CullFaceSetting(bool cull) :
    RenderSetting(),
    m_cullFace(cull),
    m_cullFlags(kModifyEnable)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::CullFaceSetting(bool cull, CulledFace culledFace) :
    RenderSetting(),
    m_cullFace(cull),
    m_culledFace(culledFace),
    m_cullFlags(kModifyFace | kModifyEnable)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CullFaceSetting::~CullFaceSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CullFaceSetting::set(RenderContext& context) const
{
    if (isSettingEnabled()) {
        if (m_cullFace) {
            // Enable face culling and set face
            context.functions()->glEnable(GL_CULL_FACE);
        }
        else {
            // Disable face culling
            context.functions()->glDisable(GL_CULL_FACE);
        }
    }

    if (isSettingCullFace()) {
        context.functions()->glCullFace((int)m_culledFace);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CullFaceSetting::operator==(const CullFaceSetting & other) const
{
    return m_cullFlags == other.m_cullFlags &&
        m_cullFace == other.m_cullFace &&
        m_culledFace == other.m_culledFace;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CullFaceSetting::operator!=(const CullFaceSetting & other) const
{
    return !operator==(other);
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
void CullFaceSetting::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    RenderSetting::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_cullFace = object["isCullFace"].toBool();
    if (m_cullFace) {
        m_culledFace = CulledFace(object["culledFace"].toInt());
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CullFaceSetting::cacheSettings(RenderContext& context)
{
    context.renderSettings().addSetting(Current(context));
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Blend
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BlendSetting::BlendSetting() :
    RenderSetting(),
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
    RenderSetting(),
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
void BlendSetting::set(RenderContext& context) const
{
    if (m_blendEnabled) {
        // Enable blending
        context.functions()->glEnable(GL_BLEND);
        context.functions()->glBlendFuncSeparate(m_sourceAlpha, m_destinationAlpha,
            m_sourceRGB, m_destinationRGB);
        context.functions()->glBlendEquationSeparate(m_blendEquationRGB, m_blendEquationAlpha);
        context.functions()->glBlendColor(m_blendColor.x(), m_blendColor.y(), m_blendColor.z(),
            m_blendColor.w());
    }
    else {
        // Disable face culling
        context.functions()->glDisable(GL_BLEND);
    }
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
void BlendSetting::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

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
        m_blendColor = Vector4(object["blendColor"]);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BlendSetting::cacheSettings(RenderContext& context)
{
    if (!context.isCurrent()) {
        throw("Error context is not current");
    }

    bool blendEnabled = context.functions()->glIsEnabled(GL_BLEND);
    int sourceAlpha;
    int sourceRGB;
    int destAlpha;
    int destRGB;
    int blendEquationAlpha;
    int blendEquationRGB;
    float blendColor[4];

    context.functions()->glGetIntegerv(GL_BLEND_SRC_ALPHA, &sourceAlpha);
    context.functions()->glGetIntegerv(GL_BLEND_SRC_RGB, &sourceRGB);
    context.functions()->glGetIntegerv(GL_BLEND_DST_ALPHA, &destAlpha);
    context.functions()->glGetIntegerv(GL_BLEND_DST_RGB, &destRGB);
    context.functions()->glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha);
    context.functions()->glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRGB);
    context.functions()->glGetFloatv(GL_BLEND_COLOR, &blendColor[0]);
    std::vector<float> blendVec(blendColor, blendColor + 4);


    BlendSetting setting(blendEnabled,
        sourceAlpha, destAlpha, sourceRGB, destRGB);
    setting.setBlendColor(blendVec);

    context.renderSettings().addSetting(setting);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Depth Test
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting DepthSetting::Current(RenderContext & context)
{
    if (!context.isCurrent()) {
        throw("Error context is not current");
    }

    bool enabled = context.functions()->glIsEnabled(GL_DEPTH_TEST);
    int mode, writeMask;
    context.functions()->glGetIntegerv(GL_DEPTH_FUNC, &mode);
    context.functions()->glGetIntegerv(GL_DEPTH_WRITEMASK, &writeMask);

    return DepthSetting(enabled, DepthPassMode(mode), writeMask);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::DepthSetting() :
    RenderSetting(),
    m_testEnabled(true),
    m_passMode(DepthPassMode::kLessEqual),
    m_writeToDepthBuffer(true),
    m_depthFlags(0)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::DepthSetting(bool enableTesting, DepthPassMode depthPassMode) :
    RenderSetting(),
    m_testEnabled(enableTesting),
    m_passMode(depthPassMode),
    m_depthFlags(kModifyTesting | kModifyPassMode)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::DepthSetting(bool enableTesting, DepthPassMode depthPassMode, bool writeToDepthBuffer) :
    RenderSetting(),
    m_testEnabled(enableTesting),
    m_passMode(depthPassMode),
    m_writeToDepthBuffer(writeToDepthBuffer),
    m_depthFlags(kModifyWrite | kModifyTesting | kModifyPassMode)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::DepthSetting(DepthPassMode depthPassMode, bool writeToDepthBuffer) :
    RenderSetting(),
    m_testEnabled(true),
    m_passMode(depthPassMode),
    m_writeToDepthBuffer(writeToDepthBuffer),
    m_depthFlags(kModifyWrite | kModifyPassMode)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DepthSetting::~DepthSetting()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DepthSetting::operator==(const DepthSetting & other) const
{
    return m_passMode == other.m_passMode &&
        m_testEnabled == other.m_testEnabled &&
        m_writeToDepthBuffer == other.m_writeToDepthBuffer &&
        m_depthFlags == other.m_depthFlags;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DepthSetting::operator!=(const DepthSetting & other) const
{
    return !operator==(other);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DepthSetting::set(RenderContext& context) const
{
    if (!m_testEnabled && m_writeToDepthBuffer) {
        throw("Error, cannot write to depth buffer if testing disabled");
    }

    if (isSettingTesting()) {
        if (m_testEnabled) {
            // Enable face culling and set face
            context.functions()->glEnable(GL_DEPTH_TEST);
        }
        else {
            // Disable depth test
            context.functions()->glDisable(GL_DEPTH_TEST);
        }
    }

    if (isSettingPassMode()) {
        context.functions()->glDepthFunc((int)m_passMode);
    }

    if (isSettingDepthWrite()) {
        // Whether or not to write to depth buffer
        context.functions()->glDepthMask(m_writeToDepthBuffer);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue DepthSetting::asJson() const
{
    QJsonObject object = RenderSetting::asJson().toObject();
    object.insert("test", m_testEnabled);
    object.insert("mode", (int)m_passMode);
    object.insert("writeDepth", (int)m_writeToDepthBuffer);
    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DepthSetting::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    RenderSetting::loadFromJson(json);
    QJsonObject object = json.toObject();
    m_testEnabled = object["test"].toBool();
    m_passMode = (DepthPassMode)object["mode"].toInt();
    m_writeToDepthBuffer = object["writeDepth"].toBool(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DepthSetting::cacheSettings(RenderContext& context)
{
    context.renderSettings().addSetting(Current(context));
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Settings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::CacheSettings(RenderContext& context)
{
    CullFaceSetting cullFace;
    BlendSetting blend;
    DepthSetting depth;

    if (!cullFace.isSet(context)) {
        // Cache current GL settings for face culling
        cullFace.cacheSettings(context);
    }
    if (!blend.isSet(context)) {
        // Cache current GL settings for blending
        blend.cacheSettings(context);
    }
    if (!depth.isSet(context)) {
        // Cache current GL settings for depth testing
        depth.cacheSettings(context);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettings::RenderSettings(const QJsonValue & json):
    RenderSettings()
{
    for (size_t i = 0; i < m_settings.size(); i++) {
        m_settings[i] = nullptr;
    }
    loadFromJson(json);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettings::RenderSettings():
    m_shapeMode(GL_TRIANGLES)
{
    for (size_t i = 0; i < m_settings.size(); i++) {
        m_settings[i] = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderSettings::~RenderSettings()
{
    clearSettings();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::overrideSettings(const RenderSettings & other)
{
    for (const auto& setting : other.m_settings) {
        if (setting) {
            addSetting(*setting);
        }
    }
    m_shapeMode = other.m_shapeMode;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::addDefaultBlend()
{
    addSetting<BlendSetting>(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::addSetting(const RenderSetting& setting)
{
    RenderSetting* oldSetting = m_settings[(int)setting.settingType()];
    if (oldSetting) {
        delete oldSetting;
        m_settings[(int)setting.settingType()] = nullptr;
    }

    RenderSetting* newSetting;
    switch (setting.settingType()) {
    case RenderSetting::kCullFace:
        newSetting = new CullFaceSetting(static_cast<const CullFaceSetting&>(setting));
        break;
    case RenderSetting::kBlend:
        newSetting = new BlendSetting(static_cast<const BlendSetting&>(setting));
        break;
    case RenderSetting::kDepth:
        newSetting = new DepthSetting(static_cast<const DepthSetting&>(setting));
        break;
    default:
        throw("Unimplemented");
    }
    m_settings[(int)setting.settingType()] = newSetting;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::bind(RenderContext& context)
{
    for (RenderSetting* setting : m_settings) {
        if (setting){
            setting->bind(context);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::release(RenderContext& context)
{
    for (RenderSetting* setting : m_settings) {
        if (setting) {
            setting->release(context);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::clearSettings()
{    
    for (size_t i = 0; i < m_settings.size(); i++) {
        delete m_settings[i];
        m_settings[i] = nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QJsonValue RenderSettings::asJson() const
{
    QJsonObject object;

    // Cache settings
    QJsonArray settings;
    for(RenderSetting* setting : m_settings) {
        if (setting) { 
            settings.append(setting->asJson());
        }
    }
    object.insert("settings", settings);

    // Rendering mode
    object.insert("shapeMode", int(m_shapeMode));

    return object;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderSettings::loadFromJson(const QJsonValue& json, const SerializationContext& context)
{
    Q_UNUSED(context)

    // Clear current settings
    clearSettings();

    const QJsonObject& object = json.toObject();

    // Load which settings to toggle
    const QJsonArray& settings = object.value("settings").toArray();
    for (const auto& settingJson : settings) {
        // Preserve backwards compatability
        QJsonObject settingObject = settingJson.toObject();
        if (!settingObject.contains("type")) continue;

        std::shared_ptr<RenderSetting> setting = RenderSetting::Create(settingJson);
        addSetting(*setting);
    }

    // Cache primitive mode, triangles by default
    m_shapeMode = object["shapeMode"].toInt(GL_TRIANGLES);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end namespacing