#include "core/rendering/shaders/GUniform.h"
#include "core/rendering/shaders/GShaderProgram.h"
#include "core/rendering/shaders/GUniformContainer.h"

namespace rev {

Uniform Uniform::FromJson(const nlohmann::json& json, ShaderProgram& shaderProgram, UniformContainer& container)
{
    Uniform uniform;
    uniform.fromJson(json, shaderProgram, container);
    return uniform;
}

Uniform::Uniform(const Uniform & other):
    m_id(other.m_id),
    m_data(other.m_data)
{
}

Uniform::Uniform(Uniform&& other):
    m_id(other.m_id),
    m_data(other.m_data)
{
}

Uniform::Uniform(Int32_t id, const UniformData& data):
    m_id(id),
    m_data(data)
{
}

Uniform::Uniform(Int32_t id, UniformData&& data):
    m_id(id),
    m_data(std::move(data))
{
}

Uniform::~Uniform()
{
}

QString Uniform::asString(const UniformContainer& uc) const
{
    QString string;
    switch (m_data.m_uniformType) {
    case ShaderVariableType::kBool:
        string = QString::number(uc.getUniformValue<bool>(m_data.m_storageIndex));
        break;
    case ShaderVariableType::kInt:
        string = QString::number(uc.getUniformValue<int>(m_data.m_storageIndex));
        break;
    case ShaderVariableType::kFloat:
        if (m_data.m_count < 0) {
            string = QString::number(uc.getUniformValue<Float32_t>(m_data.m_storageIndex));
        }
        else {
            string += "{";
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Float32_t& entry = uc.getUniformValue<Float32_t>(m_data.m_storageIndex + i);
                string += QString::number(entry) + ", ";
            }
            string += "}";
        }
        break;
    case ShaderVariableType::kVec2:
        string = std::string(uc.getUniformValue<Vector2>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kVec3:
        if (m_data.m_count < 0) {
            string = std::string(uc.getUniformValue<Vector3>(m_data.m_storageIndex)).c_str();
        }
        else {
            string += "{";
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Vector3& entry = uc.getUniformValue<Vector3>(m_data.m_storageIndex + i);
                string += (std::string(entry) + ", \n").c_str();
            }
            string += "}";
        }
        break;
    case ShaderVariableType::kVec4:
        if (m_data.m_count < 0) {
            string = std::string(uc.getUniformValue<Vector4>(m_data.m_storageIndex)).c_str();
        }
        else {
            string += "{";
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Vector4& entry = uc.getUniformValue<Vector4>(m_data.m_storageIndex + i);
                string += (std::string(entry) + ", \n").c_str();
            }
            string += "}";
        }
        break;
    case ShaderVariableType::kIVec2:
        string = std::string(uc.getUniformValue<Vector2i>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kIVec3:
        string = std::string(uc.getUniformValue<Vector3i>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kIVec4:
        string = std::string(uc.getUniformValue<Vector4i>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kUVec2:
        string = std::string(uc.getUniformValue<Vector2u>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kUVec3:
        string = std::string(uc.getUniformValue<Vector3u>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kUVec4:
        string = std::string(uc.getUniformValue<Vector4u>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kMat2:
        string = std::string(uc.getUniformValue<Matrix2x2g>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kMat3:
        string = std::string(uc.getUniformValue<Matrix3x3g>(m_data.m_storageIndex)).c_str();
        break;
    case ShaderVariableType::kMat4:
        if (m_data.m_count < 0) {
            string = std::string(uc.getUniformValue<Matrix4x4g>(m_data.m_storageIndex)).c_str();
        }
        else {
            string += "{";
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Matrix4x4& entry = uc.getUniformValue<Matrix4x4>(m_data.m_storageIndex + i);
                string += (std::string(entry) + ", \n").c_str();
            }
            string += "}";
        }
        break;
    default:
        GString err = "Error, this uniform to QString conversion is not supported";
        Logger::Throw(err);
    }

    return QString("Uniform") + ": " + string;
}

Uniform & Uniform::operator=(const Uniform & rhs)
{
    m_id = rhs.m_id;
    m_data = rhs.m_data;
    return *this;
}

Uniform& Uniform::operator=(Uniform&& rhs)
{
    m_id = rhs.m_id;
    m_data = rhs.m_data;
    return *this;
}

const GString& Uniform::getName(const ShaderProgram& shaderProgram) const
{
    return shaderProgram.getUniformName(m_id);
}

bool Uniform::matchesInfo(const ShaderInputInfo& typeInfo) const
{
    int inputType = (int)typeInfo.m_variableType;
    if (!ShaderInputInfo::IsValidGLType(inputType)) {
        return false;
    }

    //const std::type_index& type = s_uniformGLTypeMap.at(typeInfo.m_variableType);
    if (typeInfo.m_variableType == m_data.m_uniformType) {
        return true;
    }
    else {
        if (m_data.m_uniformType == ShaderVariableType::kInt &&
            (typeInfo.m_variableType == ShaderVariableType::kSampler2D ||
            typeInfo.m_variableType == ShaderVariableType::kSamplerCube)
            ) 
        {
            return true;
        }
        else {
            return false;
        }
    }

}

json Uniform::asJson(const ShaderProgram& shaderProgram, const UniformContainer& uc) const
{
    json orJson;
    orJson["name"] = shaderProgram.getUniformName(m_id).c_str();
    orJson["value"] = valueAsJson(uc);
    orJson["id"] = m_id;
    return orJson;
}

void Uniform::fromJson(const nlohmann::json& korJson, ShaderProgram& shaderProgram, UniformContainer& container)
{    
    /// @todo Don't serialize uniforms, should always be a member or otherwise
    /// @see https://stackoverflow.com/questions/4484982/how-to-convert-typename-t-to-string-in-c
    if (korJson.contains("name")) {
        // Parse json if name and value are separate key-value pairs
        /// @note The name is no longer set here, since a string-view requires a persistent string
        if (korJson.contains("id")) {
            m_id = korJson["id"].get<Int32_t>();
        }
        else {
            GString uniformName = korJson["name"].get_ref<const std::string&>();
            m_id = shaderProgram.getUniformId(uniformName);
        }
        loadValue(container, korJson.at("value"));
    }
    else {
#ifdef DEBUG_MODE
        // Incorrect JSON format
        Logger::Throw("Error, uniform JSON format is not recognized");
#else
        Logger::LogError("Error, uniform JSON format is not recognized");
#endif
    }
}

json Uniform::valueAsJson(const UniformContainer & uc) const
{
    json oJson;
    switch (m_data.m_uniformType) {
    case ShaderVariableType::kInt:
        oJson = uc.getUniformValue<int>(m_data.m_storageIndex);
        break;
    case ShaderVariableType::kBool:
        oJson = uc.getUniformValue<bool>(m_data.m_storageIndex);
        break;
    case ShaderVariableType::kFloat:
        if (m_data.m_count < 0) {
            oJson = uc.getUniformValue<Float32_t>(m_data.m_storageIndex);
        }
        else {
            oJson = json::array();
            for (size_t i = 0; i < m_data.m_count; i++) {
                Float32_t entry = uc.getUniformValue<Float32_t>(m_data.m_storageIndex + i);
                oJson.push_back(entry);
            }
        }
        break;
    case ShaderVariableType::kVec2:
        oJson = uc.getUniformValue<Vector2>(m_data.m_storageIndex);
        break;
    case ShaderVariableType::kVec3:
        if (m_data.m_count < 0) {
            oJson = uc.getUniformValue<Vector3>(m_data.m_storageIndex);
        }
        else {
            oJson = json::array();
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Vector3& entry = uc.getUniformValue<Vector3>(m_data.m_storageIndex + i);
                oJson.push_back(entry);
            }
        }
        break;
    case ShaderVariableType::kVec4:
        if(m_data.m_count < 0) {
            oJson = uc.getUniformValue<Vector4>(m_data.m_storageIndex);
        }
        else {
            oJson = json::array();
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Vector4& entry = uc.getUniformValue<Vector4>(m_data.m_storageIndex + i);
                oJson.push_back(entry);
            }
        }
        break;
    case ShaderVariableType::kMat2:
        oJson = uc.getUniformValue<Matrix2x2g>(m_data.m_storageIndex);
        break;
    case ShaderVariableType::kMat3:
        oJson = uc.getUniformValue<Matrix3x3g>(m_data.m_storageIndex);
        break;
    case ShaderVariableType::kMat4:
        if (m_data.m_count < 0) {
            oJson = uc.getUniformValue<Matrix4x4g>(m_data.m_storageIndex);
        }
        else {
            oJson = json::array();
            for (size_t i = 0; i < m_data.m_count; i++) {
                const Matrix4x4& entry = uc.getUniformValue<Matrix4x4>(m_data.m_storageIndex + i);
                oJson.push_back(entry);
            }
        }
        break;
    default:
        GString err = "Error, this uniform to QVariant conversion is not supported";
        Logger::Throw(err);
    }

    return oJson;
}


void Uniform::loadValue(UniformContainer& uc, const nlohmann::json& korJson)
{
    // FIXME: Do this more elegantly
    json::value_t type = korJson.type();
    if (type == json::value_t::boolean) {
        uc.addUniformValue(korJson.get<bool>(), m_data);
    }
    else if (type == json::value_t::number_integer || type == json::value_t::number_unsigned) {
        uc.addUniformValue(korJson.get<int>(), m_data);
    }
    else if (type == json::value_t::number_float) {
        Float32_t val = korJson.get<Float32_t>();
        uc.addUniformValue(val, m_data);
    }
    else if (type == json::value_t::array){
        // Determine type from form of JSON
        if (korJson.size() == 0) {
            Logger::Throw("Error, empty array should not be stored in JSON");
        }
        else if (korJson.at(0).is_string()) {
            // Is a matrix type
            int columnSize = korJson.at(1).size();
            if (columnSize == 4) {
                uc.addUniformValue(Matrix4x4g(korJson), m_data);
            }
            else if (columnSize == 3) {
                uc.addUniformValue(Matrix3x3g(korJson), m_data);
            }
            else if (columnSize == 2) {
                uc.addUniformValue(Matrix2x2g(korJson), m_data);
            }
            else {
                Logger::Throw("Error, matrix of this size is not JSON serializable");
            }
        }
        else if(korJson.at(0).is_array()){
            if (korJson.at(0).at(0).is_string()) {
                // Is a vector of Matrix type
                int columnSize = korJson.at(0).at(1).size();
                if (columnSize == 4) {
                    uc.addUniformValue(korJson.get<std::vector<Matrix4x4g>>(), m_data);
                }
            }
            else {
                // Is a std::vec of Vector type
                if (korJson.at(0).size() == 3) {
                    uc.addUniformValue(korJson.get<Vec3List>(), m_data);
                }
                else if(korJson.at(0).size() == 4) {
                    uc.addUniformValue(korJson.get<Vec4List>(), m_data);
                }
                else {
                    Logger::Throw("Error, std:vec of Vector of this size is not JSON serializable");
                }
            }
        }
        else {
            
            // Is a vector type
            if (korJson.size() > 4) {
                uc.addUniformValue(korJson.get<std::vector<Float32_t>>(), m_data);
            }
            else if (korJson.size() == 4) {
                uc.addUniformValue(Vector4(korJson), m_data);
            }
            else if (korJson.size() == 3) {
                uc.addUniformValue(Vector3(korJson), m_data);
            }
            else if (korJson.size() == 2) {
                uc.addUniformValue(Vector2(korJson), m_data);
            }
        }
    }
    else {
        Logger::Throw("Error, unsupported JSON");
    }
}

tsl::robin_map<ShaderVariableType, std::type_index> Uniform::s_uniformGLTypeMap = {
    {ShaderVariableType::kBool, typeid(bool)},
    {ShaderVariableType::kInt, typeid(int)},
    {ShaderVariableType::kFloat, typeid(float)},
    {ShaderVariableType::kDouble, typeid(double)},
    {ShaderVariableType::kUVec2, typeid(Vector<unsigned int, 2>)},
    {ShaderVariableType::kUVec3, typeid(Vector<unsigned int, 3>)},
    {ShaderVariableType::kUVec4, typeid(Vector<unsigned int, 4>)},
    {ShaderVariableType::kIVec2, typeid(Vector<int, 2>)},
    {ShaderVariableType::kIVec3, typeid(Vector<int, 3>)},
    {ShaderVariableType::kIVec4, typeid(Vector<int, 4>)},
    {ShaderVariableType::kVec2, typeid(Vector2)},
    {ShaderVariableType::kVec3, typeid(Vector3)},
    {ShaderVariableType::kVec4, typeid(Vector4)},
    {ShaderVariableType::kMat2, typeid(Matrix2x2g)},
    {ShaderVariableType::kMat3, typeid(Matrix3x3g)},
    {ShaderVariableType::kMat4, typeid(Matrix4x4g)},
    {ShaderVariableType::kSamplerCube, typeid(int)},
    {ShaderVariableType::kSamplerCubeArray, typeid(int)},
    {ShaderVariableType::kSampler2D, typeid(int)},
    {ShaderVariableType::kSampler2DShadow, typeid(int)},
    {ShaderVariableType::kSampler2DArray, typeid(int)}
};

tsl::robin_map<QString, ShaderVariableType> Uniform::s_uniformTypeStrMap = {
    {"bool",   ShaderVariableType::kBool},
    {"int",    ShaderVariableType::kInt},
    {"float",  ShaderVariableType::kFloat},
    {"double", ShaderVariableType::kDouble},
    {"ivec2",   ShaderVariableType::kIVec2},
    {"ivec3",   ShaderVariableType::kIVec3},
    {"ivec4",   ShaderVariableType::kIVec4},
    {"uvec2",   ShaderVariableType::kUVec2},
    {"uvec3",   ShaderVariableType::kUVec3},
    {"uvec4",   ShaderVariableType::kUVec4},
    {"vec2",   ShaderVariableType::kVec2},
    {"vec3",   ShaderVariableType::kVec3},
    {"vec4",   ShaderVariableType::kVec4},
    {"mat2",   ShaderVariableType::kMat2},
    {"mat3",   ShaderVariableType::kMat3},
    {"mat4",   ShaderVariableType::kMat4},
    {"samplerCube", ShaderVariableType::kSamplerCube},
    {"samplerCubeArray", ShaderVariableType::kSamplerCubeArray},
    {"sampler2D", ShaderVariableType::kSampler2D},
    {"sampler2DShadow", ShaderVariableType::kSampler2DShadow},
    {"sampler2DArray", ShaderVariableType::kSampler2DArray}
};


tsl::robin_map<ShaderVariableType, QString> Uniform::s_uniformStrTypeMap = {
    {ShaderVariableType::kBool,   "bool"},
    {ShaderVariableType::kInt,    "int"},
    {ShaderVariableType::kFloat,  "float"},
    {ShaderVariableType::kDouble, "double"},
    {ShaderVariableType::kUVec2,   "uvec2"},
    {ShaderVariableType::kUVec3,   "uvec3"},
    {ShaderVariableType::kUVec4,   "uvec4"},
    {ShaderVariableType::kIVec2,   "ivec2"},
    {ShaderVariableType::kIVec3,   "ivec3"},
    {ShaderVariableType::kIVec4,   "ivec4"},
    {ShaderVariableType::kVec2,   "vec2"},
    {ShaderVariableType::kVec3,   "vec3"},
    {ShaderVariableType::kVec4,   "vec4"},
    {ShaderVariableType::kMat2,   "mat2"},
    {ShaderVariableType::kMat3,   "mat3"},
    {ShaderVariableType::kMat4,   "mat4"},
    {ShaderVariableType::kSamplerCube, "samplerCube"},
    {ShaderVariableType::kSamplerCubeArray, "samplerCubeArray"},
    {ShaderVariableType::kSampler2D,   "sampler2D"},
    {ShaderVariableType::kSampler2DShadow,   "sampler2DShadow"},
    {ShaderVariableType::kSampler2DArray,   "sampler2DArray"}
};


// End namespacing
}