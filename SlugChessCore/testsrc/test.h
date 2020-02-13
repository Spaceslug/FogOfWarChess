#pragma once

#ifndef _SHARED_EXPORTS_H__
#define _SHARED_EXPORTS_H__

#ifdef WIN
#ifdef SHARED_EXPORTS
#define SHARED_API __declspec(dllexport)
#else
#define SHARED_API __declspec(dllimport)
#endif
#else
#define SHARED_API
#endif

#endif /* _SHARED_EXPORTS_H__ */



enum wers__Damage { wers__NoDamage, wers__MobilityKill, wers__FirepowerKill, wers__Destroyed, wers__DataFieldMissing=-1 };
struct wers__triplet {
	double pMobilityKill;
	double pFirepowerKill;
	double pDestroyed;
};
	
int version = 1;
int subversion = 3;
	

extern "C" {
	SHARED_API bool wers__initialize(const char* filepath);
	SHARED_API wers__Damage wers__getDamageSimple(const char* Projectile, const double DistanceTravelled, const double ProjectileHeading, const char* Target);
	SHARED_API int* CountDownHandleGiven(const int count, int* arr); 
}
