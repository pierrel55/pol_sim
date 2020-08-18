# pol_sim
Simulation of a polarizer in EPR experiment.

This project presents a local realistic approach attempting to explain EPR correlations using local hidden variables.

It uses a function to simulate the behaviour of a polarizer in a logical form.
It has the advantage of using simple boolean and additions operators which can represent realistic physical interactions.

The result returned by this function can be used to:

  - Generate Malus' law between polarizers.
  - Produce detection coincidences for non entangled photons.
  - Produce 3/4 of the QM detection coincidences for entangled photons.
  - Predict in the latter case, one of the three types of correlation that the pair of particles can produce.

#### How it works:

------------


Written in C language the function has the following form:

>   int polarize (struct pho_t * p, int a_pol);

With p a structure representing the state of the photon, and containing two local hidden variables.

>   struct pho_t
  {
    int a;
    int b;
  };

The logical form is as follow:

r = ((a >> (RBITS-2)) & 0x2) ^ (0xbb74 >> ((((a + b) >> (RBITS-2)) & 0xc) | ((a >> (RBITS-1)) & 0x2)));

Although it seem complex, it require low compute resources.

a is photon/polarizer polarization angle difference.
b is a hidden variable attached to photon.
RBITS is a constant defining count of bits used to define PI
0xbb74 is a constant defining polarizer logic.
& | ^ and >> are C language standard boolean operators.

The function uses the angle of the local polarizer and the two hidden variables of the photon to define the output of the polarizer taken by the photon.

It returns an integer containing two informations in bits b0 and b1.
  - Bit b0 indicates the output of the polarizer taken by the photon.
  - Bit b1 defines a state of the particle after passing through the polarizer.

During a detection correlation test between two photons, the pair of bits b1 makes it possible to define 4 states (00 01 10 11).
Then the binary state value predict the correlation type produced by particle pair.

There are 3 possible types:
  - 00 produce cos² correlation.
  - 11 produce square correlation.
  - 10 and 01 produce uncorrelated noise.

There are 4 programs to test the function.

#### correl_QM.C

This program simulates a source of entangled photons and performs correlation measurements for angle differences between polarizers from 0 to 90 ° in steps of 5 degrees.
It selects pairs containing photons in identical states (00 and 11).
Using these pairs it measures the correlation rates of coincidences and compares them with the theoretical results predicted by the QM.
As a reminder, these are cos²(a_diff) and sin²(a_diff) for the detection correlations ++/-- and +-/-+.

#### correl_MT.C

This program is identical to the previous one, but simulates two sources of independent polarized photons.
It then uses all the pairs detected, and compares the correlation measurements with those predicted for non-entangled photons.
These are 0.25 + 0.5 * cos²(a_diff) and 0.25 + 0.5 * sin²(a_diff)

#### malus_check.c

This program allow to verify compliance with Malus law by calculating the rate of photons transmitted as a function of the angle between the polarizers.
It simulate a random number of aligned polarizers with random angles, and checks the final transmission intensity.
Another test evaluating accuracy is performed by aligning 91 polarizers.
It shows that without using the cos² function it is possible to accurately reproduce the Malus law between polarizers.

#### test_eber.c

The correl_QM.C program shows that the polarizer switching effect mathematically generates 3 types of pairs producing different correlations.
If we assume that a physical effect generates a lower probability of detecting pairs containing particles in a different state, then the EPR experiments naturally appear non local.

This latter program shows that with only 1% of simple measurements, produced by partial detection of pairs in states 01 and 10, the Eberhard inequality is violated significantly.


#### Compilation:

------------


These programs run in text mode and can be compiled with GCC using the following command:

Example for correl_QM.c
>   gcc -O2 correl_QM.C -lm -o correl_QM

Graphics and explanations of the operation are available [here](http://pierrel5.free.fr/physique/sim_pol/sim_pol_gr_e.htm "sim_pol_gr")
