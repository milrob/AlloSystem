#ifndef INCLUDE_AL_GRAPHICS_SHAPES_HPP
#define INCLUDE_AL_GRAPHICS_SHAPES_HPP

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "allocore/graphics/al_Mesh.hpp"

namespace al{

// Platonic solids
// Platonic solid data taken from "Platonic Solids (Regular Polytopes In 3D)"
// http://local.wasp.uwa.edu.au/~pbourke/geometry/platonic/

/// Add tetrahedron as triangle vertices and indices
int addTetrahedron(Mesh& m);

/// Add cube as triangle vertices and indices
int addCube(Mesh& m);

/// Add octahedron as triangle vertices and indices
int addOctahedron(Mesh& m);

/// Add dodecahedron as triangle vertices and indices
int addDodecahedron(Mesh& m);

/// Add icosahedron as triangle vertices and indices
int addIcosahedron(Mesh& m);

} // al::

#endif