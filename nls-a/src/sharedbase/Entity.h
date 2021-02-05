/**
* \file Entity
* \author Adam Martin
* \date 2011-07-20
* \brief A single object to represent everything inside the world.
*
* A entity is a link between different components that make it up.
* Anything inside the world is represented by an entity and its components.
* Entity's contain an id and location, rotation, scale that define the most
* common attributes of everything in the world.
*
*/
#pragma once

// Standard Includes
#include <memory>
#include <set>

// Library Includes
#include <d3dx9math.h>
#include <boost/any.hpp>

// Local Includes
#include "QuatMath.h"
#include "PropertyMap.h"
#include "Entity_fwd.h"

// Forward Declarations
class ComponentInterface;

// Typedefs
typedef std::shared_ptr<std::string> MutableStringSPTR;


class Entity {
	friend class ComponentInterface;
	friend class EntityList;
	
public:
	static EntitySPTR Factory(const std::string& = "", const int& = -1);
	static void FactoryAtAddress(void*);
	static void FactoryAtAddress(void*, const std::string&);
	static void FactoryAtAddress(void*, const std::string&, const int&);
	~Entity();
	
	void SetParent(EntitySPTR newParent);
	EntitySPTR GetParent(void) const;
	
	int GetId() const;

	/**
	* Get the absolute position of a given object, recursively accounting for all parent positions, rotations, and scales.
	*/
	D3DXVECTOR3 GetWorldPosition(void) const;
	
	void SetWorldPosition(float, float, float);

	/**
	* Get the absolute rotation of a given object, recursively accounting for all parent rotations.
	*/
	D3DXQUATERNION GetWorldRotation(void) const;
	
	/**
	* Get the absolute scale of a given object, recursively accounting for all parent scales.
	*/
	float GetWorldScale(void) const;
	
	/**
	* Rotate this entity, in its parent entity's coordinate space, by rot.
	*/
	void SetRotation(D3DXQUATERNION);
	
	/**
	* Rotate this entity, in its parent entity's coordinate space, by yaw, pitch, and roll in radians.
	*/
	void SetRotation(float, float, float);
	
	/**
	* Offset this entity, relative to its current location and in its parent entity's coordinate space, by delta.
	*/
	void ChangePosition(D3DXVECTOR3);
	
	/**
	* Rotate this entity, relative to its current rotation and in its parent entity's coordinate space, by delta.
	*/
	void ChangeRotation(D3DXQUATERNION);
	
	/**
	* Rotate this entity, relative to its current rotation and in its parent entity's coordinate space, by delta yaw, pitch, and roll in radians.
	*/
	void ChangeRotation(float, float, float);
	
	/**
	* Scale this entity, relative to its current scale, by delta.
	*/
	void ChangeScale(float);
	
	void SetName(const std::string& name);
	
	std::string GetName();
	
	MutableStringSPTR GetNameSPTR();
	
protected: // Friend access
	void RegisterComponent(ComponentInterface*);
	void UnregisterComponent(ComponentInterface*);
	
	bool NotifyEntityRemoval(EntitySPTR);
	
private:
	Entity(const std::string& = "", const int& = -1);
	
	void ClearComponents();
	
public:
	D3DXVECTOR3 location; ///< Offset relative to parent entity space.
	D3DXQUATERNION rotation; ///< Rotation relative to parent
	float scale; ///< Scale relative to parent
	int id;
	int priority;
	PropertyMap properties;
private:
	MutableStringSPTR name;
	
	mutable Threading::ReadWriteMutex parentMutex;
	EntitySPTR parent; ///< Parent entity
	
	mutable Threading::ReadWriteMutex componentsMutex;
	std::set<ComponentInterface*> components; ///< Components that are parented to this entity.  Not designed to be the primary storage of the relationship - that is maintained by the components themselves.
};
