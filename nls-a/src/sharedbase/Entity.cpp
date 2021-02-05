/**
* \file Entity
* \author Ricky Curtice
* \date 2011-12-28
* \brief A single object to represent everything inside the world.
*
* A entity is a link between different components that make it up.
* Anything inside the world is represented by an entity and its components.
* Entity's contain an id and location, rotation, scale that define the most
* common attributes of everything in the world.
*
*/

#include "Entity.h"

// Standard Includes

// Library Includes
#include <d3dx9math.h>
#include <boost/any.hpp>
#include <boost/foreach.hpp>

// Local Includes
#include "QuatMath.h"
#include "ComponentInterface.h"
#include "ModuleInterface.h"

// Forward Declarations

// Typedefs

// Methods
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
EntitySPTR Entity::Factory(const std::string& name, const int& id) {
	EntitySPTR entity(new Entity(name, id));
	return entity;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/// Creates a new Entity at the specified address.
void Entity::FactoryAtAddress(void* address) {
	Entity::FactoryAtAddress(address, "", -1);
}

void Entity::FactoryAtAddress(void* address, const std::string& name) {
	Entity::FactoryAtAddress(address, name, -1);
}

void Entity::FactoryAtAddress(void* address, const std::string& name, const int& id) {
	new (address) EntitySPTR(new Entity(name, id));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
Entity::Entity(const std::string& name, const int& id) :
	location(D3DXVECTOR3(0.0f, 0.0f, 0.0f)),
	rotation(D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f)),
	scale(1.0f),
	id(id),
	priority(0),
	name(new std::string(name)),
	parent(nullptr)
	{
	LOG(LOG_PRIORITY::FLOW, "Entity '" + this->GetName() + "' created.");
}

Entity::~Entity() {
	LOG(LOG_PRIORITY::FLOW, "Entity '" + this->GetName() + "' destroyed.");
	
	this->ClearComponents();
	
	// Break from parent object.
	this->SetParent(nullptr);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Entity::SetParent(EntitySPTR new_parent) {
	Threading::WriteLock w_lock(this->parentMutex);
	
	if (new_parent.get() != nullptr) {
		// Verify that a recursive relationship has not been established
		bool status = true;
		Entity* new_parent_p = new_parent.get();
		Entity* parent = this->parent.get();
		
		// Linearized recursive scan up the family tree.
		while (parent != nullptr && (status = (parent != new_parent_p))) {
			parent = parent->GetParent().get();
		}
		
		if (status) {
			this->parent = new_parent;
		}
		else {
			LOG(LOG_PRIORITY::CONFIG, "ERROR: Recursive parenting NOT allowed.  Attempted to parent '" + this->GetName() + "' to '" + new_parent->GetName() + "'.");
		}
	}
	else {
		this->parent.reset();
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
EntitySPTR Entity::GetParent(void) const {
	Threading::ReadLock r_lock(this->parentMutex);
	
	EntitySPTR ent(this->parent);
	
	return ent;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int Entity::GetId() const {
	return this->id;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Get the absolute position of a given object, recursively accounting for all parent positions, rotations, and scales.
*/
D3DXVECTOR3 Entity::GetWorldPosition(void) const {
	D3DXVECTOR3 result = this->location;
	Entity* entity = this->GetParent().get();
	D3DXVECTOR3 scaled, rotated;
	
	
	/* N=3
	push
		[n-2].translate
		[n-2].scale
		[n-2].rotate
		push
			[n-1].translate
			[n-1].scale
			[n-1].rotate
			push
				[n-0].translate
			pop
		pop
	pop
	
	(([n-0].translate) * [n-1].scale * [n-1].rotate + [n-1].translate) * [n-2].scale * [n-2].rotate + [n-2].translate
	*/
	
	while (entity != nullptr) {
		//result = (result * entity->scale) * entity->rotation + entity->location;
		
		scaled = (result * entity->scale);
		
		RotateVectorByQuaternion(&rotated, &scaled, &(entity->rotation));
		
		result = rotated + entity->location;
		
		entity = entity->GetParent().get();
	}
	
	return result;
}

void Entity::SetWorldPosition(float x, float y, float z) {
	this->location = D3DXVECTOR3(x, y, z);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Get the absolute rotation of a given object, recursively accounting for all parent rotations.
*/
D3DXQUATERNION Entity::GetWorldRotation(void) const {
	D3DXQUATERNION result = this->rotation;
	Entity* entity = this->GetParent().get();
	
	// Linearized recursive accumulation of rotations.
	while (entity != nullptr) {
		result = entity->rotation * result; // Remember, quat multiplication is NOT commutative!
		
		entity = entity->GetParent().get();
	}
	
	return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Get the absolute scale of a given object, recursively accounting for all parent scales.
*/
float Entity::GetWorldScale(void) const {
	float result = this->scale;
	Entity* entity = this->GetParent().get();
	
	// Linearized recursive accumulation of scales.
	while (entity != nullptr) {
		result *= entity->scale;
		
		entity = entity->GetParent().get();
	}
	
	return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Rotate this entity, in its parent entity's coordinate space, by rot.
*/
void Entity::SetRotation(D3DXQUATERNION rot) {
	D3DXQuaternionNormalize(&rot, &rot);
	this->rotation = rot;
	// *NOTE: If the normalize's sqrt call needs to be optimized away, there is a way (supposedly) to normalize quats without sqrt.
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Rotate this entity, in its parent entity's coordinate space, by yaw, pitch, and roll in radians.
*/
void Entity::SetRotation(float yaw, float pitch, float roll) {
	D3DXQUATERNION rot;
	D3DXQuaternionRotationYawPitchRoll(&rot, yaw, pitch, roll);
	this->SetRotation(rot);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Offset this entity, relative to its current location and in its parent entity's coordinate space, by delta.
*/
void Entity::ChangePosition(D3DXVECTOR3 delta) {
	this->location += delta;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Rotate this entity, relative to its current rotation and in its parent entity's coordinate space, by delta.
*/
void Entity::ChangeRotation(D3DXQUATERNION delta) {
	D3DXQuaternionNormalize(&delta, &delta);
	this->rotation *= delta;
	// *NOTE: If the normalize's sqrt call needs to be optimized away, there is a way (supposedly) to normalize quats without sqrt.
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Rotate this entity, relative to its current rotation and in its parent entity's coordinate space, by delta yaw, pitch, and roll in radians.
*/
void Entity::ChangeRotation(float deltaYaw, float deltaPitch, float deltaRoll) {
	D3DXQUATERNION delta;
	D3DXQuaternionRotationYawPitchRoll(&delta, deltaYaw, deltaPitch, deltaRoll);
	this->ChangeRotation(delta);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
* Scale this entity, relative to its current scale, by delta.
*/
void Entity::ChangeScale(float delta) {
	this->scale *= delta;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Entity::RegisterComponent(ComponentInterface* component) {
	Threading::WriteLock w_lock(this->componentsMutex);
	
	this->components.insert(component);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Entity::UnregisterComponent(ComponentInterface* component) {
	Threading::WriteLock w_lock(this->componentsMutex);
	
	this->components.erase(component);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool Entity::NotifyEntityRemoval(EntitySPTR entity) {
	if (entity == this->GetParent()) {
		this->SetParent(nullptr);
		
		return true;
	}
	
	return false;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Entity::SetName(const std::string& name) {
	if (this->name.get() != nullptr) {
		LOG(LOG_PRIORITY::FLOW, "Entity '" + this->GetName() + "' renamed to '" + name + "'.");
		
		// Mutate the string to contain the new value
		this->name->erase(0, this->name->length());
		this->name->insert(0, name);
	}
	else {
		LOG(LOG_PRIORITY::FLOW, "Entity given name '" + name + "'. Pretty sure this is impossible because of the way the factories are set up.  Let me know if you see this in the log! ~Ricky");
		
		this->name.reset(new std::string(name));
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
std::string Entity::GetName() {
	if (this->name != nullptr) {
		return *this->name;
	}
	
	return "";
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
MutableStringSPTR Entity::GetNameSPTR() {
	return this->name;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void  Entity::ClearComponents() {
	static std::set<ComponentInterface*> components;
	static ModuleInterface* module;
	
	{
		Threading::ReadLock r_lock(this->componentsMutex);
		components = this->components;
	}
	
	// Remove the child components - each will unregister itself upon dtor.
	BOOST_FOREACH(ComponentInterface* component, components) {
		module = component->GetModule();
		if (module->RemoveComponent(component) == WHO_DELETES::CALLER) {
			delete component;
		}
	}
}
