![drawing](doc/logo/laiklogo.png)

[![Build Status](https://travis-ci.org/envelope-project/laik.svg?branch=master)](https://travis-ci.org/envelope-project/laik)

# A Library for Automatic Data Migration in Parallel Applications

LAIK is a lightweight library that helps HPC programmers to simplify and improve the flexibility of the code parts responsible for communication between parallel processes of an HPC application. LAIK is not meant to replace usage of e.g. MPI, but it allows programmers to replace part of the communication code by calls into LAIK, which ideally triggers the exact same sequence of MPI calls as a pure MPI application would have done. Pure LAIK programs are possible and enable switching among different communcation libraries.

The benefit of LAIK is a separation of concerns, by requiring the programmer to think about data and work decomposition as a partitioning of abstract index spaces. Communication required for data structures is specified as transition between access phases to the abstract index spaces. For each access phase, the programmer declares which indexes need to be locally available for  processes of the parallel application to do their computation. Knowning the mapping of indexes to concrete data enables LAIK to execute required communication automatically.

For more details, see the [overview](doc/Overview.md) section in the documentation.


## Benefits of using LAIK

* explicit decoupling of the specification of data decomposition
  among parallel tasks from other application code.
  This makes it easier to exchange partitioning algorithms or to
  implement dynamic re-partitioning schemes as well as external control.

* programming style becomes agnostic to the number of parallel processes.
  Each process works on index ranges as specified in the partitioning
  by LAIK. This makes it easier to support elasticity, ie. dynamically
  shrinking and expanding parallel applications.

* use of LAIK data containers can significantly reduce application code
  required for explicit data communication. This makes maintainance easier.
  
* the declarative style of programming allows LAIK to trigger communication
  early, enabling it to overlap communication and computation.

  
# Example

LAIK uses SPMD (single program multiple data) programming style similar to MPI.
For the following simple example, a parallel vector sum, LAIK's communication
funtionality via repartitioning is enough. This example shows the use of a
simple LAIK data container.

```C
    #include "laik-backend-mpi.h"

    int main(int argc, char* argv[])
    {
        // use provided MPI backend, let LAIK do MPI_Init
        Laik_Instance* inst = laik_init_mpi(&argc, &argv);
        Laik_Group* world = laik_world(inst);

        // global 1d double array: 1 mio entries, equal sized blocks
        Laik_Data* a = laik_new_data_1d(world, laik_Double, 1000000);
        // parallel initialization: write 1.0 to own partition
        laik_fill_double(a, 1.0);

        // partial vector sum over own partition via direct access
        double mysum = 0.0, *base;
        uint64_t count, i;
        // map own partition to local memory space
        // (using 1d identity mapping from indexes to addresses, from <base>)
        laik_map_def1(a, (void**) &base, &count);
        for (i = 0; i < count; i++) mysum += base[i];

        // for adding the partial sums and making the result available at
        // master, first, everybody gets write access to a LAIK container
        Laik_Data* sum = laik_new_data_1d(world, laik_Double, 1);
        laik_switchto_new_partitioning(sum, laik_All, LAIK_DF_CopyOut, 0);
        // write partial sum
        laik_fill_double(sum, mysum);
        // we specify that all values from writers should be added using a
        // sum reduction, with the result available at master (process 0)
        laik_switchto_new_partitioning(sum, laik_Master, LAIK_DF_CopyIn, LAIK_RO_Sum);

        if (laik_myid(world) == 0) {
            laik_map_def1(sum, (void**) &base, &count);
            printf("Result: %f\n", base[0]);
        }
        laik_finalize(inst);
    }
```
Compile:
```
    cc vectorsum.c -o vectorsum -llaik
```
To run this example (could use vectorsum directly for OpenMP backend):
```
    mpirun -np 4 ./vectorsum
```

For other examples, see the [index](examples/README.md) of provided examples.


# Build and Install

There is a 'configure' command that detects features of your system and enables corresponding LAIK functionality if found:
* for MPI backend support, MPI must be installed
* for external control via MQTT, mosquitto and protobuf must be installed

To compile, run

    ./configure
    make

There also are clean, install, and uninstall targets. The install defaults
to '/usr/local'. To set the installation path to your home directory, use

    PREFIX=~ ./configure

## Installing the dependencies on Debian/Ubuntu

Packages required for minimal functionality:

    gcc make python

We currently focus on MPI support. E.g. for OpenMPI, install

    libopenmpi-dev openmpi-bin

Other packages:

    g++ libmosquitto-dev libpapi-dev libprotobuf-c-dev protobuf-c-compiler

Mosquitto and protobuf will enable external agents, and PAPI allows
to use performance counters for profiling. C++ is used in some examples.


# Publications

* Josef Weidendorfer, Dai Yang, and Carsten Trinitis. LAIK: A Library for Fault Tolerant Distribution of Global Data for Parallel Applications. In Proceedings of the 27th PARS Workshop (PARS 2017). Hagen, DE, 2017. [PDF](https://mediatum.ub.tum.de/1375185)


# License

LGPLv3+, (c) LRR/TUM
