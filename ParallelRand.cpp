////////////////////////////////////////////////////////////////
//
// FILE:        ParallelRand.cpp
//
// PURPOSE:     Provides structures and functions to
//              automate generation of random numbers during MPI sends
//              and receives.
//
// USE:         (1) Create an instance of the class using either the
//              Bernoulli constructor or the Uniform constructor(the
//              default parameters can serve as a guideline).
//              (2) Use Send() and Recv() just as you would use
//              MPI_Send() and MPI_Recv().
//              (3) Use RandBernoulli() or RandUniform() to generate
//              random numbers.
//
// HOW IT WORKS:During Send() and Recv(), ParallelRand generates
//              random numbers while waiting for communication to
//              finish. The Rand*() calls then use these random
//              numbers when they are invoked. If there are no random
//              numbers available, the Rand*() calls will generate
//              them on demand.
//
// CHANGES:     Created 2003 May 8 by Ben Hocking
/////////////////////////////////////////////////////////////////

#if !defined(PARALLELRAND_HPP)
#include "ParallelRand.hpp"
#endif

ParallelRand ParallelRand::RandComm;

// Bernoulli Constructor
ParallelRand::ParallelRand(float p, int seed, int rng_max_available,
                           int rng_max_gen):m_p(p), m_rng_available(0),
m_rng_max_available(rng_max_available), m_rng_next_available(0),
m_rng_max_gen(rng_max_gen), m_rng_double_bucket(NULL)
{
   m_rng_type = PR_BERNOULLI;
   m_rng_bool_bucket = new bool[rng_max_available];   
   LocalSynNoise.Reset(seed);
}

// Uniform double Constructor(for numbers between low and high)
ParallelRand::ParallelRand(double low, double high, int seed,
                           int rng_max_available,
                           int rng_max_gen):m_low(low), m_high(high), 
m_rng_available(0), m_rng_max_available(rng_max_available),
m_rng_next_available(0), m_rng_max_gen(rng_max_gen), m_rng_bool_bucket(NULL)
{
   m_rng_type = PR_UNIFORM;
   m_rng_double_bucket = new double[rng_max_available];
   LocalSynNoise.Reset(seed);
}

void ParallelRand::Send(void *message, int count, MPI_Datatype datatype,
                        int dest, int tag, MPI_Comm comm)
{
   MPI_Request P_Request;
   MPI_Isend(message, count, datatype, dest, tag, comm, &P_Request);
   MPI_Status P_Status;
   if (m_rng_type == PR_UNIFORM)
      GenUniform(&P_Request, &P_Status);
   else
      GenBernoulli(&P_Request, &P_Status);
}

void ParallelRand::Recv(void *message, int count, MPI_Datatype datatype,
                        int dest, int tag, MPI_Comm comm, MPI_Status * status)
{
   MPI_Request P_Request;
   MPI_Irecv(message, count, datatype, dest, tag, comm, &P_Request);
   if (m_rng_type == PR_UNIFORM)
      GenUniform(&P_Request, status);
   else
      GenBernoulli(&P_Request, status);
}

void ParallelRand::GenBernoulli(MPI_Request * request, MPI_Status * status)
{
   int flag = false;
   MPI_Test(request, &flag, status);
   while (!flag) {
      // If we have room to generate m_rng_max_gen more rng's ...
      if (m_rng_max_available - m_rng_available > m_rng_max_gen) {
         int next_to_gen = (m_rng_next_available + m_rng_available)
             % m_rng_max_available;
         int last_to_gen = (next_to_gen + m_rng_max_gen - 1)
             % m_rng_max_available;
         if (last_to_gen < next_to_gen) {
            // If we've wrapped around, first generate the last
            // few numbers in the bucket...
            LocalSynNoise.Bernoulli(&m_rng_bool_bucket[next_to_gen],
                                  m_rng_max_available - next_to_gen, m_p);
            // Then generate the first few numbers in the bucket
            LocalSynNoise.Bernoulli(m_rng_bool_bucket, last_to_gen + 1, m_p);
         } else {
            LocalSynNoise.Bernoulli(&m_rng_bool_bucket[next_to_gen],
                                  m_rng_max_gen, m_p);
         }
         m_rng_available += m_rng_max_gen;
      }
      MPI_Test(request, &flag, status);
   }
}

void ParallelRand::GenUniform(MPI_Request * request, MPI_Status * status)
{
   int flag = false;
   MPI_Test(request, &flag, status);
   while (!flag) {
      // If we have room to generate m_rng_max_gen more rng's ...
      if (m_rng_max_available - m_rng_available > m_rng_max_gen) {
         int next_to_gen = (m_rng_next_available + m_rng_available)
             % m_rng_max_available;
         int last_to_gen = (next_to_gen + m_rng_max_gen - 1)
             % m_rng_max_available;
         if (last_to_gen < next_to_gen) {
            // If we've wrapped around, first generate the last
            // few numbers in the bucket...
            LocalSynNoise.Uniform(&m_rng_double_bucket[next_to_gen],
                                  m_rng_max_available - next_to_gen, m_low,
                                  m_high);
            // Then generate the first few numbers in the bucket
            LocalSynNoise.Uniform(m_rng_double_bucket, last_to_gen + 1, m_low,
                                  m_high);
         } else {
            LocalSynNoise.Uniform(&m_rng_double_bucket[next_to_gen],
                                  m_rng_max_gen, m_low, m_high);
         }
         m_rng_available += m_rng_max_gen;
      }
      MPI_Test(request, &flag, status);
   }
}
