#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

#include "includes/coulomb/coulomb_potential.h"
#include "includes/ewald/edwald.h"
#include "includes/utils/statistic.h"

int _N_PARTICLES = 30;
double _DENSITY = 0.01;
double _CELL_LENGHT = 1;
double _SIGMA_VELOCITIES = 1.;

int writeParticlesPositions(Particle *particles, int n_particles, FILE *file)
{
    if (!file) return 1;
    if (!particles) return 1;

    for (size_t i = 0; i < n_particles; i++)
    {
        fprintf(file, "%.5E;%.5E;%.5E\n", particles[i].x, particles[i].y, particles[i].z);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    printlnElementSymbol(107);
    srand(RAND_SEED);
    System system;
    system.n_particles = _N_PARTICLES;
    system.cell_lenght = _CELL_LENGHT;
    system.particles = (Particle *)malloc(sizeof(Particle) * system.n_particles);
    if (!system.particles)
    {
        perror("Error, malloc 'system.particles' returned NULL:");
        exit(EXIT_FAILURE);
    }

    //==>INITIALIZATION<==//
    double charge_sum = 0;
    for (size_t i = 0; i < system.n_particles; i++)
    {
        system.particles[i].x = randUnif(-system.cell_lenght / 2, system.cell_lenght / 2);
        system.particles[i].y = randUnif(-system.cell_lenght / 2, system.cell_lenght / 2);
        system.particles[i].z = randUnif(-system.cell_lenght / 2, system.cell_lenght / 2);

        system.particles[i].vx = randGauss(0, _SIGMA_VELOCITIES);
        system.particles[i].vy = randGauss(0, _SIGMA_VELOCITIES);
        system.particles[i].vz = randGauss(0, _SIGMA_VELOCITIES);

        system.particles[i].mass = 1;
        system.particles[i].charge = (i % 2 == 0) ? 1 : -1;
        charge_sum += system.particles[i].charge;
    }

    printf("Total charge: %lf\n", charge_sum);
    FILE *file_start_position = fopen("./data/particel_start_pos.csv", "w");

    writeParticlesPositions(system.particles, system.n_particles, file_start_position);
    // DO THINGS

    int MAX_RANGE = 15;
    _CUTOFF = _CELL_LENGHT / 2;

    /* // Stima errore se particelle disposte casualmente
    double max_error_short = erfc(_ALPHA * _CUTOFF) / _CUTOFF;
    double volume_esterno_sfera = pow(_CELL_LENGHT, 3) - 4. / 3. * PI * pow(_CELL_LENGHT / 2., 3);
    double n_interazioni = system.n_particles * (system.n_particles - 1) / 2.;
    printf("Stima errore totale: %.5E\n", max_error_short * n_interazioni * volume_esterno_sfera);
    printf("Stima errore singolo: %.5E\n", max_error_short); */

    //===== EDWALD 3.5 =====//
    FILE *file_convergenza_edw_3_5 = fopen("./data/range_variabile_edw_3_5.csv", "w");
    _ALPHA = 3.5 / _CUTOFF;
    double last_pot = 0;
    for (size_t i = 0; i < MAX_RANGE; i++)
    {
        _K_RANGE = i;
        double pot = ewald_energy(&system);
        double rel_error = fabs((last_pot - pot) / pot);
        fprintf(file_convergenza_edw_3_5, "%d;%.5E;%.5E\n", i, pot, rel_error);
        printf("{ED, Range:%d, RelErr:%.3E}: %.10E\n", i, rel_error, pot);
        last_pot = pot;
    }

    //===== EDWALD 9 =====//
    FILE *file_convergenza_edw_9 = fopen("./data/range_variabile_edw_9.csv", "w");
    _ALPHA = 9 / _CUTOFF;
    last_pot = 0;
    for (size_t i = 0; i < MAX_RANGE; i++)
    {
        _K_RANGE = i;
        double pot = ewald_energy(&system);
        double rel_error = fabs((last_pot - pot) / pot);
        fprintf(file_convergenza_edw_9, "%d;%.5E;%.5E\n", i, pot, rel_error);
        printf("{ED, Range:%d, RelErr:%.3E}: %.10E\n", i, rel_error, pot);
        last_pot = pot;
    }

    //===== EDWALD 7 =====//
    FILE *file_convergenza_edw_7 = fopen("./data/range_variabile_edw_7.csv", "w");
    _ALPHA = 7 / _CUTOFF;
    last_pot = 0;
    for (size_t i = 0; i < MAX_RANGE; i++)
    {
        _K_RANGE = i;
        double pot = ewald_energy(&system);
        double rel_error = fabs((last_pot - pot) / pot);
        fprintf(file_convergenza_edw_7, "%d;%.5E;%.5E\n", i, pot, rel_error);
        printf("{ED, Range:%d, RelErr:%.3E}: %.10E\n", i, rel_error, pot);
        last_pot = pot;
    }

    //===== EDWALD 9 =====//
    FILE *file_convergenza_edw_2 = fopen("./data/range_variabile_edw_2.csv", "w");
    _ALPHA = 2 / _CUTOFF;
    last_pot = 0;
    for (size_t i = 0; i < MAX_RANGE; i++)
    {
        _K_RANGE = i;
        double pot = ewald_energy(&system);
        double rel_error = fabs((last_pot - pot) / pot);
        fprintf(file_convergenza_edw_2, "%d;%.5E;%.5E\n", i, pot, rel_error);
        printf("{ED, Range:%d, RelErr:%.3E}: %.10E\n", i, rel_error, pot);
        last_pot = pot;
    }

    //===== COULOMB =====//
    FILE *file_convergenza_coulomb = fopen("./data/range_variabile_coulomb.csv", "w");
    last_pot = 0;
    for (size_t i = 0; i < 50; i++)
    {
        _R_RANGE = i;
        double pot = getCoulombPotential(&system);
        double rel_error = fabs((last_pot - pot) / pot);
        fprintf(file_convergenza_coulomb, "%d;%.5E;%.5E\n", i, pot, rel_error);
        printf("{C, Range:%d, RelErr:%.3E}: %.10E\n", i, rel_error, pot);
        last_pot = pot;
    }
    free(system.particles);
}
