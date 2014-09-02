#ifndef COMMONMATH_H
#define COMMONMATH_H

#ifdef __cplusplus
extern "C" {
#endif

// Calculating ChiScquare percantage
double GetPercantByChiSqruare(int Dof, double Cv);
double CalculateIncompleteGamma(double S, double Z);
double CalculateGamma(double N);

#ifdef __cplusplus
}
#endif

#endif // COMMONMATH_H
