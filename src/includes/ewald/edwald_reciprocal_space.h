#ifndef EDWALD_RECIPROCAL_SPACE_H
#define EDWALD_RECIPROCAL_SPACE_H

#include "../structures.h"
#include <complex.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

int _K_RANGE_EWALD = 10;

double complex compute_structural_factor(Particle *particles, int n_particles, Vec3 k)
{
    double complex structural_factor = 0;
    for (size_t i = 0; i < n_particles; i++)
    {
        double qi = particles[i].charge;

        Vec3 r_i = {
            .x = particles[i].x,
            .y = particles[i].y,
            .z = particles[i].z,
        };
        double dot_prod = (k.x * r_i.x + k.y * r_i.y + k.z * r_i.z);
        structural_factor += qi * cexp(I * dot_prod);
    }

    return structural_factor;
}

double reciprocal_space_coulomb_energy(System *s, double ALPHA)
{
    if (ALPHA <= 0)
    {
        printf("ERROR: ALPHA must be greater than 0");
        exit(EXIT_FAILURE);
    }

    // All reciprocal lattice frequencies are multiple of the base frequency 2π/L
    double base_frequency = (2 * PI / s->cell_lenght);

    double sum = 0;
    for (int k_x = -_K_RANGE_EWALD; k_x <= _K_RANGE_EWALD; k_x++)
    {
        for (int k_y = -_K_RANGE_EWALD; k_y <= _K_RANGE_EWALD; k_y++)
        {
            for (int k_z = -_K_RANGE_EWALD; k_z <= _K_RANGE_EWALD; k_z++)
            {
                // K sphere
                if (k_x * k_x + k_y * k_y + k_z * k_z > _K_RANGE_EWALD * _K_RANGE_EWALD) continue;
                // Ignore cell (0,0,0) in k-space
                if (k_x == 0 && k_y == 0 && k_z == 0) continue;

                Vec3 k = {
                    .x = k_x * base_frequency,
                    .y = k_y * base_frequency,
                    .z = k_z * base_frequency,
                };

                double k_mod2 = (k.x * k.x + k.y * k.y + k.z * k.z);
                double complex structural_factor = compute_structural_factor(s->particles, s->n_particles, k);
                sum += (structural_factor * conj(structural_factor)) * exp(-k_mod2 / (4 * ALPHA * ALPHA)) / k_mod2;
            }
        }
    }
    double volume = pow(s->cell_lenght, 3);
    return 0.5 * (4 * PI) / volume * sum;
}

/* double reciprocal_space_coulomb_energy(System *s, double ALPHA)
{
    if (ALPHA <= 0)
    {
        printf("ERROR: ALPHA must be greater than 0");
        exit(EXIT_FAILURE);
    }

    // All reciprocal lattice frequencies are multiple of the base frequency 2π/L
    double base_frequency = (2 * PI / s->cell_lenght);
    double qi, qj;

    double sum = 0;
    for (int k_x = -_K_RANGE; k_x <= _K_RANGE; k_x++)
    {
        for (int k_y = -_K_RANGE; k_y <= _K_RANGE; k_y++)
        {
            for (int k_z = -_K_RANGE; k_z <= _K_RANGE; k_z++)
            {
                // Ignore cell (0,0,0) in k-space
                if (k_x == 0 && k_y == 0 && k_z == 0) continue;

                for (size_t i = 0; i < s->n_particles; i++)
                {
                    qi = s->particles[i].charge;
                    for (size_t j = 0; j < s->n_particles; j++)
                    {
                        qj = s->particles[j].charge;

                        Vec3 r_ij = {
                            .x = s->particles[i].x - s->particles[j].x,
                            .y = s->particles[i].y - s->particles[j].y,
                            .z = s->particles[i].z - s->particles[j].z,
                        };
                        double k_mod2 = (k_x * k_x + k_y * k_y + k_z * k_z) * (base_frequency * base_frequency);
                        double dot_prod = (k_x * r_ij.x + k_y * r_ij.y + k_z * r_ij.z) * base_frequency;
                        sum += (qi * qj) * cexp(I * dot_prod) * exp(-k_mod2 / (4 * ALPHA * ALPHA)) / k_mod2;
                    }
                }
            }
        }
    }
    double volume = pow(s->cell_lenght, 3);
    return 0.5 * (4 * PI) / volume * sum;
} */

