
/*#define BUILD
#define BUILDING_DLL*/


#ifdef BUILD
	#ifdef BUILDING_DLL
		#define DLLIMPORT __declspec(dllexport)
		//#define PEWAPI DLLIMPORT
	#else
		#define DLLIMPORT __declspec(dllimport)
	#endif
	
	#define PEWAPI DLLIMPORT
#else 
	#define PEWAPI	
#endif /* BUILD */


//#ifdef BUILD
//	#define PEWAPI DLLIMPORT
//#else
//	#define PEWAPI
//#endif



