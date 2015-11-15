// Public Domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// ALL APIS ARE PLACEHOLDER AT THE MOMENT!
//
// General
// - easy_physics is split into two main parts - collision detection and dynamics.
// - Collision detection is completely independant of the dynamics component and can be used as it's own independant library.
// - The dynamics component depends on the collision detection component.
//
// Collision Detection
// - There are two levels of collision detection:
//   1) Low-level collision detection against two shapes with a particular transform.
//   2) World-based collision detection for detecting collisions at a more global, world level.
// - Collision worlds use a broadphase to accelerate collision detection. There are several broadphase algorithms, each of which
//   have their own advantages and disadvantages.
// - A custom broadphase algorithm can be implemented by simply implementing a create_collision_broadphase_*() function.
//

#ifndef easy_physics
#define easy_physics

#ifdef __cplusplus
extern "C" {
#endif


#ifndef easy_math
// This is temporary while easy_physics and easy_math are in active development. Once things stabilize, easy_math will be included
// directly into this section so that easy_physics is entirely implemented as a single .c and single .h file with no dependencies.
#include "../easy_math/easy_math.h"
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Collision Detection
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct collision_world collision_world;
typedef struct collision_broadphase collision_broadphase;

/// Creates a naive broadphase for use with collision worlds.
///
/// @remarks
///     The naive broadphase is a brute force implementation that can be thought of a reference implementation.
///     @par
///     This is not suitable for collision worlds with more than a few objects.
collision_broadphase* create_collision_broadphase_naive();

/// Creates an accelerated broadphase using a Dynamic AABB Tree.
collision_broadphase* create_collision_broadphase_dbvt();

/// Deletes the given broadphase.
void delete_collision_broadphase(collision_broadphase* pBroadphase);


/// Creates a collision world.
collision_world* create_collision_world(collision_broadphase* pBroadphase);

/// Deletes a collision world.
void delete_collision_world(collision_world* pWorld);





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Dynamics
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct dynamics_world dynamics_world;

/// Creates a dynamics world.
dynamics_world* create_dynamics_world(collision_world* pCollisionWorld);

/// Deletes a dynamics world.
///
/// @remarks
///     This does not delete the collision world.
void delete_dynamics_world(dynamics_world* pWorld);




#ifdef __cplusplus
}
#endif

#endif

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
