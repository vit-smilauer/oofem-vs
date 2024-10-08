/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
 *              ##   ##  ##   ##  ####    ####    ##  #  ##
 *             ##   ##  ##   ##  ##      ##      ##     ##
 *            ##   ##  ##   ##  ##      ##      ##     ##
 *            #####    #####   ##      ######  ##     ##
 *
 *
 *             OOFEM : Object Oriented Finite Element Code
 *
 *               Copyright (C) 1993 - 2013   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "field.h"
#include "set.h"

#include <cstdarg>

namespace oofem {
/// Constructor
Field :: Field(FieldType b) : type(b), regionSets()
{
    regionSets.resize(0);
}

void
Field :: setSetsNumbers (const IntArray sets)
{
   regionSets = sets;
}


bool
Field :: hasElementInSets(int nElem, Domain *d)
{
    if (regionSets.giveSize() == 0) {
        return true;
    }
    
    for (int i = 1; i <= regionSets.giveSize(); i++){
        Set *set = d->giveSet( regionSets.at(i) );
        if (set->hasElement(nElem)){
            return true;    
        }
    }
    
    return false;
}


std :: string
Field :: errorInfo(const char *func) const
{
    return std :: string(this->giveClassName()) + "::" + func;
}

} // end namespace oofem
