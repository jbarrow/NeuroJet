////////////////////////////////////////////////////////////////
//
// FILE:        ParallelRand.hpp
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
#   define PARALLELRAND_HPP

#   if !defined(NOISE_HPP)
#      include "Noise.hpp"
#   endif

#   if defined(MULTIPROC)
#      if !defined(MPI_H)
#         include "mpi.h"
#         define MPI_H
#      endif
// Set DEBUG to add a degree of "type" checking. This will
// generate error messages if a Bernoulli instance invokes a
// RandUniform() method or if a Uniform instance invokes a
// RandBernoulli() method.
#      if defined(DEBUG)
#         include <ostream>
#      endif

#      include <cmath>

// Number of random numbers to generate between status checks
const int RNG_MAX_GEN = 1000;

class ParallelRand {
 private:
   enum RandType {
      PR_BERNOULLI,
      PR_UNIFORM
   };
   Noise LocalSynNoise;
   float m_p;                   // For Bernoulli random numbers, probability of success
   double m_low, m_high;        // For Uniform random numbers, range of numbers
   int m_rng_available;         // Number of random numbers available for use
   int m_rng_max_available;     // Depth of random number bucket
   int m_rng_next_available;    // Next random number in bucket to use
   int m_rng_max_gen;           // Number of random numbers to generate at a time
   bool *m_rng_bool_bucket;     // Bucket to hold random Bernoulli numbers
   double *m_rng_double_bucket; // Bucket to hold random floating point numbers
   RandType m_rng_type;         // PR_BERNOULLI (0/1) or PR_UNIFORM (0 to max_int)
   void GenBernoulli(MPI_Request * request, MPI_Status * status);
   void GenUniform(MPI_Request * request, MPI_Status * status);
 public:
    ParallelRand(): m_rng_bool_bucket(NULL), m_rng_double_bucket(NULL) {};

   // Constructor used for creating a uniform distribution between low and high
    ParallelRand(double low, double high, int seed = 1,
                 int rng_max_available = 1000000, int rng_max_gen = 1000);

   // Constructor used for creating a Bernoulli distribution with probability p
    ParallelRand(float p, int seed = 1, int rng_max_available = 1000000,
                 int rng_max_gen = 1000);

   // Use Send the same way you use MPI_Send
   void Send(void *message, int count, MPI_Datatype datatype, int dest, int tag,
             MPI_Comm comm);

   inline void SetParams(float p, int rng_max_available = -1,
                         int rng_max_gen = -1) {
      m_p = p;
      if (rng_max_available >= 0) {
         m_rng_max_available = rng_max_available;
         m_rng_max_gen = rng_max_gen;
      }
      m_rng_type = PR_BERNOULLI;
      // "Delete" current set of random numbers
      m_rng_available = 0;
      m_rng_next_available = 0;
   }

   inline void SetParams(double low, double high,
                    int rng_max_available = -1, int rng_max_gen = -1) {
      m_low = low;
      m_high = high;
      if (rng_max_available >= 0) {
         m_rng_max_available = rng_max_available;
         m_rng_max_gen = rng_max_gen;
      }
      m_rng_type = PR_UNIFORM;
      // "Delete" current set of random numbers
      m_rng_available = 0;
      m_rng_next_available = 0;
   }

   inline void ResetSeed(int seed) {
      LocalSynNoise.Reset(seed);
      // "Delete" current set of random numbers
      m_rng_available = 0;
      m_rng_next_available = 0;
   }

   // Use Recv the same way you use MPI_Recv
   void Recv(void *message, int count, MPI_Datatype datatype, int dest, int tag,
             MPI_Comm comm, MPI_Status * status);

   // Used to retrieve the numbers created during Send
   inline bool RandBernoulli() {
#      if defined(DEBUG)
      if (m_rng_type != PR_BERNOULLI) {
         stderr <<
             "Called RandBernoulli on ParallelRand object created as Uniform!"
             << endl;
         return false;
      } else {
#      else
      {
#      endif
         if (fabs(1.0f - m_p) < 0.000001f) return true;
         if (m_rng_available) {
            bool result = m_rng_bool_bucket[m_rng_next_available];
            m_rng_next_available =
                (m_rng_next_available + 1) % m_rng_max_available;
            m_rng_available--;
            return result;
         } else {
            return LocalSynNoise.Bernoulli(m_p);
         }
      }
   }

   inline double RandUniform() {
#      if defined(DEBUG)
      if (m_rng_type != PR_UNIFORM) {
         stderr <<
             "Called RandUniform on ParallelRand object created as Bernoulli!"
             << endl;
         return 0.0;
      } else {
#      endif
         if (m_rng_available) {
            double result = m_rng_double_bucket[m_rng_next_available];
            m_rng_next_available =
                (m_rng_next_available + 1) % m_rng_max_available;
            m_rng_available--;
            return result;
         } else {
            return LocalSynNoise.Uniform(m_low, m_high);
         }
#      if defined(DEBUG)
      }
#      endif
   }

   static ParallelRand RandComm;
};

#   endif                       //if defined(MULTIPROC)
#endif                          //PARALLELRAND_HPP
