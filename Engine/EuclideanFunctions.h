#ifndef LEVIATHAN_EUCLIDEANFUNCTIONS
#define LEVIATHAN_EUCLIDEANFUNCTIONS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "MPUINT.h"
/*--------------------------------------
Original Functions written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
Code Edited by Henri Hyyryläinen
----------------------------------------*/

void EuclideanAlgorithm(const mpuint &x, const mpuint &y, mpuint &a, mpuint &b, mpuint &g);

#endif