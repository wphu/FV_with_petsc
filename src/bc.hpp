#ifndef BC_H
#define BC_H

#include <petscsys.h>
#include <petscdm.h>
#include <petscdmda.h>

// Use an enum for boundary condition flags
enum class BC_type 
{
    constantBC,
    derivativeBC,
    periodicBC
};

// Upper and lower boundary conditions and values
struct BC 
{
    // Initialize with default values
    BC_type lower_BC_type = BC_type::derivativeBC;
    double lower_BC_val = 0.;

    BC_type upper_BC_type = BC_type::derivativeBC;
    double upper_BC_val = 0.;
           
    // Helper function to convert to PETSc's DMDA boundary condition types
    DMBoundaryType get_DMBoundaryType()
    {
        if (lower_BC_type == BC_type::periodicBC) {
            return DM_BOUNDARY_PERIODIC;
        }
        return DM_BOUNDARY_GHOSTED;
    }
};

#endif