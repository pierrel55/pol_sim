# pol_sim
Simulation of a polarizer

This project is a local realistic approach for explaining EPR correlations using local hidden variables.

It uses a function to simulate the behaviour of a polarizer using a logical approach, rather than a simulation of physical laws.

The result returned by this function can be used to:

  - Generate Malus' law between polarizers.
  - Produce detection coincidences for non entangled photons.
  - Produce 3/4 of the detection coincidences for entangled photons.
  - Predict in the latter case whether the measurement conforms to the QM correlations or not.

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

The function uses the angle of the local polarizer and the two hidden variables of the photon to define the output of the polarizer taken by the photon.

It returns an integer containing two informations in bits b0 and b1.

  - Bit b0 indicates the output of the polarizer taken by the photon.
  - Bit b1 defines a state of the particle after passing through the polarizer.

During a detection correlation test between two photons, the pair of bits b1 makes it possible to define 4 states (00 01 10 11).

Pairs of particles with identical states 00 and 11 will then generate the QM detection correlations.
It is then possible with a test using an exclusive OR to know the correlation in advance.

With r1 and r2 being the results returned by each polarizer, we can define the variable c:

> c = (r1 ^ r2) & 2

  - if c = 0   => pair is QM correlated.
  - if c = 2   =>pair is not QM correlated.

Here are 3 programs to test the function.

#####correl_QM.C
This program simulates a source of entangled photons and performs correlation measurements for angle differences between polarizers from 0 to 90 ° in steps of 5 degrees.
It selects pairs containing photons in identical states using the test described above.
Using these pairs it measures the correlation rates of coincidences and compares them with the theoretical results predicted by the QM.
As a reminder, these are cos²(a_diff) and sin²(a_diff) for the detection correlations ++/-- and +-/-+.

#####correl_MT.C
This program is identical to the previous one, but simulates two sources of independent polarized photons.
It then uses all the pairs detected, and compares the correlation measurements with those predicted for non-entangled photons.
These are 0.25 + 0.5*cos²(a_diff) and 0.25 + 0.5*sin²(a_diff)

#####malus_check.c
This program allow to verify compliance with Malus law by calculating the rate of photons transmitted as a function of the angle between the polarizers.
It simulate a random number of aligned polarizers and checks the final transmission intensity.
Another test evaluating accuracy is performed by aligning 91 polarizers.
This show that the output directions chosen by the polarizer function are precise.

####Compilation:

------------


These programs run in text mode and can be compiled with GCC using the following command:

Example for correl_QM.c
>   gcc -O2 correl_QM.C -lm -o correl_QM

Graphics and explanations of the operation are available [here](http://pierrel5.free.fr/physique/sim_pol/sim_pol_gr_e.htm "sim_pol_gr")
