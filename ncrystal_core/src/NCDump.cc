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

#include "NCrystal/NCDump.hh"
#include "NCNeutronSCL.hh"
#include "NCMath.hh"
#include "NCrystal/NCInfo.hh"
#include <cstdio>
#include <sstream>
#include <algorithm>

void NCrystal::dump(const NCrystal::Info*c)
{
  const char * hr = "---------------------------------------------------------\n";
  if (c->hasStructureInfo()) {
    const StructureInfo& si = c->getStructureInfo();
    printf("%s", hr);
    if (si.spacegroup!=0)
      printf("Space group number      : %i\n",si.spacegroup);
    printf("Lattice spacings   [Aa] : %g %g %g\n",si.lattice_a,si.lattice_b,si.lattice_c);
    printf("Lattice angles    [deg] : %g %g %g\n",si.alpha,si.beta,si.gamma);
    printf("Unit cell volume [Aa^3] : %g\n",si.volume);
    printf("Atoms / unit cell       : %i\n",si.n_atoms);
  };

  if (c->hasAtomInfo()) {
    printf("%s", hr);
    AtomList::const_iterator itE = c->atomInfoEnd();
    unsigned ntot = 0;
    for (AtomList::const_iterator it = c->atomInfoBegin();it!=itE;++it)
      ntot += it->number_per_unit_cell;
    printf("Atoms per unit cell (total %i):\n",ntot);
    for (AtomList::const_iterator it = c->atomInfoBegin();it!=itE;++it) {
      std::string elem_name = NeutronSCL::instance()->getAtomName(it->atomic_number);
      std::ostringstream s;
      s << "     "<<it->number_per_unit_cell<<" "<<elem_name<<" atoms";
      if (c->hasPerElementDebyeTemperature()||c->hasAtomMSD()) {
        s <<" [";
        if (c->hasPerElementDebyeTemperature()) {
          s <<"T_Debye="<<it->debye_temp<<"K";
          if (c->hasAtomMSD())
            s << ", ";
        }
        if (c->hasAtomMSD())
          s <<"MSD="<<it->mean_square_displacement<<"Aa^2";
        s<<"]";
      }
      printf("%s\n",s.str().c_str());
    }
    if (c->hasAtomPositions()) {
      printf("%s", hr);
      printf("Atomic coordinates:\n");
      for (AtomList::const_iterator it = c->atomInfoBegin();it!=itE;++it) {
        std::string elem_name = NeutronSCL::instance()->getAtomName(it->atomic_number);
        std::vector<AtomInfo::Pos>::const_iterator itP(it->positions.begin()),itPE(it->positions.end());
        for (;itP!=itPE;++itP) {
          printf("     %3s   %10g   %10g   %10g\n",elem_name.c_str(),itP->x,itP->y,itP->z);
        }
      }
    }
  }

  if (c->hasDensity()) {
    printf("%s", hr);
    printf("Density : %g g/cm3\n",c->getDensity());
  }

  if (c->hasNumberDensity()) {
    printf("%s", hr);
    printf("NumberDensity : %g atoms/Aa3\n",c->getNumberDensity());
  }

  if (c->hasComposition()) {
    printf("%s", hr);
    printf("Composition:\n");
    for (auto& ef : c->getComposition())
      printf(" %20g%% %s\n",ef.second*100.0,ef.first.c_str());
  }

  if (c->hasTemperature()) {
    printf("%s", hr);
    printf("Temperature : %g kelvin\n",c->getTemperature());
  }

  if (c->hasGlobalDebyeTemperature()) {
    printf("%s", hr);
    printf("Debye temperature (global) : %g kelvin\n",c->getGlobalDebyeTemperature());
  }

  if (c->hasDynamicInfo()) {
    printf("%s", hr);
    for (auto& di: c->getDynamicInfoList()) {
      printf("Dynamic info for %s (%g%%):\n",di->elementName().c_str(),di->fraction()*100.0);
      auto di_knl = dynamic_cast<const DI_ScatKnl*>(di.get());
      if (di_knl) {
        auto di_skd = dynamic_cast<const DI_ScatKnlDirect*>(di_knl);
        auto di_vdos = dynamic_cast<const DI_VDOS*>(di_knl);
        auto di_vdosdebye = dynamic_cast<const DI_VDOSDebye*>(di_knl);
        printf("   type: S(alpha,beta)%s\n",(di_vdos?" [from VDOS]":(di_vdosdebye?" [from VDOSDebye]":"")));
        auto sp_egrid = di_knl->energyGrid();
        if (!!sp_egrid)
          printf("   egrid: %g -> %g (%llu points)\n",sp_egrid->front(),sp_egrid->back(), (unsigned long long)sp_egrid->size());
        if (di_skd) {
          const auto& sabdata = *(di_skd->ensureBuildThenReturnSAB());
          const auto& ag = sabdata.alphaGrid();
          const auto& bg = sabdata.betaGrid();
          const auto& sab = sabdata.sab();
          printf("   alpha-grid   : %g -> %g (%llu points)\n",ag.front(),ag.back(), (unsigned long long)ag.size());
          printf("   beta-grid    : %g -> %g (%llu points)\n",bg.front(),bg.back(), (unsigned long long)bg.size());
          printf("   S(alpha,beta): %llu points, S_max = %g\n",(unsigned long long)sab.size(), *std::max_element(sab.begin(),sab.end()));
        }
        if (di_vdos) {
          printf("   VDOS Source: %llu points\n",(unsigned long long)di_vdos->vdosData().vdos_density().size());
          printf("   VDOS E_max: %g meV\n",di_vdos->vdosData().vdos_egrid().second*1000.0);
        } else if (di_vdosdebye) {
          printf("   VDOS E_max: %g meV\n",di_vdosdebye->debyeTemperature()*constant_boltzmann*1000.0);
        }
      } else if (dynamic_cast<const DI_Sterile*>(di.get())) {
        printf("   type: sterile\n");
      } else if (dynamic_cast<const DI_FreeGas*>(di.get())) {
        printf("   type: freegas\n");
      } else {
        nc_assert_always(false);
      }
    }
  }

  if (c->hasXSectAbsorption()||c->hasXSectFree()) {
    printf("%s", hr);
    printf("Neutron cross-sections:\n");
    if (c->hasXSectAbsorption())
      printf("   Absorption at 2200m/s : %g barn\n", c->getXSectAbsorption());
    if (c->hasXSectFree())
      printf("   Free scattering       : %g barn\n", c->getXSectFree());
  }

  if (c->providesNonBraggXSects()) {
    printf("%s", hr);
    printf("Provides non-bragg cross-section calculations. A few values are:\n");
    printf("   lambda[Aa]  sigma_scat[barn]\n");
    double ll[] = {0.5, 1.0, 1.798, 2.5, 5, 10, 20 };
    for (unsigned i = 0; i < sizeof(ll)/sizeof(ll[0]); ++i)
      printf("%13g %17g\n",ll[i],c->xsectScatNonBragg(ll[i]));
  }
  if (c->hasHKLInfo()) {
    printf("%s", hr);
    printf("HKL planes (d_lower = %g Aa, d_upper = %g Aa):\n",c->hklDLower(),c->hklDUpper());
    printf("  H   K   L  d_hkl[Aa] Multiplicity FSquared[barn]%s\n",
           (c->hasExpandedHKLInfo()?" Expanded-HKL-list":""));
    HKLList::const_iterator itE = c->hklEnd();
    for (HKLList::const_iterator it = c->hklBegin();it!=itE;++it) {
      printf("%3i %3i %3i %10g %12i %14g%s",it->h,it->k,it->l,it->dspacing,
             it->multiplicity,it->fsquared,(it->eqv_hkl?"":"\n"));
      if (it->eqv_hkl) {
        const short * eqv_hkl = it->eqv_hkl;
        const size_t nEqv = it->demi_normals.size();
        nc_assert_always( nEqv );
        printf(" ");
        for (size_t i = 0 ; i < nEqv; ++i) {
          const short h(eqv_hkl[i*3]), k(eqv_hkl[i*3+1]), l(eqv_hkl[i*3+2]);
          nc_assert(h||k||l);
          printf("%i,%i,%i | %i,%i,%i%s",h,k,l,-h,-k,-l,(i+1==nEqv?"":" | "));
        }
        printf("\n");
      }
    }
  }
  printf("%s", hr);
}
