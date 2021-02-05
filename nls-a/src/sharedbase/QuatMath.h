#pragma once

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

// Standard Includes

// Library Includes
#include <d3dx9math.h>

// Local Includes

// Forward Declarations

// Typedefs

void RotateVectorByQuaternion(D3DXVECTOR3* result, const D3DXVECTOR3* vec, const D3DXQUATERNION* rot);
void RotateVectorByQuaternion(D3DXVECTOR3* result, const D3DXQUATERNION* rot);
