// libWers.cpp : Defines the exported functions for the DLL application.
//

#ifdef WIN

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

#endif

#include <string>
#include <time.h>
#include <random>
#include <map>
#include <iterator>
#include <fstream>
#include <math.h>


#include "test.h"
#include <iostream>

#define DEG 0.01745329252

#ifndef INTERPTYPE
#define INTERPTYPE 0
/* 0 : bilinear
 * 1 : nearest
 */ 
#endif

using namespace std;
using json = nlohmann::json;

struct MyPair {
	string target;
	string projectile;
	vector<double> directions;
	vector<double> distances;
	vector<vector<double>> pDestroyed;
	vector<vector<double>> pMobilityKill;
	vector<vector<double>> pFirepowerKill;
};

static map<string,MyPair> ProjectileVsTarget;
static vector<const char *> keys;


// Simple initialization of random generator. Check this.
static default_random_engine generator((unsigned int)time(NULL)); 
static uniform_real_distribution<double> distribution(0.0, 1.0);
double randP() { return distribution(generator); }

wers__triplet interp(MyPair p, double angle, double distance) {
	// Move angle into [0, 360>
	double Angle = angle-floor(angle/360.0)*360.0;
	if (p.directions[0] >= 0) Angle = 180.0-abs(180.0-Angle); // Assume Symmetric if directions start at 0 or more. 
	else if (Angle > 180.0) Angle -= 360.0; // If not use angles between (-180, 180).
	double Distance = distance;	
	if (Angle<p.directions[0]) Angle = p.directions[0];
	if (Angle>p.directions[p.directions.size()-1]) Angle = p.directions[p.directions.size()-1];
	if (Distance<p.distances[0]) Distance = p.distances[0];
	if (Distance>p.distances[p.distances.size()-1]) Distance = p.distances[p.distances.size()-1];		
	unsigned int dirIndex, disIndex;
	for (dirIndex = 0; dirIndex<p.directions.size()-1 && p.directions[dirIndex+1]<Angle;  dirIndex++);
	for (disIndex = 0; disIndex<p.distances.size()-1 && p.distances[disIndex+1]<Distance; disIndex++);
#if INTERPTYPE == 1
	if (dirIndex < p.directions.size() && abs(Angle-p.directions[dirIndex+1]) < abs(Angle-p.directions[dirIndex]))
		dirIndex++;
	if (disIndex < p.distances.size() && abs(Distance-p.distances[disIndex+1]) < abs(Distance-p.distances[disIndex]))
		disIndex++;				
	return { p.pMobilityKill[disIndex][dirIndex], p.pFirepowerKill[disIndex][dirIndex], p.pDestroyed[disIndex][dirIndex]};
#else
	if (dirIndex<p.directions.size()-1 && disIndex<p.distances.size()-1) {
		double x = (Angle-p.directions[dirIndex])/(p.directions[dirIndex+1]-p.directions[dirIndex]);
		double y = (Distance-p.distances[disIndex])/(p.distances[disIndex+1]-p.distances[disIndex]);
		return { (1-y)*((1-x)*p.pMobilityKill[disIndex  ][dirIndex] +  x*p.pMobilityKill[disIndex  ][dirIndex+1]) +
					y*((1-x)*p.pMobilityKill[disIndex+1][dirIndex] +  x*p.pMobilityKill[disIndex+1][dirIndex+1]),
				(1-y)*((1-x)*p.pFirepowerKill[disIndex  ][dirIndex] + x*p.pFirepowerKill[disIndex  ][dirIndex+1]) +
					y*((1-x)*p.pFirepowerKill[disIndex+1][dirIndex] + x*p.pFirepowerKill[disIndex+1][dirIndex+1]),
					(1-y)*((1-x)*p.pDestroyed[disIndex  ][dirIndex] +     x*p.pDestroyed[disIndex  ][dirIndex+1]) +
						y*((1-x)*p.pDestroyed[disIndex+1][dirIndex] +     x*p.pDestroyed[disIndex+1][dirIndex+1])
				};
	}
	else if (disIndex < p.distances.size()-1) {
		double y = (Distance-p.distances[disIndex])/(p.distances[disIndex+1]-p.distances[disIndex]);
		return { (1-y)*p.pMobilityKill[disIndex][dirIndex] + y*p.pMobilityKill[disIndex+1][dirIndex],
				 (1-y)*p.pFirepowerKill[disIndex][dirIndex] + y*p.pFirepowerKill[disIndex+1][dirIndex],
				 (1-y)*p.pDestroyed[disIndex][dirIndex] + y*p.pDestroyed[disIndex+1][dirIndex]};
	}
	else if (dirIndex < p.directions.size()-1) {
		double x = (Angle-p.directions[dirIndex])/(p.directions[dirIndex+1]-p.directions[dirIndex]);
		return { (1-x)*p.pMobilityKill[disIndex][dirIndex] + x*p.pMobilityKill[disIndex][dirIndex+1],
				 (1-x)*p.pFirepowerKill[disIndex][dirIndex] + x*p.pFirepowerKill[disIndex][dirIndex+1],
				 (1-x)*p.pDestroyed[disIndex][dirIndex] + x*p.pDestroyed[disIndex][dirIndex+1]};
	}
	else
		return { p.pMobilityKill[disIndex][dirIndex], p.pFirepowerKill[disIndex][dirIndex], p.pDestroyed[disIndex][dirIndex]};
#endif
}




