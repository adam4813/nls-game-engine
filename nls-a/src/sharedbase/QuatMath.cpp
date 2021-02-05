/**
* @file QuatMath.h
* @author Ricky Curtice
* @date 20110731
* @brief Provides some simple helper functions that seem to be missing in DX in regards to quaternions.
*
* <p>
* </p>
*
* Team: Team A - Graphics
* Justin Avignone, Ricky Curtice, Jordan Lawyer, Brett Ramsey, Chris Schaber
*/

#include "QuatMath.h"

// Standard Includes

// Library Includes

// Local Includes

// Static class member initialization

// Class methods in the order they are defined within the class header
void RotateVectorByQuaternion(D3DXVECTOR3* result, const D3DXVECTOR3* vec, const D3DXQUATERNION* rot) {
	D3DXQUATERNION rotInverse;
	D3DXQUATERNION pureQuat (vec->x, vec->y, vec->z, 0.0f);
	D3DXQuaternionInverse(&rotInverse, rot);
	
	pureQuat = rotInverse * pureQuat * *rot;
	
	result->x = pureQuat.x;
	result->y = pureQuat.y;
	result->z = pureQuat.z;
}

void RotateVectorByQuaternion(D3DXVECTOR3* result, const D3DXQUATERNION* rot) {
	RotateVectorByQuaternion(result, result, rot);
}
