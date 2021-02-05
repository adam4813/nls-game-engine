#pragma once

/**
* \file file base name
* \author Adam Martin
* \date 2012-01-30
* \brief Template base class for objects that have a map pattern to another type.
*
*
*/

// Standard Includes
#include <map>
#include <string>

// Library Includes

// Local Includes

// Forward Declarations

// Typedefs

template < typename Class >
class ObjectMap {
public: 
	bool Add(std::string* name, Class* obj) {
		if (this->objMap.find(name) == this->objMap.end()) {
			this->objMap[name] = obj;
			LOG(LOG_PRIORITY::FLOW, std::string("Adding object: " + name).c_str());
			return true;
		} else {
			LOG(LOG_PRIORITY::CONFIG, std::string("Object " + name + ", already exists.").c_str());
			return false;
		}
	}
	Class* Find(std::string name) {
		if (this->objMap.find(name) != this->objMap.end()) {
			this->objMap[name]->Addref();
			return this->objMap[name];
		} else {
			LOG(LOG_PRIORITY::CONFIG, std::string("Object " + name +  " not found.").c_str());
			return nullptr;
		}
	}
	bool Remove(std::string name) {
		if (this->objMap.find(name) != this->objMap.end()) {
			LOG(LOG_PRIORITY::FLOW, std::string("Removing object " + name + " and deleting it.").c_str());
			delete this->objMap[name];
			this->objMap.erase(name);
			return true;
		}

		LOG(LOG_PRIORITY::CONFIG, std::string("Object " + name +  " not found. Unable to remove and delete.").c_str());
		return false;
	}

protected:
	std::map<std::string*, Class*> objMap;
};