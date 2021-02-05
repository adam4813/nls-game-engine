/**
* \file Entity
* \author Ricky Curtice
* \date 2012-02-25
* \brief A simple header for easy forward declaration of EntitySPTRs
*
*/
#pragma once

// Standard Includes
#include <memory>

// Library Includes

// Local Includes

// Forward Declarations
class Entity;

// Typedefs
typedef std::shared_ptr<Entity> EntitySPTR;

namespace WHO_DELETES {
	enum TYPE {
		CALLEE,
		CALLER
	};
}
