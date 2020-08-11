#ifndef NCrystal_ScatterIsotropic_hh
#define NCrystal_ScatterIsotropic_hh

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   //
//                                                                            //
//  Copyright 2015-2020 NCrystal developers                                   //
//                                                                            //
//  Licensed under the Apache License, Version 2.0 (the "License");           //
//  you may not use this file except in compliance with the License.          //
//  You may obtain a copy of the License at                                   //
//                                                                            //
//      http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                            //
//  Unless required by applicable law or agreed to in writing, software       //
//  distributed under the License is distributed on an "AS IS" BASIS,         //
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//  See the License for the specific language governing permissions and       //
//  limitations under the License.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "NCrystal/NCScatter.hh"

//////////////////////////////////////////////////////////////////////////////////
// Base class for crystal calculations of scattering in non-oriented materials, //
// i.e. materials where the scattering results do not depend on the incident    //
// direction of the neutron, and where the scatterings are phi-symmetric.       //
//                                                                              //
// Derived classes should reimplement at least crossSectionNonOriented and      //
// generateScatteringNonOriented, but might also reimplement crossSection and   //
// generateScattering for reasons of computational efficiency.                  //
//////////////////////////////////////////////////////////////////////////////////

namespace NCrystal {

  class NCRYSTAL_API ScatterIsotropic : public Scatter {
  public:

    ScatterIsotropic(const char * calculator_type_name);

    //These two methods must be overriden in all derived classes:
    double crossSectionNonOriented( double ekin ) const override;
    void generateScatteringNonOriented( double ekin,
                                        double& angle, double& delta_ekin ) const override;

    //crossSection and generateScattering are here reimplemented in terms of
    //generateScatteringNonOriented and crossSectionNonOriented:
    double crossSection(double ekin, const double (&neutron_direction)[3] ) const override;
    void generateScattering( double ekin, const double (&neutron_direction)[3],
                             double (&resulting_neutron_direction)[3], double& delta_ekin ) const override;

    bool isOriented() const override { return false; }

  protected:
    virtual ~ScatterIsotropic();
  };

}

#endif
