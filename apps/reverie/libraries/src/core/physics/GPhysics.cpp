#include "core/physics/GPhysics.h"

namespace rev {

PhysicsBase::PhysicsBase()
{
}

PhysicsBase::PhysicsBase(const GString & name) :
    NameableInterface(name)
{
}

PhysicsBase::~PhysicsBase()
{
}

void to_json(json& orJson, const PhysicsBase& korObject)
{
    orJson[JsonKeys::s_name] = korObject.m_name.c_str();
}

void from_json(const json& korJson, PhysicsBase& orObject)
{
    orObject.m_name = korJson.at("name").get_ref<const std::string&>().c_str();
}




} // End namespace