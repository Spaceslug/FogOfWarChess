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