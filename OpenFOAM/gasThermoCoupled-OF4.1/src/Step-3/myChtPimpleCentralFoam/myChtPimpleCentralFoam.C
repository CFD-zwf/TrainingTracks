/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2014 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    myChtPimpleCentralFoam

Description
    Pressure-based semi implicit compressible flow of perfect gas
    with coupled heat transfer in solids solver based on 
    central-upwind schemes of Kurganov and Tadmor 
    and LTS support for steady-state calculations

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "psiThermo.H"
#include "pimpleControl.H"
#include "turbulentFluidThermoModel.H"
#include "zeroGradientFvPatchFields.H"
#include "coupledFvsPatchFields.H"
#include "localEulerDdtScheme.H"
#include "cellQuality.H"
#include "fvOptions.H"
#include "kappaFunction.H"
#include "fvcSmooth.H"
#include "correctCentralACMIInterpolation.H"
#include "Function1.H"
#include "regionProperties.H"
#include "coordinateSystem.H"
#include "solidThermo.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMeshes.H"
    
    pimpleControl pimple(mesh);

    #include "createRDeltaT.H" 
    #include "createRDeltaTVariables.H"
    #include "createTimeControls.H"
    
    #include "createFields.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
    Info<< "\nStarting time loop\n" << endl;
    
    while (runTime.run())
    {
        #include "readAdditionalPimpleControl.H"
        #include "acousticCourantNo.H"
        #include "centralCompressibleCourantNo.H"
        #include "readTimeControls.H"
        #include "setDeltaT.H"

        runTime++;
        
        psi.oldTime();
        rho.oldTime();
        p.oldTime();
        U.oldTime();
        h.oldTime();
        K.oldTime();
        
        Info<< "Time = " << runTime.timeName() << nl << endl;

        // --- Predict density
        #include "massEqn.H"
        
        // --- SIMPLE-like Pressure-Velocity Coupling
        while (pimple.loop())
        {
            // --- Solve turbulence
            turbulence->correct();
            
            // --- Solve momentum
            #include "UEqn.H"
              
            // --- Solve energy
            if (!updateEnergyInPISO)
            {
                #include "hEqn.H"
            }
            
            // --- Solve pressure (PISO)
            while (pimple.correct())
            {
                if (updateEnergyInPISO) //update each iteration before pressure
                {
                    #include "hEqn.H"
                }
                
                #include "pEqn.H"
                if (updateEnergyInPISO)
                {
                    
                    // --- update blending function
                    #include "updateKappa.H"
                    
                    // --- update mechanical fields
                    #include "updateMechanicalFields.H"
                }
            }
            

            if (!updateEnergyInPISO)
            {
                //// --- update weightings for central scheme
                //#include "updateCentralWeights.H"
                
                // --- update blending function
                #include "updateKappa.H"
                
                // --- update mechanical fields
                #include "updateMechanicalFields.H"
            }
        }

        runTime.write();
        
        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}

// ************************************************************************* //
