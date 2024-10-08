#if !defined(COULMB_POTENTIAL_H)
#define COULMB_POTENTIAL_H

#include "../constants.h"
#include "../structures.h"
#include "../utils/lattice.h"
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>

int _R_RANGE = 5;

double getCoulombPotential(System *s)
{
    int64_t skipped_for_truncation = 0;
    double sum = 0;

    for (size_t i = 0; i < s->n_particles; i++)
    {
        for (size_t j = 0; j < s->n_particles; j++)
        {
            for (int r_x = -_R_RANGE; r_x <= _R_RANGE; r_x++)
            {
                for (int r_y = -_R_RANGE; r_y <= _R_RANGE; r_y++)
                {
                    for (int r_z = -_R_RANGE; r_z <= _R_RANGE; r_z++)
                    {
                        // Exclude self particle in cell (0,0,0)
                        if (r_x == 0 && r_y == 0 && r_z == 0 && i == j) continue;

                        Vec3 r_ij = {
                            .x = s->particles[i].x - s->particles[j].x,
                            .y = s->particles[i].y - s->particles[j].y,
                            .z = s->particles[i].z - s->particles[j].z,
                        };

                        // if (r_x * r_x + r_y * r_y + r_z * r_z > _R_RANGE * _R_RANGE) continue;

                        r_ij.x = minimum_image(r_ij.x, s->cell_lenght) + r_x * s->cell_lenght;
                        r_ij.y = minimum_image(r_ij.y, s->cell_lenght) + r_y * s->cell_lenght;
                        r_ij.z = minimum_image(r_ij.z, s->cell_lenght) + r_z * s->cell_lenght;

                        double r_ij_mod = sqrt(r_ij.x * r_ij.x + r_ij.y * r_ij.y + r_ij.z * r_ij.z);

                        double old_sum = sum;
                        sum += (s->particles[i].charge * s->particles[j].charge) / r_ij_mod;

                        if (old_sum == sum && (s->particles[i].charge * s->particles[j].charge) != 0)
                        {
                            skipped_for_truncation += 1;
                        }
                    }
                }
            }
        }
    }

    if (skipped_for_truncation != 0) printf("WARNING: %lld skipped for truncation\n", skipped_for_truncation);

    return 0.5 * sum;
}

#endif // COULMB_POTENTIAL_H
