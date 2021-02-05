/**
* \file sptrtypes
* \author Ricky Curtice
* \date 2012-02-26
* \brief A collection of functions that make registering the shared_ptr types much easier
*
*/
#pragma once

// Standard Includes
#include <cassert>

// Library Includes
#include <angelscript/sptrwrapper/as_smart_ptr_wrapper.h>

// Local Includes
#include "../sharedbase/Entity.h"

// Forward Declarations

// Typedefs

// Functions

// Helper functions

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <typename T>
void construct(void* address) {
  new (address) T();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <typename T>
void destroy(T* object) {
  object->~T();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <typename T>
void copy_construct(void* address, T* other) {
  new (address) T(*other);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <typename T>
void assign(T * lhs, T* rhs) {
  *lhs = *rhs;
}


// Registration functions

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void RegisterEntity(asIScriptEngine* const engine) {
	int ret = 0;
	
	// *NOTE: Works, just there's an issue:
	// In current scripts, Entity ent; creates an object.
	// With this it creates a shared_ptr with a value pointed at garbage.  I'd like it to point at nullptr, but that's another issue.
	// This problem means that scripts need to add an extra set of parens to call the ctor: Entity ent();
	// This breaks backwards compat.
	
	// Register Object
	ret = engine->RegisterObjectType("Entity", sizeof(EntitySPTR), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(ret >= 0);
	
	// Register behaviors and operations
	ret = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT,  "void f()",                                asFUNCTIONPR(Entity::FactoryAtAddress,(void*), void),   asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT,  "void f(const string& in)",                asFUNCTIONPR(Entity::FactoryAtAddress,(void*, const std::string&), void),   asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT,  "void f(const string& in, const int& in)", asFUNCTIONPR(Entity::FactoryAtAddress,(void*, const std::string&, const int&), void),   asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Entity", asBEHAVE_CONSTRUCT,  "void f(const Entity& in)", asFUNCTION(copy_construct<EntitySPTR>), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectBehaviour("Entity", asBEHAVE_DESTRUCT,   "void f()",                 asFUNCTION(destroy<EntitySPTR>),        asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "Entity &opAssign(const Entity &in)", asFUNCTION(assign<EntitySPTR>), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	// Register properties
	ret = engine->RegisterObjectMethod("Entity", "const int& get_id() const",  REF_GETTER(Entity, id), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void set_id(const int& in)", REF_SETTER(Entity, id), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "PropertyMap& get_properties() const",  REF_GETTER(Entity, properties), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void set_properties(const PropertyMap& in)", REF_SETTER(Entity, properties), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "float& get_scale() const",  REF_GETTER(Entity, scale), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void set_scale(const float& in)", REF_SETTER(Entity, scale), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "Vector& get_positionOffset() const",  REF_GETTER(Entity, location), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void set_positionOffset(const Vector& in)", REF_SETTER(Entity, location), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "Rotation& get_rotationOffset() const",  REF_GETTER(Entity, rotation), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void set_rotationOffset(const Rotation& in)", REF_SETTER(Entity, rotation), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	// Register methods
	ret = engine->RegisterObjectMethod("Entity", "void SetParent(Entity)", CALLER(Entity, SetParent),     asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "Entity GetParent()",     CALLER(Entity, GetParent),     asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "void ChangeScale(float)", CALLER(Entity, ChangeScale),   asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "float GetWorldScale()",   CALLER(Entity, GetWorldScale), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "Vector GetWorldPosition()", CALLER(Entity, GetWorldPosition), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void SetWorldPosition(float, float, float)", CALLER(Entity, SetWorldPosition), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void ChangePosition(Vector)", CALLER(Entity, ChangePosition), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	
	ret = engine->RegisterObjectMethod("Entity", "Rotation GetWorldRotation()", CALLER(Entity, GetWorldRotation), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void SetRotation(float, float, float)", CALLER_PR(Entity, SetRotation, (float, float, float), void), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
	ret = engine->RegisterObjectMethod("Entity", "void ChangeRotation(float, float, float)", CALLER_PR(Entity, ChangeRotation, (float, float, float), void), asCALL_CDECL_OBJFIRST); assert(ret >= 0);
}
