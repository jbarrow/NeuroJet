// Noise.hpp
//
// Random number generation from uniform and normal distributions
//
// Matt Harrison
//
// Last update : 6/17/97
//
///////////////////////////////////////////////////////////////////////////////
//
// Noise()
// Noise(unsigned long int seed, char type = 't')
// void Reset(unsigned long int seed, char type = 't')
//
// The above function initialize a random number generator with
// a given seed. Initialization using the default constructor
// is provided for use with arrays of Noises. Calling any
// of the distribution functions before initializing the Noise
// with Reset is undefined.
// The type is always 't' and indicates the underlying
// pseudo-random number generator being used.
// 't' corresponds to the TT800 algorithm and 'm' used to correspond
// to the algorithm found in Numerical Recipes in C.
//
// The Uniform functions return doubles on [low,high]
//
// double Uniform(double low, double high)
// void Uniform(double *vec, int rows, double low, double high)
// void Uniform(float *vec, int rows, double low, double high)
// void Uniform(double **matrix, int rows, int cols, double low, double high)
// void Uniform(float **matrix, int rows, int cols, double low, double high)
//
// The Normal functions return doubles from Normal(mu,sigma)
// where mu is the mean and sigma is the standard deviation.
//
// double Normal(double mu, double sigma)
// void Normal(double *vec, int rows, double mu, double sigma)
// void Normal(float *vec, int rows, double mu, double sigma)
// void Normal(double **matrix, int rows, int cols, double mu, double sigma)
// void Normal(float **matrix, int rows, int cols, double mu, double sigma)
//
// The RandInt functions return intgers from the set of integers:
// {low, low+1, low+2, ... , high-2, high-1, high}
// The syntax is identical to the Uniform functions except that
// the RandInt functions take integer parameters.
//
// int RandInt(int low, int high)
// void RandInt(int *vec, int rows, int low, int high)
// void RandInt(int **matrix, int rows, int cols, int low, int high)
//
// The Bernoulli functions return zeros and ones where the
// probability of a one is rate.
//
// int Bernoulli(double rate)
// void Bernoulli(int *vec, int rows, double rate)
// void Bernoulli(int **matrix, int rows, int cols, double rate)
// void Bernoulli(bool *vec, int rows, double rate)
// void Bernoulli(bool **matrix, int rows, int cols, double rate)
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(NOISE_HPP)
#define NOISE_HPP

#include <limits>
#if defined(WIN32) || defined(NONUMERICLIMITS)
#define EPSILON 0.00000001
#else
#define EPSILON std::numeric_limits<float>::epsilon()
#endif

#include "ArgFuncts.hpp"
#include <iostream>
#include <cmath>

class Noise {
 private:

   int IsInit;
   int UseMRNG;
   int QNNumber;
   unsigned long int QNArray[25];
   unsigned long int mag01[2];
   long ix1;
   long ix2;
   long ix3;
   double r[98];
   float gen_time;

   void TRandInit(const unsigned long int &Seed);
   double TRandDbl();
   void TRandDblVect(double *vec, int rows);
   void TRandDblMat(double **matrix, int rows, int cols);
   void TRandFltVect(float *vec, int rows);
   void TRandFltMat(float **matrix, int rows, int cols);

   inline double RandDbl();
   inline void RandDblVect(double *vec, const int rows);

 public:

   inline Noise():IsInit(0), gen_time(0) {
   };
   Noise(const long unsigned int &seed, const char &type = 't');
   inline ~Noise() {
   };

   void Reset(const unsigned long int &seed, const char &type = 't');

   inline double Uniform(const double &low, const double &high);
   void Uniform(double *vec, const int &rows, const double &low,
                const double &high);
   void Uniform(float *vec, const int &rows, const double &low,
                const double &high);
   void Uniform(double **matrix, const int &rows, const int &cols,
                const double &low, const double &high);
   void Uniform(float **matrix, const int &rows, const int &cols,
                const double &low, const double &high);

