#pragma once

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

// Standard Includes
#include <map>
#include <string>
#include <assert.h>

// Library Includes
#include <boost/any.hpp>
#include <d3dx9math.h>
#include <angelscript/scriptany/scriptany.h>

// Local Includes
#include "../sharedbase/Envelope.h"
#include "../sharedbase/Entity.h"
#include "../sharedbase/ScriptObjectInterface.h"

// *HACK: This is indicating that we need to re-think how modules register their information with the script engine.
#include "../GraphicsDLL/component/RenderableComponent.h"

// Forward Declarations

// Typedefs

class CreateEntityFactory : public ScriptObjectInterface {
public:
	CreateEntityFactory (void) : ScriptObjectInterface() {
		this->entity.reset();
	}
	~CreateEntityFactory (void) {
	}

	void SetName(std::string name) {
		LOG(LOG_PRIORITY::INFO, "This method is depricated and does nothing.");
	}
	void AddAttribute(std::string componentType, std::string key, std::string value) {
		this->components[componentType][key] = value;
	}

	void AddAttribute(std::string componentType, std::string key, CScriptAny* value) {
		std::string type(engine->GetTypeDeclaration(value->GetTypeId()));
		boost::any val;
		
		if (type == "bool") {
			bool myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "int64") {
			long long myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type.compare(0, 3, "int") == 0) {
			int myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "uint64") {
			unsigned long long myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type.compare(0, 4, "uint") == 0) {
			unsigned int myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "float") {
			float myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "double") {
			double myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "string") {
			std::string myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "Rotation") {
			D3DXQUATERNION myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "Vector") {
			D3DXVECTOR3 myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "Color") {
			D3DXCOLOR myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "Entity") {
			EntitySPTR myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			EntitySPTR ent(myvalue);
			val = ent;
		}
		else if (type == "Envelope") {
			Envelope* myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "EnvelopeSPTR") {
			EnvelopeSPTR myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else if (type == "ProjectionStyle") {
			GraphicsCore::ECoordinateSpace myvalue;
			value->Retrieve(&myvalue, value->GetTypeId());
			val = myvalue;
		}
		else {
			assert(/*unknown type*/false);
		}
		
		this->components[componentType][key] = val;
	}

	void SetEntity(EntitySPTR entity) {
		this->entity = entity;
	}
	void CreateEntity(int id = 0) {
		this->entity = Entity::Factory("", id);
	}
	void Finalize(EnvelopeSPTR envelope) {
		if (this->entity.get() != nullptr) {
			EntitySPTR ent(this->entity);
			envelope->AddData(ent->GetName());
			envelope->AddData(ent);
			envelope->AddData(this->components);
		}
		else {
			LOG(LOG_PRIORITY::CONFIG, "Message being finalized with a null Entity!  Aborting...");
			assert(/* Missing Entity */this->entity.get() != nullptr);
		}
	}
	
	static asIScriptEngine* engine;
private:
	EntitySPTR entity;
	std::map< std::string, std::map<std::string, boost::any> > components;
};
