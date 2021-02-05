/**
* \file file base name
* \author Adam Martin
* \date 2011-07-27
* \brief A factory for creating a CreateEntity message within Angelscript
*
* A full factory for creating a CreateEntity message inside Angelscript.
* Functions are provided for creating the components and assigning their attributes.
* Finally a Finalize() function allows for an envelope to be created for use in
* actually created the entity within the EngineCoreModule
*
*/

#include "EntityFactory.h"

// Standard Includes

// Library Includes

// Local Includes

// Static class member initialization

// Class methods in the order they are defined within the class header
asIScriptEngine* CreateEntityFactory::engine;