/* double reciprocal_space_coulomb_energy_2(System *s, double ALPHA)
{
    if (ALPHA <= 0)
    {
        printf("ERROR: ALPHA must be greater than 0");
        exit(EXIT_FAILURE);
    }

    // All reciprocal lattice frequencies are multiple of the base frequency 2π/L
    double base_frequency = (2 * PI / s->cell_lenght);

    //____________________________________________
    // COMPUTE STRUCTURAL FACTOR FOR ALL K-VECTORS

    double complex structure_factor[2 * _K_RANGE + 1][2 * _K_RANGE + 1][2 * _K_RANGE + 1];

    for (int k_x = -_K_RANGE; k_x <= _K_RANGE; k_x++)
    {
        for (int k_y = -_K_RANGE; k_y <= _K_RANGE; k_y++)
        {
            for (int k_z = -_K_RANGE; k_z <= _K_RANGE; k_z++)
            {
                structure_factor[k_x + _K_RANGE][k_y + _K_RANGE][k_z + _K_RANGE] = 0 + 0 * I;
            }
        }
    }

    for (size_t i = 0; i < s->n_particles; i++)
    {
        // Compute exp(i*2π/L*rᵢ) where i = {x,y,z} for all particle i
        // NOTE: does it depend on reference system?
        double complex e_x = cexp(I * base_frequency * s->particles[i].x);
        double complex e_y = cexp(I * base_frequency * s->particles[i].y);
        double complex e_z = cexp(I * base_frequency * s->particles[i].z);

        double precompure_pow_x[2 * _K_RANGE + 1];
        double precompure_pow_y[2 * _K_RANGE + 1];
        double precompure_pow_z[2 * _K_RANGE + 1];

        for (int j = -_K_RANGE; j <= _K_RANGE; j++)
        {
            precompure_pow_x[j + _K_RANGE] = cpow(e_x, j);
            precompure_pow_y[j + _K_RANGE] = cpow(e_y, j);
            precompure_pow_z[j + _K_RANGE] = cpow(e_z, j);
        }

        for (int k_x = -_K_RANGE; k_x << _K_RANGE; k_x++)
        {
            for (int k_y = -_K_RANGE; k_y <= _K_RANGE; k_y++)
            {
                for (int k_z = -_K_RANGE; k_z <= _K_RANGE; k_z++)
                {
                    structure_factor[k_x + _K_RANGE][k_y + _K_RANGE][k_z + _K_RANGE] +=
                        precompure_pow_x[k_x + _K_RANGE] *
                        precompure_pow_y[k_y + _K_RANGE] *
                        precompure_pow_z[k_z + _K_RANGE] * s->particles[i].charge;
                }
            }
        }
    }

    //________________________________
    // COMPUTE RECIPROCAL SPACE ENERGY

    double potential_sum = 0;

    for (int k_x = -_K_RANGE; k_x <= _K_RANGE; k_x++)
    {
        for (int k_y = -_K_RANGE; k_y <= _K_RANGE; k_y++)
        {
            for (int k_z = -_K_RANGE; k_z <= _K_RANGE; k_z++)
            {
                // If the system is charge neutral ignore the first cell
                if (k_x == 0 && k_y == 0 && k_z == 0) continue;

                double k_mod2 = (k_x * k_x + k_y * k_y + k_z * k_z) * (base_frequency * base_frequency);
                double str_quad = (structure_factor[k_x + _K_RANGE][k_y + _K_RANGE][k_z + _K_RANGE] * conj(structure_factor[k_x + _K_RANGE][k_y + _K_RANGE][k_z + _K_RANGE])); // |S(k)|²
                potential_sum += exp(-k_mod2 / (4 * ALPHA * ALPHA)) / k_mod2 * str_quad;
            }
        }
    }

    double volume = pow(s->cell_lenght, 3);
    return 0.5 * (4 * PI / volume) * potential_sum;

ALPHA_ERROR:
    printf("ALPHA must be greater than 0\n");
    exit(EXIT_FAILURE);
} */

double reciprocal_space_coulomb_energyV(SystemV *s, double ALPHA)
{
    if (ALPHA <= 0)
    {
        printf("ERROR: ALPHA must be greater than 0");
        exit(EXIT_FAILURE);
    }

    // All reciprocal lattice frequencies are multiple of the base frequency 2π/L
    double base_frequency = (2 * PI / s->cell_lenght);

    double sum = 0;
    for (int k_x = -_K_RANGE_EWALD; k_x <= _K_RANGE_EWALD; k_x++)
    {
        for (int k_y = -_K_RANGE_EWALD; k_y <= _K_RANGE_EWALD; k_y++)
        {
            for (int k_z = -_K_RANGE_EWALD; k_z <= _K_RANGE_EWALD; k_z++)
            {
                // K sphere
                if (k_x * k_x + k_y * k_y + k_z * k_z > _K_RANGE_EWALD * _K_RANGE_EWALD) continue;
                // Ignore cell (0,0,0) in k-space
                if (k_x == 0 && k_y == 0 && k_z == 0) continue;

                Vec3 k = {
                    .x = k_x * base_frequency,
                    .y = k_y * base_frequency,
                    .z = k_z * base_frequency,
                };

                double k_mod2 = (k.x * k.x + k.y * k.y + k.z * k.z);
                double complex structural_factor = compute_structural_factor(s->particles, s->n_particles, k);
                sum += (structural_factor * conj(structural_factor)) * exp(-k_mod2 / (4 * ALPHA * ALPHA)) / k_mod2;
            }
        }
    }
    double volume = pow(s->cell_lenght, 3);
    return 0.5 * (4 * PI) / volume * sum;
}

#endif