   inline double Normal(const double &mu, const double &sigma);
   void Normal(double *vec, const int &rows, const double &mu,
               const double &sigma);
   void Normal(float *vec, const int &rows, const double &mu,
               const double &sigma);
   void Normal(double **matrix, const int &rows, const int &cols,
               const double &mu, const double &sigma);
   void Normal(float **matrix, const int &rows, const int &cols,
               const double &mu, const double &sigma);

   inline int RandInt(const int &low, const int &high);
   void RandInt(int *vec, const int &rows, const int &low, const int &high);
   void RandInt(int **matrix, const int &rows, const int &cols,
                const int &low, const int &high);

   inline bool Bernoulli(const double &rate) {
      return ((rate  > veryCloseToOne) || (RandDbl() < rate));
   }
   inline bool Bernoulli(const float &rate) {
      return ((rate  > veryCloseToOne) || (RandDbl() < rate));
   }
   void Bernoulli(int *vec, const int &rows, const double &rate);
   void Bernoulli(int **matrix, const int &rows, const int &cols, const double &rate);
   void Bernoulli(bool * vec, const int &rows, const double &rate);
   void Bernoulli(bool ** matrix, const int &rows, const int &cols, const double &rate);
   inline int Initialized() const { return IsInit; }
};

inline double Noise::Uniform(const double &low, const double &high)
{
   return(RandDbl() * (high - low)) + low;
}

inline double Noise::Normal(const double &mu, const double &sigma)
{
   double U[2];
   RandDblVect(U, 2);
   return((sqrt(-2.0 * log(U[0])) * cos(6.28318530718 * U[1])) * sigma) + mu;
}

inline int Noise::RandInt(const int &low, const int &high)
{

   // 7/21/00
   // Correction added:  while (r > limit -1) 
   //
   // Why?  As apl9f explains it, suppose RAND_MAX is 8 and the interval
   // is 4.  Then, the first interval is going to be 0..3 and the second
   // interval is 4..7  (each of these intervals contains 4.)  You don't
   // want to hit 8 -- that's out of the limit.

   // -----------------------------------------------------
   // 7/18/00 -- New, more correct implementation of RandInt.
   // This new randint makes use of the Linux GCC random number generator.
   // We have checked it via generation of an 8000 neuron cMatrix.  This
   // corrects the bug in which neurons closer together in cMatrix[x] would
   // be more likely to connect to the same neuron.
   // Blame dws3t and pal4s.
   // -----------------------------------------------------

   // -----------------------------------------------------
   // 7/8/04 -- New, even more correct implementation of RandInt.
   // This new randint uses TRandDbl() by default. MRandDbl() can also be used
   // but is STRONGLY discouraged. An 8000 neuron cMatrix was analyzed by the
   // following technique: I compared how many connections neuron i had with
   // neuron i-1 for all neurons, and then did the same thing for i-2, etc., up
   // to i-50. I then plotted the mean values for neurons 1001-2000, 2001-3000,
   // etc. For TRandDbl() and the commented out rand() implementation, the
   // graphs for one group of neurons to the next shared little similarity. The
   // same is not true for MRandDbl().
   // I guess you can blame abh2n, if you must.
   // -----------------------------------------------------
   // MRandDbl was the Numerical Recipes in C random number generator
   
   // How this works:
   // Because of integer division, myRange will be the largest
   // number of complete Intervals in 0...RAND_MAX.
   // limit is < RAND_MAX.  It is the highest multiple of interval
   // less than RAND_MAX.  So if r > limit, we do NOT want to mod on
   // it -- rather, we generate another random number.

   return ifloor(Uniform(low, high + 1 - EPSILON));
}

inline double Noise::RandDbl() {
   return TRandDbl();
}

inline void Noise::RandDblVect(double *vec, const int rows) {
   TRandDblVect(vec, rows);
}

#endif

///////////////////////////////////////////////////////////////////////////////
