/***************************************************************************
 *   Copyright (C) 2009-2011 by the resistivity.net development team       *
 *   Thomas G�nther thomas@resistivity.net                                 *
 *   Carsten R�cker carsten@resistivity.net                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _GIMLI_EM1DMODELLING__H
#define _GIMLI_EM1DMODELLING__H

#include "gimli.h"
#include "mesh.h"
#include "meshgenerators.h"
#include "modellingbase.h"

namespace GIMLI{
/*! this file holds different electromagnetic forward operators for 1D discretizations */

//! Magnetotellurics (MT) 1D modelling
/*! Classical MT 1D block model yielding rhoa&phase for given resistivities&thicknesses */
class DLLEXPORT MT1dModelling : public ModellingBase {
public:
    MT1dModelling( const RVector & periods, size_t nlay, bool verbose = false )
        : ModellingBase( verbose ), periods_( periods ), nlay_( nlay ) {
        setMesh( createMesh1DBlock( nlay ) );
    }

    MT1dModelling( Mesh & mesh, const RVector & periods, size_t nlay, bool verbose = false )
        : ModellingBase( mesh, verbose ), periods_( periods ), nlay_( nlay ) {
    }

    virtual ~MT1dModelling() { }

    /*! different sub-forward operators for alternate use */
    virtual RVector rhoaphi( const RVector & rho, const RVector & thk ); //! app. res. and phase

    virtual RVector rhoa( const RVector & rho, const RVector & thk );    //! only app. res.

    virtual RVector rhoa( const RVector & model );
    //! app. res. for thk/res vector
       

    /*! the actual (full) forward operator returning app.res.+phase for thickness+resistivity */
    virtual RVector response( const RVector & model );
        
protected:
    RVector periods_;
    size_t nlay_;
};

/*! variant with fixed thickness vector (for smooth (Occam) inversion) */
class DLLEXPORT MT1dRhoModelling : public MT1dModelling {
public:
    MT1dRhoModelling( Mesh & mesh, const RVector & periods, const RVector & thk, bool verbose = false )
        : MT1dModelling( mesh, periods, thk.size(), verbose ), thk_( thk ) {
    }

//    MT1dRhoModelling( RVector & periods, RVector & thk, bool verbose = false )
//        : MT1dModelling( createMesh1D( thk.size() + 1 ), periods, thk.size(), verbose ), thk_( thk ) { }

    virtual ~MT1dRhoModelling() { }

    virtual RVector response( const RVector & rho ) { return rhoaphi( rho, thk_ ); }

    virtual RVector rhoa( const RVector & rho ) { return MT1dModelling::rhoa( rho, thk_ ); }

protected:
    RVector thk_;
};

/*! Magnetic Resonance Sounding (MRS) modelling */
/*! classical variant using a fixed parameterization */
class DLLEXPORT MRSModelling : public ModellingBase {
public:
    //! constructor using a predefined mesh and real/imag matrix
    MRSModelling( Mesh & mesh, RMatrix & KR, RMatrix & KI, bool verbose = false ) :
            ModellingBase( mesh, verbose ), KR_( &KR ), KI_( &KI ) { }
    //! constructor with real/imag matrix only
    MRSModelling( RMatrix & KR, RMatrix & KI, bool verbose = false ) :
            ModellingBase( verbose ), KR_( &KR ), KI_( &KI ) { }
    //! destructor
    virtual ~MRSModelling() { }
    //! return response voltage for a given water content vector
    RVector response( const RVector & model );
    //! create jacobian matrix (AmplitudeJacobian)
    void createJacobian( RMatrix & jacobian, const RVector & model );
        
protected:
    RMatrix *KR_, *KI_;
};

/*! Magnetic Resonance Sounding (MRS) modelling */
/*! non-classical variant using a block model using mapping to classical varian */
class DLLEXPORT MRS1dBlockModelling : public MRSModelling{
public:
    //! constructor using mesh, real/imag matrix and zvector;
    MRS1dBlockModelling( Mesh & mesh, RMatrix & KR, RMatrix & KI, RVector & zvec, bool verbose = false ) 
        : MRSModelling( mesh, KR, KI, verbose ), nlay_( ( mesh.cellCount() + 1 ) / 2 ),     
                        nvec_( KR.cols() ),zvec_( zvec ) { 
    }
    //! constructor using real/imag matrix and zvector;
    MRS1dBlockModelling( int nlay, RMatrix & KR, RMatrix & KI, RVector & zvec, bool verbose = false ) 
        : MRSModelling( KR, KI, verbose ), nlay_( nlay ), nvec_( KR.cols() ), zvec_( zvec ) { 
            setMesh( createMesh1DBlock( nlay ) ); 
        }
    
    virtual ~MRS1dBlockModelling() { }
    //! return voltage for a given block model vector;
    RVector response( const RVector & model );
    //! use default (brute-force) jacobian generator (override the smooth one);
    void createJacobian( RMatrix & jacobian, const RVector & model ){
        ModellingBase::createJacobian( jacobian, model );
    }

protected:
    size_t nlay_;
    size_t nvec_;
    RVector zvec_;
};



} // namespace GIMLI{

#endif // _GIMLI_EM1D__H