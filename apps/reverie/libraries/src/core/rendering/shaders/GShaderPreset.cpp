#include "core/rendering/shaders/GShaderPreset.h"

// Qt
#include <QDebug>

// internal
#include "core/GCoreEngine.h"
#include "core/resource/GResourceCache.h"
#include "fortress/process/GProcess.h"
#include "fortress/json/GJson.h"
#include "core/rendering/GGLFunctions.h"
#include "core/rendering/renderer/GRenderCommand.h"
#include "core/rendering/renderer/GRenderContext.h"
#include "core/rendering/renderer/GOpenGlRenderer.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniformContainer.h"
#include "core/scene/GScenario.h"
#include "core/scene/GScenarioSettings.h"

namespace rev {



// ShaderPreset

std::shared_ptr<ShaderPreset> ShaderPreset::GetBuiltin(const GString & name)
{
    auto iter = std::find_if(s_builtins.begin(), s_builtins.end(),
        [&](const std::shared_ptr<ShaderPreset>& preset) {
        return preset->getName() == name;
    });

    bool hasPreset = iter != s_builtins.end();
    if (hasPreset) {
        return *iter;
    }
    else {
        return nullptr;
    }
}

void ShaderPreset::InitializeBuiltins(CoreEngine* engine)
{
    s_builtins.clear();

    static GString canvasGui = "canvas_gui";
    static GString canvasBillboard = "canvas_billboard";

    // Create canvas gui preset and set shader
    std::shared_ptr<ShaderPreset> canvasGuiPreset = std::make_shared<ShaderPreset>(engine, canvasGui);
    auto canvasGuiShader = ResourceCache::Instance().getHandleWithName(
        canvasGui, EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
    canvasGuiPreset->setShaderProgram(canvasGuiShader);
    s_builtins.push_back(canvasGuiPreset);

    // Create billboard preset and set shader 
    std::shared_ptr<ShaderPreset> canvasBillboardPreset = std::make_shared<ShaderPreset>(engine, canvasBillboard);
    auto canvasBillboardShader = ResourceCache::Instance().getHandleWithName(
        canvasBillboard, EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
    canvasBillboardPreset->setShaderProgram(canvasBillboardShader);
    s_builtins.push_back(canvasBillboardPreset);
}

ShaderPreset::ShaderPreset(CoreEngine * core, const nlohmann::json& json) :
    m_engine(core),
    m_uniformValues(core->openGlRenderer()->renderContext().uniformContainer())
{
    json.get_to(*this);
}

ShaderPreset::ShaderPreset(CoreEngine* core, const GString& name):
    NameableInterface(name),
    m_engine(core),
    m_uniformValues(core->openGlRenderer()->renderContext().uniformContainer())
{
}

ShaderPreset::~ShaderPreset()
{
}

void ShaderPreset::queueUniforms()
{
    for (const Uniform& uniform : m_uniforms) {
        m_shaderProgram->setUniformValue(uniform);
    }
}

void ShaderPreset::applyPreset(DrawCommand& command) const
{
    // Set uniforms in draw command
    command.addUniforms(m_uniforms, m_prepassUniformIds);

    // Set render settings in draw command
    command.renderSettings().overrideSettings(m_renderSettings);
}

void ShaderPreset::addUniform(const Uniform& uniform)
{
#ifdef DEBUG_MODE
    if (uniform.getId() == -1) {
        Logger::Throw("Error, uniform is invalid");
    }
#endif

    // Assumes that since uniform is already created, there is no value to update
    // if the shader preset already has the uniform
    int idx = -1;
    if (!hasUniform(uniform, &idx)) {
        m_uniforms.push_back(uniform);
        if (m_prepassShaderProgram) {
            const GStringView& uniformName = uniform.getName(*m_shaderProgram);
            Int32_t prepassId{ -1 };
            if (m_prepassShaderProgram->hasUniform(uniformName)) {
                prepassId = m_prepassShaderProgram->getUniformId(uniformName);
            }
            m_prepassUniformIds.push_back(prepassId);
        }
    }
}

bool ShaderPreset::hasUniform(Uint32_t uniformId, int* outIndex)
{
    auto iter = std::find_if(m_uniforms.begin(), m_uniforms.end(),
        [&](const Uniform& u) {
            return u.getId() == uniformId;
        });

    if (iter == m_uniforms.end()) {
        return false;
    }
    else {
        // Replace uniform if already set
        *outIndex = iter - m_uniforms.begin();
        return true;
    }
}

bool ShaderPreset::hasUniform(const Uniform& uniform, int* outIndex)
{
    return hasUniform(uniform.getId(), outIndex);
}

void ShaderPreset::clearUniforms()
{
    m_uniforms.clear();
    m_prepassUniformIds.clear();
    m_uniformValues.clear();
}


void to_json(json& orJson, const ShaderPreset& korObject)
{
    orJson["renderSettings"] = korObject.m_renderSettings;

    orJson["name"] = korObject.m_name.c_str();

    // Cache uniforms used by the preset
    json uniforms;
    for (const Uniform& uniform : korObject.m_uniforms) {
#ifdef DEBUG_MODE
        if (uniform.getId() == -1) {
            Logger::Throw("Error, invalid uniform");
        }
#endif
        const GStringView& uniformName = uniform.getName(*korObject.m_shaderProgram);
        uniforms[uniformName.c_str()] = uniform.asJson(*korObject.m_shaderProgram, korObject.m_uniformValues);
    }
    orJson["uniforms"] = uniforms;

    if (korObject.m_shaderProgram) {
        orJson["shaderProgram"] = korObject.m_shaderProgram->handle()->getName().c_str();
    }

    if (korObject.m_prepassShaderProgram) {
        orJson["prepassShaderProgram"] = korObject.m_prepassShaderProgram->handle()->getName().c_str();
    }
}

void from_json(const json& korJson, ShaderPreset& orObject)
{

    if (korJson.contains("renderSettings")) {
        korJson["renderSettings"].get_to(orObject.m_renderSettings);
    }

    if (korJson.contains("name")) {
        korJson["name"].get_to(orObject.m_name);
    }

    if (korJson.contains("shaderProgram")) {
        GString shaderProgramName = korJson["shaderProgram"].get_ref<const std::string&>().c_str();
        std::shared_ptr<ResourceHandle> handle = ResourceCache::Instance().getHandleWithName(
            shaderProgramName, EResourceType::eShaderProgram);
        if (!handle) {
            // FIXME: 2/9/2021 Noticed when switching scenarios, but not duplicable
            Logger::Throw("Error, handle to shader program " + shaderProgramName + " not found");
        }
        orObject.m_shaderProgram = handle->resourceAs<ShaderProgram>();
        if (!orObject.m_shaderProgram) {
            Logger::Throw("Error, shader program with name " + shaderProgramName + " not found");
        }
    }

    if (korJson.contains("prepassShaderProgram")) {
        GString shaderProgramName = korJson["prepassShaderProgram"].get_ref<const std::string&>().c_str();
        orObject.m_prepassShaderProgram = ResourceCache::Instance().getHandleWithName(
            shaderProgramName, EResourceType::eShaderProgram)->resourceAs<ShaderProgram>();
        if (!orObject.m_prepassShaderProgram) {
            Logger::Throw("Error, prepass shader program with name " + shaderProgramName + " not found");
        }
    }

    // Load uniforms used by the preset
    orObject.m_uniforms.clear();
    orObject.m_prepassUniformIds.clear();
    const json& uniforms = korJson["uniforms"];
    for (const auto& jsonPair : uniforms.items()) {
#ifdef DEBUG_MODE
        if (jsonPair.value().is_null()) {
            Logger::Throw("Error, uniform not found");
        }
        else if (!jsonPair.value().is_object()) {
            Logger::Throw("Error, uniform value is not an object");
        }
#endif
        const json& uniformObject = jsonPair.value();

        orObject.addUniform(Uniform::FromJson(uniformObject, *orObject.m_shaderProgram, orObject.m_uniformValues));
#ifdef DEBUG_MODE
        if (orObject.m_uniforms.back().getId() == -1) {
            Logger::Throw("Invalid uniform loaded");
        }
#endif
    }


    orObject.m_engine->scenario()->settings().onShaderPresetChanged(orObject.m_uuid);
}

std::vector<std::shared_ptr<ShaderPreset>> ShaderPreset::s_builtins;


// End namespaces
}