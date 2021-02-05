#pragma once

/**
* \file file base name
* \author Adam Martin
* \date 2011-07-21
* \brief Manager for all entities in existence
*
* Manages the lifetime, and finding of entities that currently exist.
*
*/

// Standard Includes
#include <map>
#include <string>

// Library Includes
#include <boost/foreach.hpp>
#include <threading.h>

// Local Includes
#include "EventLogger.h"
#include "Entity.h"

// Forward Declarations

// Typedefs
struct MutableStringSPTRCompare {
	bool operator() (const MutableStringSPTR& lhs, const MutableStringSPTR& rhs) const {
		return *lhs < *rhs;
	}
};

typedef std::map<MutableStringSPTR, EntitySPTR, MutableStringSPTRCompare> NamedEntityMap;
typedef NamedEntityMap::value_type NamedEntity;


class EntityList {
public:
	EntityList(void) : debug_output(true) { }
	~EntityList(void) {
	}
	
	bool AddEntity(EntitySPTR const entity) {
		return AddEntity("", entity);
	}
	
	bool AddEntity(const std::string name, EntitySPTR const entity) {
		if (entity.get() != nullptr) {
			Threading::UpgradeLock up_lock(this->mutex);
			
			// Don't change the name if the entity already has a name.
			{
				MutableStringSPTR ent_name(entity->GetNameSPTR());
				if (ent_name.get() == nullptr || ent_name->length() <= 0) {
					// Make sure there's a name to use.
					if (name.length() > 0) {
						entity->SetName(name);
					}
					else {
						LOG(LOG_PRIORITY::CONFIG, "ERROR: Entity must have a name to be added!");
						return false;
					}
				}
				else if (name.length() > 0) {
					LOG(LOG_PRIORITY::INFO, "WARNING: Attempt to rename Entity '" + *ent_name.get() + "' to '" + name + "' during addition to EntList was ignored!");
				}
				// else: a name of "" and a fully named entity is a valid operation.
			}
			
			MutableStringSPTR name_sptr(entity->GetNameSPTR());
			
			if (this->entities.find(name_sptr) == this->entities.end()) {
				Threading::UpgradeToUniqueLock unique_lock(up_lock);
				
				EntitySPTR ent(entity);
				this->entities.insert(NamedEntity(name_sptr, ent));
				
				LOG2(LOG_PRIORITY::FLOW, "Adding entity: '" + *name_sptr.get() + "'");
				return true;
			}
			else {
				LOG(LOG_PRIORITY::CONFIG, "ERROR: Entity '" + *name_sptr.get() + "', already exists.");
				return false;
			}
		}
		else {
			LOG(LOG_PRIORITY::CONFIG, "ERROR: Adding a null entity is an invalid operation.  Attempted to add with the name '" + name + "'");
			
			return false;
		}
	}
	
	EntitySPTR FindEntity(const std::string name) const {
		Threading::ReadLock r_lock(this->mutex);
		
		MutableStringSPTR name_sptr(new std::string(name));
		
		NamedEntityMap::const_iterator entitymap_it = this->entities.find(name_sptr);
		if (entitymap_it != this->entities.end()) {
			EntitySPTR found_entity((*entitymap_it).second);
			
			return found_entity;
		}
		else {
			LOG2(LOG_PRIORITY::CONFIG, "Entity '" + name +  "' not found.  Did you forget to name it?  Or have you already removed it?");
			return nullptr;
		}
	}
	
	std::string FindName(EntitySPTR const entity) const {
		if (entity.get() != nullptr) {
			return entity->GetName();
		}
		else {
			return "";
		}
	}
	
	bool RemoveEntity(const std::string name) {
		Threading::WriteLock w_lock(this->mutex);
		
		MutableStringSPTR name_sptr(new std::string(name));
		
		NamedEntityMap::const_iterator entitymap_it = this->entities.find(name_sptr);
		if (entitymap_it != this->entities.end()) {
			LOG2(LOG_PRIORITY::FLOW, "Removing entity '" + name + "' and deleting it.");
			
			EntitySPTR ent((*entitymap_it).second);
			
			this->entities.erase(entitymap_it);
			
			ent->ClearComponents();
			
			// Let all the entities know that the selected entity has been removed.
			// *TODO: build a structure that makes this not have to iterate through every entity.
			BOOST_FOREACH(NamedEntity& named_entity, this->entities) {
				EntitySPTR entity(named_entity.second);
				bool ret = entity->NotifyEntityRemoval(ent);
			}
			
			return true;
		}
		
		LOG2(LOG_PRIORITY::CONFIG, "Entity '" + name +  "' not found. Unable to remove and delete.  Did you add it to the EntityList?");
		return false;
	}

	bool debug_output;
private:
	mutable Threading::ReadWriteMutex mutex;
	NamedEntityMap entities;
};
