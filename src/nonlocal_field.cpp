#include "nonlocal_field.hpp"

// constructor
template <typename T>
NonLocalField<T>::NonLocalField(DM *da, BC_type lower_BC_type, double lower_BC_val, \
                        BC_type upper_BC_type, double upper_BC_val) : Field<T>(da)
{
    DMCreateLocalVector(*da, &local_vec);
    DMDAVecGetArray(*da, local_vec, &local_array);
    
    // Initialize boundary conditions
    bc.lower_BC_type = lower_BC_type;
    bc.lower_BC_val = lower_BC_val;
    bc.upper_BC_type = upper_BC_type;
    bc.upper_BC_val = upper_BC_val;
}

// destructor
template <typename T>
NonLocalField<T>::~NonLocalField()
{
    DMDAVecRestoreArray(*(this->da), local_vec, &local_array);
    VecDestroy(&local_vec);
}

// Specialization of template to specific types
// 3D fields
template NonLocalField<double***>::NonLocalField(DM *da, BC_type lower_BC_type, double lower_BC_val, \
                        BC_type upper_BC_type, double upper_BC_val);
template NonLocalField<double***>::~NonLocalField();
// 2D fields
template NonLocalField<double**>::NonLocalField(DM *da, BC_type lower_BC_type, double lower_BC_val, \
                        BC_type upper_BC_type, double upper_BC_val);
template NonLocalField<double**>::~NonLocalField();

// Implementation specific to 3D
template<>
void NonLocalField<double***>::send_global_to_local()
{
    DMGlobalToLocalBegin(*(this->da), global_vec, INSERT_VALUES, local_vec);
    DMGlobalToLocalEnd(*(this->da), global_vec, INSERT_VALUES, local_vec);
    
    // Fill in boundary conditions
    int xs, ys, zs, xm, ym, zm, j, k;
    int nx, ny, nz;
    DMBoundaryType x_BC_type, y_BC_type;
    DMDAGetInfo(*(this->da), NULL, &nx, &ny, &nz, NULL, NULL, NULL, NULL, NULL, &x_BC_type, &y_BC_type, NULL, NULL);
    DMDAGetCorners(*(this->da), &xs, &ys, &zs, &xm, &ym, &zm);
    if (zs == 0)
    {
        // derivative BC
        if (bc.upper_BC_type == derivativeBC)
        {
            for (j = ys; j < ys+ym; j++)
            {
                for (k = xs; k < xs+xm; k++)
                {
                    local_array[-1][j][k] = global_array[1][j][k] - 2.*bc.upper_BC_val;
                }
            }
        }
        else // const BC
        {
            for (j = ys; j < ys+ym; j++)
            {
                for (k = xs; k < xs+xm; k++)
                {
                    local_array[-1][j][k] = bc.upper_BC_val;
                }
            }
        }
    }
    if (zs + zm == nz)
    {
        // derivative BC
        if (bc.lower_BC_type == derivativeBC)
        {
            for (j = ys; j < ys+ym; j++)
            {
                for (k = xs; k < xs+xm; k++)
                {
                    local_array[nz][j][k] = global_array[nz-2][j][k] + 2.*bc.lower_BC_val;
                }
            }
        }
        else // const BC
        {
            for (j = ys; j < ys+ym; j++)
            {
                for (k = xs; k < xs+xm; k++)
                {
                    local_array[nz][j][k] = bc.lower_BC_val;
                }
            }
        }
    }
    
    if (y_BC_type == DM_BOUNDARY_GHOSTED)
    {
        // Use zero-derivative boundary conditions.
        // DM_BOUNDARY_MIRROR is not yet implemented in 3D so have to manually fill these cells
        if (ys == 0)
        {
            for (j = zs; j < zs+zm; j++)
            {
                for (k = xs; k < xs+xm; k++)
                {
                    local_array[j][-1][k] = global_array[j][1][k];
                }
            }
        }
        if (ys + ym == ny)
        {
            for (j = zs; j < zs+zm; j++)
            {
                for (k = xs; k < xs+xm; k++)
                {
                    local_array[j][ny][k] = global_array[j][ny-2][k];
                }
            }
        }
    }
    
    if (x_BC_type == DM_BOUNDARY_GHOSTED)
    {
        // Use zero-derivative boundary conditions.
        // DM_BOUNDARY_MIRROR is not yet implemented in 3D so have to manually fill these cells
        if (xs == 0)
        {
            for (j = zs; j < zs+zm; j++)
            {
                for (k = ys; k < ys+ym; k++)
                {
                    local_array[j][k][-1] = global_array[j][k][1];
                }
            }
        }
        if (xs + xm == nx)
        {
            for (j = zs; j < zs+zm; j++)
            {
                for (k = ys; k < ys+ym; k++)
                {
                    local_array[j][k][nx] = global_array[j][k][nx-2];
                }
            }
        }
    }
    
    return;
}

// Implementation specific to 2D
// TODO



