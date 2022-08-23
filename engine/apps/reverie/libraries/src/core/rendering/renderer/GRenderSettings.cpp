#include "core/rendering/renderer/GRenderSettings.h"
#include "core/rendering/renderer/GRenderContext.h"

namespace rev {


std::shared_ptr<RenderSetting> RenderSetting::Create(const nlohmann::json& korJson)
{
    std::shared_ptr<RenderSetting> setting;
    
    RenderSettingType type = RenderSettingType(korJson.at("type").get<Int32_t>());
    switch (type) {
    case kCullFace:
        setting = std::make_shared<CullFaceSetting>();
        FromJson<CullFaceSetting>(korJson, *setting);
        break;
    case kBlend:
        setting = std::make_shared<BlendSetting>();
        FromJson<BlendSetting>(korJson, *setting);
        break;
    case kDepth:
        setting = std::make_shared<DepthSetting>();
        FromJson<DepthSetting>(korJson, *setting);
        break;
    default:
        Logger::Throw("Error, type of setting not recognized");
    }

    return setting;
}

RenderSetting::RenderSetting()
{
}

RenderSetting::~RenderSetting()
{
}

json RenderSetting::asJson() const
{
    json myJson;
    switch (settingType()) {
    case RenderSettingType::kCullFace:
        ToJson<CullFaceSetting>(myJson, *this);
        break;
    case RenderSettingType::kBlend:
        ToJson<BlendSetting>(myJson, *this);
        break;
    case RenderSettingType::kDepth:
        ToJson<DepthSetting>(myJson, *this);
        break;
    case RenderSettingType::kStencil:
        ToJson<StencilSetting>(myJson, *this);
        break;
    default:
        Logger::Throw("Error, invalid render setting type");
    }
    return myJson;
}

void RenderSetting::bind(RenderContext& context)
{
    if (!context.isCurrent()) {
        Logger::Throw("Error context is not current");
    }

    // If no settings in render context, obtain from OpenGL
    if (!isSet(context)) {
        //cacheSettings(context);
        Logger::Throw("Error, settings not cached from OpenGL");
    }

    // Cache previous setting in render context
    //context.previousRenderSettings().addSetting(
    //    *context.renderSettings().setting(settingType()));

    // Set settings
    set(context);
    
    // Make current setting in render context
    //makeCurrent(context);
}

void RenderSetting::release(RenderContext& context)
{
    if (!context.isCurrent()) {
        Logger::Throw("Error context is not current");
    }

    // Set previous settings back in OpenGL
    const RenderSetting* prevSetting = context.renderSettings().setting(settingType());
    if (!prevSetting) {
        Logger::Throw("No previous setting");
    }
    prevSetting->set(context);

    // Add previous setting back as setting in render context
    //prevSetting->makeCurrent(context);

    // Throw error if previous setting is the same as the current
#ifdef DEBUG_MODE
    if (prevSetting == this) {
        Logger::Throw("Error previous setting is the same as the current one");
    }
#endif
}

bool RenderSetting::isSet(RenderContext& context) const
{
    if (!context.isCurrent()) {
        Logger::Throw("Error context is not current");
    }
    return context.renderSettings().setting(settingType()) != nullptr;
}

void to_json(json& orJson, const RenderSetting& korObject)
{
    orJson["type"] = (int)korObject.settingType();
}

void from_json(const json& korJson, RenderSetting& orObject)
{
    Q_UNUSED(korJson)
    Q_UNUSED(orObject)
}

void RenderSetting::makeCurrent(RenderContext& context) const
{
    context.renderSettings().addSetting(*this);
}




// Cull Face

CullFaceSetting CullFaceSetting::Current(RenderContext & context)
{
    if (!context.isCurrent()) {
        Logger::Throw("Error context is not current");
    }

    bool isCulled = context.functions()->glIsEnabled(GL_CULL_FACE);
    int culledFace;
    context.functions()->glGetIntegerv(GL_CULL_FACE_MODE, &culledFace);

    return CullFaceSetting(isCulled, (CulledFace)culledFace);
}

CullFaceSetting::CullFaceSetting():
    RenderSetting(),
    m_cullFace(true),
    m_culledFace(CulledFace::kBack),
    m_cullFlags(0)
{
}

CullFaceSetting::CullFaceSetting(CulledFace culledFace):
    RenderSetting(),
    m_culledFace(culledFace),
    m_cullFlags(kModifyFace)
{
}

CullFaceSetting::CullFaceSetting(bool cull) :
    RenderSetting(),
    m_cullFace(cull),
    m_cullFlags(kModifyEnable)
{
}

CullFaceSetting::CullFaceSetting(bool cull, CulledFace culledFace) :
    RenderSetting(),
    m_cullFace(cull),
    m_culledFace(culledFace),
    m_cullFlags(kModifyFace | kModifyEnable)
{
}

CullFaceSetting::~CullFaceSetting()
{
}

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

bool CullFaceSetting::operator==(const CullFaceSetting & other) const
{
    return m_cullFlags == other.m_cullFlags &&
        m_cullFace == other.m_cullFace &&
        m_culledFace == other.m_culledFace;
}

bool CullFaceSetting::operator!=(const CullFaceSetting & other) const
{
    return !operator==(other);
}

void to_json(json& orJson, const CullFaceSetting& korObject)
{
    ToJson<RenderSetting>(orJson, korObject);
    orJson["cullFace"] = korObject.m_cullFace;
    if (korObject.m_cullFace)
    {
        orJson["culledFace"] = int(korObject.m_culledFace);
    }
}

void from_json(const json& korJson, CullFaceSetting& orObject)
{
    FromJson<RenderSetting>(korJson, orObject);
    
    orObject.m_cullFace = korJson.at("cullFace").get<bool>();
    if (orObject.m_cullFace) {
        orObject.m_culledFace = CulledFace(korJson.at("culledFace").get<Int32_t>());
    }
}

void CullFaceSetting::cacheSettings(RenderContext& context)
{
    context.renderSettings().addSetting(Current(context));
}





// Blend

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

BlendSetting::~BlendSetting()
{
}

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

void to_json(json& orJson, const BlendSetting& korObject)
{
    ToJson<RenderSetting>(orJson, korObject);
    orJson["blendEnabled"] = korObject.m_blendEnabled;
    if (korObject.m_blendEnabled) {
        orJson["sourceRGB"] = korObject.m_sourceRGB;
        orJson["sourceAlpha"] = korObject.m_sourceAlpha;
        orJson["destAlpha"] = korObject.m_destinationAlpha;
        orJson["destRGB"] = korObject.m_destinationRGB;
        orJson["beAlpha"] = korObject.m_blendEquationAlpha;
        orJson["beRGB"] = korObject.m_blendEquationRGB;
        orJson["blendColor"] = korObject.m_blendColor;
    }
}

void from_json(const json& korJson, BlendSetting& orObject)
{
    FromJson<RenderSetting>(korJson, orObject);
    
    orObject.m_blendEnabled = korJson.at("blendEnabled").get<bool>();
    if (orObject.m_blendEnabled) {
        orObject.m_sourceRGB = korJson.value("sourceRGB", GL_ONE);
        orObject.m_sourceAlpha = korJson.value("sourceAlpha", GL_SRC_ALPHA);
        orObject.m_destinationAlpha = korJson.value("destAlpha", GL_ONE_MINUS_SRC_ALPHA);
        orObject.m_destinationRGB = korJson.value("destRGB", GL_ZERO);
        orObject.m_blendEquationAlpha = korJson.value("beAlpha", GL_FUNC_ADD);
        orObject.m_blendEquationRGB = korJson.value("beRGB", GL_FUNC_ADD);
        orObject.m_blendColor = Vector4(korJson["blendColor"]);
    }
}

void BlendSetting::cacheSettings(RenderContext& context)
{
    if (!context.isCurrent()) {
        Logger::Throw("Error context is not current");
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




// Depth Test


DepthSetting DepthSetting::Current(RenderContext & context)
{
    if (!context.isCurrent()) {
        Logger::Throw("Error context is not current");
    }

    bool enabled = context.functions()->glIsEnabled(GL_DEPTH_TEST);
    int mode, writeMask;
    context.functions()->glGetIntegerv(GL_DEPTH_FUNC, &mode);
    context.functions()->glGetIntegerv(GL_DEPTH_WRITEMASK, &writeMask);

    return DepthSetting(enabled, DepthPassMode(mode), writeMask);
}

DepthSetting::DepthSetting() :
    RenderSetting(),
    m_testEnabled(true),
    m_passMode(DepthPassMode::kLessEqual),
    m_writeToDepthBuffer(true),
    m_depthFlags(0)
{
}

DepthSetting::DepthSetting(bool enableTesting, DepthPassMode depthPassMode) :
    RenderSetting(),
    m_testEnabled(enableTesting),
    m_passMode(depthPassMode),
    m_depthFlags(kModifyTesting | kModifyPassMode)
{
}

DepthSetting::DepthSetting(bool enableTesting, DepthPassMode depthPassMode, bool writeToDepthBuffer) :
    RenderSetting(),
    m_testEnabled(enableTesting),
    m_passMode(depthPassMode),
    m_writeToDepthBuffer(writeToDepthBuffer),
    m_depthFlags(kModifyWrite | kModifyTesting | kModifyPassMode)
{
}

DepthSetting::DepthSetting(DepthPassMode depthPassMode, bool writeToDepthBuffer) :
    RenderSetting(),
    m_testEnabled(true),
    m_passMode(depthPassMode),
    m_writeToDepthBuffer(writeToDepthBuffer),
    m_depthFlags(kModifyWrite | kModifyPassMode)
{
}

DepthSetting::~DepthSetting()
{
}

bool DepthSetting::operator==(const DepthSetting & other) const
{
    return m_passMode == other.m_passMode &&
        m_testEnabled == other.m_testEnabled &&
        m_writeToDepthBuffer == other.m_writeToDepthBuffer &&
        m_depthFlags == other.m_depthFlags;
}

bool DepthSetting::operator!=(const DepthSetting & other) const
{
    return !operator==(other);
}

void DepthSetting::set(RenderContext& context) const
{
    if (!m_testEnabled && m_writeToDepthBuffer) {
        Logger::Throw("Error, cannot write to depth buffer if testing disabled");
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

void to_json(json& orJson, const DepthSetting& korObject)
{
    ToJson<RenderSetting>(orJson, korObject);
    orJson["depthFlags"] = (int)korObject.m_depthFlags;
    orJson["test"] = korObject.m_testEnabled;
    orJson["mode"] = (int)korObject.m_passMode;
    orJson["writeDepth"] = (int)korObject.m_writeToDepthBuffer;
}

void from_json(const json& korJson, DepthSetting& orObject)
{
    FromJson<RenderSetting>(korJson, orObject);
    
    if (korJson.contains("depthFlags")) {
        orObject.m_depthFlags = korJson.at("depthFlags").get<Int32_t>();
    }
    orObject.m_testEnabled = korJson.at("test").get<bool>();
    orObject.m_passMode = (DepthPassMode)korJson.at("mode").get<Int32_t>();
    orObject.m_writeToDepthBuffer = korJson.value("writeDepth", true);
}

void DepthSetting::cacheSettings(RenderContext& context)
{
    context.renderSettings().addSetting(Current(context));
}




// Stencil Setting

StencilSetting StencilSetting::Current(RenderContext & context)
{
    return StencilSetting();
}

StencilSetting::StencilSetting()
{
}

StencilSetting::~StencilSetting()
{
}

void StencilSetting::set(RenderContext & context) const
{
    if (!m_testEnabled && isWritingStencil()) {
        Logger::Throw("Error, cannot write to stencil buffer if testing disabled");
    }

    if (isSettingTesting()) {
        if (m_testEnabled) {
            // Enable stencil test
            context.functions()->glEnable(GL_STENCIL_TEST);
        }
        else {
            // Disable stencil test
            context.functions()->glDisable(GL_STENCIL_TEST);
        }
    }

    if (isSettingPassMode()) {
        context.functions()->glStencilFunc((int)m_passMode, m_ref, m_mask);
    }

    if (isSettingStencilWrite()) {
        // Whether or not to write to stencil buffer
        context.functions()->glStencilMask(m_mask);
    }

    if (isSettingOp()) {
        // Modify stencil buffer behavior, which by default is (GL_KEEP, GL_KEEP, GL_KEEP), which does not update buffer ever
        context.functions()->glStencilOp((GLint)m_failOp, (GLint)m_depthFailOp, (GLint)m_passOp);
    }
}

bool StencilSetting::operator==(const StencilSetting & other) const
{
    return m_passMode == other.m_passMode &&
        m_testEnabled == other.m_testEnabled &&
        m_mask == other.m_mask &&
        m_stencilFlags == other.m_stencilFlags &&
        m_ref == other.m_ref;
        m_failOp == other.m_failOp &&
        m_depthFailOp == other.m_depthFailOp &&
        m_passOp == other.m_passOp
        ;
}

bool StencilSetting::operator!=(const StencilSetting & other) const
{
    return !operator==(other);
}

void to_json(json& orJson, const StencilSetting& korObject)
{
    ToJson<RenderSetting>(orJson, korObject);
    orJson["stencilFlags"] = (int)korObject.m_stencilFlags;
    orJson["test"] = korObject.m_testEnabled;
    orJson["mode"] = (int)korObject.m_passMode;
    orJson["mask"] = (int)korObject.m_mask;
    orJson["ref"] = (int)korObject.m_ref;
    orJson["failOp"] = (int)korObject.m_failOp;
    orJson["depthFailOp"] = (int)korObject.m_depthFailOp;
    orJson["passOp"] = (int)korObject.m_passOp;
}

void from_json(const json& korJson, StencilSetting& orObject)
{
    FromJson<RenderSetting>(korJson, orObject);
    
    orObject.m_stencilFlags = korJson.at("stencilFlags").get<Int32_t>();
    orObject.m_testEnabled = korJson.at("test").get<bool>();
    orObject.m_passMode = (StencilPassMode)korJson.at("mode").get<Int32_t>();
    orObject.m_mask = korJson.value("mask", true);
    orObject.m_ref = korJson.at("ref").get<Int32_t>();
    orObject.m_failOp = (StencilBufferOp)korJson.at("failOp").get<Int32_t>();
    orObject.m_depthFailOp = (StencilBufferOp)korJson.at("depthFailOp").get<Int32_t>();
    orObject.m_passOp = (StencilBufferOp)korJson.at("passOp").get<Int32_t>();
}

void StencilSetting::cacheSettings(RenderContext & context)
{
    context.renderSettings().addSetting(Current(context));
}




// Render Settings

void RenderSettings::CacheSettings(RenderContext& context)
{
    CullFaceSetting cullFace;
    BlendSetting blend;
    DepthSetting depth;
    StencilSetting stencil;

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
    if (!stencil.isSet(context)) {
        // Cache current GL settings for stencil testing
        stencil.cacheSettings(context);
    }
}


RenderSettings::RenderSettings(const nlohmann::json& json):
    RenderSettings()
{
    for (size_t i = 0; i < m_settings.size(); i++) {
        m_settings[i] = nullptr;
    }
    json.get_to(*this);
}

RenderSettings::RenderSettings():
    m_shapeMode(PrimitiveMode::kTriangles),
    m_numInstances(1)
{
    for (size_t i = 0; i < m_settings.size(); i++) {
        m_settings[i] = nullptr;
    }
}

RenderSettings::~RenderSettings()
{
    clearSettings();
}

void RenderSettings::overrideSettings(const RenderSettings & other)
{
    for (const auto& setting : other.m_settings) {
        if (setting) {
            addSetting(*setting);
        }
    }
    m_shapeMode = other.m_shapeMode;
}

void RenderSettings::addDefaultBlend()
{
    addSetting<BlendSetting>(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderSettings::addSetting(const RenderSetting& setting)
{
    RenderSetting* oldSetting = m_settings[(int)setting.settingType()];
    if (oldSetting) {
        delete oldSetting;
        m_settings[(int)setting.settingType()] = nullptr;
    }

    RenderSetting* newSetting;
    switch (setting.settingType()) {
    case RenderSettingType::kCullFace:
        newSetting = new CullFaceSetting(static_cast<const CullFaceSetting&>(setting));
        break;
    case RenderSettingType::kBlend:
        newSetting = new BlendSetting(static_cast<const BlendSetting&>(setting));
        break;
    case RenderSettingType::kDepth:
        newSetting = new DepthSetting(static_cast<const DepthSetting&>(setting));
        break;
    case RenderSettingType::kStencil:
        newSetting = new StencilSetting(static_cast<const StencilSetting&>(setting));
        break;
    default:
        Logger::Throw("Unimplemented");
    }
    m_settings[(int)setting.settingType()] = newSetting;
}

void RenderSettings::bind(RenderContext& context)
{
    for (RenderSetting* setting : m_settings) {
        if (setting){
            setting->bind(context);
        }
    }
}

void RenderSettings::release(RenderContext& context)
{
    for (RenderSetting* setting : m_settings) {
        if (setting) {
            setting->release(context);
        }
    }
}

void RenderSettings::clearSettings()
{    
    for (size_t i = 0; i < m_settings.size(); i++) {
        delete m_settings[i];
        m_settings[i] = nullptr;
    }
}

void to_json(json& orJson, const RenderSettings& korObject)
{
      // Cache settings
    json settings = json::array();
    for(RenderSetting* setting : korObject.m_settings) {
        if (setting) { 
            settings.push_back(setting->asJson());
        }
    }
    orJson["settings"] = settings;

    // Rendering mode
    orJson["shapeMode"] = int(korObject.m_shapeMode);

    // Transparency mode
    orJson["transparencyMode"] = int(korObject.m_transparencyMode);
}

void from_json(const json& korJson, RenderSettings& orObject)
{
    // Clear current settings
    orObject.clearSettings();

    // Load which settings to toggle
    const json& settings = korJson.at("settings");
    for (const auto& settingJson : settings) {
        // Preserve backwards compatability
        const json& settingObject = settingJson;
        if (!settingObject.contains("type")) continue;

        std::shared_ptr<RenderSetting> setting = RenderSetting::Create(settingJson);
        orObject.addSetting(*setting);
    }

    // Load primitive mode, triangles by default
    orObject.m_shapeMode = (PrimitiveMode)korJson.value("shapeMode", (int)PrimitiveMode::kTriangles);

    // Load transparency mode
    orObject.m_transparencyMode = (TransparencyRenderMode)korJson.value("transparencyMode", (int)TransparencyRenderMode::kOpaque);
}





} // end namespacing