wers__Damage wers__getDamageSimple(const char* Projectile,
					   const double DistanceTravelled,
					   const double ProjectileHeading,
					   const char* Target) {
	double p = randP();	
	if (ProjectileVsTarget.find((string)Projectile+"+"+(string)Target) == ProjectileVsTarget.end())
		return wers__DataFieldMissing;			
	wers__triplet probabilities = interp(ProjectileVsTarget[(string)Projectile+"+"+(string)Target], ProjectileHeading, DistanceTravelled);		
	if (p < probabilities.pDestroyed)
		return wers__Destroyed;
	if (p < probabilities.pFirepowerKill+probabilities.pDestroyed)
		return wers__FirepowerKill;
	if (p < probabilities.pMobilityKill+probabilities.pFirepowerKill+probabilities.pDestroyed)
		return wers__MobilityKill;
	return wers__NoDamage;
}

wers__triplet wers__getDamageSimpleProbabilities(const char* Projectile,
									 const double DistanceTravelled,
									 const double ProjectileHeading,
									 const char* Target) {
	return interp(ProjectileVsTarget[(string)Projectile+"+"+(string)Target], ProjectileHeading, DistanceTravelled);
}

void wers__getDamageSimpleProbabilitiesArg(const char* Projectile, const double DistanceTravelled, const double ProjectileHeading, const char* Target, double *pDestroyed, double *pFirepowerKill, double *pMobilityKill) {
	wers__triplet probs = interp(ProjectileVsTarget[(string)Projectile+"+"+(string)Target], ProjectileHeading, DistanceTravelled);
	*pDestroyed = probs.pDestroyed;
	*pFirepowerKill = probs.pFirepowerKill;
	*pMobilityKill = probs.pMobilityKill;
}

bool wers__initialize(const char *filepath) {						
	std::cout << "testing filepath" << std::endl << std::flush;
	if(filepath != nullptr){
		return true;
	}else{
		return false;
	}
}

const char** wers__getKeys() {		
	return keys.data();
}

unsigned int wers__numKeys() {		
	return (unsigned int)keys.size();
}

bool wers__getKey(unsigned int i, char* projectile, char* target) {
	if (i>=keys.size()) return false;
	strcpy(projectile, ProjectileVsTarget[keys[i]].projectile.c_str());
	strcpy(target, ProjectileVsTarget[keys[i]].target.c_str());
	return true;
}

int* CountDownHandleGiven(const int count, int* arr){
	
	for (int i = 0; i < count; i++)
	{
		arr[i] = count - i;
	}
	return arr;
}

