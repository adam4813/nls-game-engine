/**
* \file
* \author	Adam Martin
* \date		2011-07-20
* \brief	A single object to represent everything inside the world.
* \details	A entity is a link between different components that make it up.
* Anything inside the world is represented by an entity and its components.
* Entity's contain an id and location, rotation, scale that define the most
* common attributes of everything in the world.
*
*/
#pragma once

// System Library Includes
#include <set>
#include <string>

// Application Library Includes
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// Local Includes
#include "Entity_fwd.h"

// Forward Declarations
class ComponentInterface;
class asIScriptEngine;

// Typedefs

/**
* \brief An in-game object representing the base point in space it exists at.
*/
class Entity {
	friend class ComponentInterface;
	
public:
	/**
	* @name Factory Methods
	* \brief Various Factory methods to create an Entity. The actual ctor is private to ensure the use
	* of the factory methods that return an EntitySPTR.
	*/
	/**@{*/
	/**
	* \brief The standard factory.
	*/
	static EntitySPTR Factory(const std::string& = "");

	/**
	* \brief The placement factory.
	*/
	static void FactoryAtAddress(void*);

	/**
	* \brief A variant placement factory.
	*/
	static void FactoryAtAddress(void*, const std::string&);
	/**@}*/

private:
	/**
	* \brief Must use a factory methods to create an instance.
	*/
	Entity(const std::string& = "");
public:
	/**
	* \brief Removes all components from the entity.
	*/
	~Entity();
	
	/**
	* \brief Registers entity to Angelscript.
	*/
	static void Register(asIScriptEngine* const);

	/**
	* \brief Sets the entity's parent.
	*/
	void SetParent(EntitySPTR newParent);

	/**
	* \brief Gets the entity's parent
	*/
	EntitySPTR GetParent(void) const;
	
	/**
	* @name World Positional methods
	* \brief Returns the world position, rotation, or scale relative to the parent.
	*/
	/**@{*/
	/**
	* \brief Get the absolute position of a given object, recursively accounting for all parent positions, rotations, and scales.
	*/
	glm::vec3 GetWorldPosition(void) const;

	/**
	* \brief Get the absolute rotation of a given object, recursively accounting for all parent rotations.
	*/
	glm::fquat GetWorldRotation(void) const;
	
	/**
	* Get the absolute scale of a given object, recursively accounting for all parent scales.
	*/
	float GetWorldScale(void) const;
	/**@}*/
	
	/**
	* \brief Sets the position of the entity (relative to the parent).
	*/
	void SetPosition(float, float, float);

	/**
	* \brief Rotate this entity, in its parent entity's coordinate space, by rot.
	*/
	void SetRotation(glm::fquat);
	
	/**
	* \brief Rotate this entity, in its parent entity's coordinate space, by yaw, pitch, and roll in radians.
	*/
	void SetRotation(float, float, float);

	/**
	* \brief Sets the entity's scale.
	*/
	void SetScale(float);

	/**
	* @name Relative Positional methods
	* \brief Changes the psoition, rotation, or scale relative to the current entity element respectively, by delta.
	*/
	/**@{*/
	/**
	* \brief Offset this entity, relative to its current location and in its parent entity's coordinate space, by delta.
	*/
	void ChangePosition(glm::vec3);
	
	/**
	* \brief Rotate this entity, relative to its current rotation and in its parent entity's coordinate space, by delta.
	*/
	void ChangeRotation(glm::fquat);
	
	/**
	* \brief Rotate this entity, relative to its current rotation and in its parent entity's coordinate space, by delta yaw, pitch, and roll in radians.
	*/
	void ChangeRotation(float, float, float);
	
	/**
	* \brief Scale this entity, relative to its current scale, by delta.
	*/
	void ChangeScale(float);
	/**@}*/
			
	///**
	//* \brief Set the entity's name.
	//*/
	//void SetName(const std::string& name);
		
	/**
	* \brief Get the entity's name.
	*/
	const std::string& GetName() const;
			
	/**
	* \brief Removes all components from the entity's set.
	*/
	void ClearComponents();
	
protected: // Friend access
	/**
	* \brief Add a component to the entity's set.
	*/
	bool RegisterComponent(ComponentInterface*);

	/**
	* \brief Remove a component from the entity's set.
	*/
	bool UnregisterComponent(ComponentInterface*);
	
	/**
	* \brief Notification that the passed in entity is being removed.
	*/
	bool NotifyEntityRemoval(EntitySPTR);
public:
	glm::vec3 location; /**< Offset relative to parent entity space. */
	glm::fquat rotation; /**< Rotation relative to parent. */
	float scale; /**< Scale relative to parent. */
private:
	std::string name; /**< The name of this entity. */
	
	//mutable Threading::ReadWriteMutex parentMutex; /**< Parent mutex lock for changing parent. */
	EntitySPTR parent; /**< Parent entity */
	
	//mutable Threading::ReadWriteMutex componentsMutex; /**< Component mutex lock for changing components. */
	std::set<ComponentInterface*> components; /**< Components that are parented to this entity.  Not designed to be the primary storage of the relationship - that is maintained by the components themselves. */
};
