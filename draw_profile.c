#include "draw_profile.h"
#include "pew.h"

/* necessary for the thing to compile... */
#ifndef NVPM_INITGUID
#define NVPM_INITGUID   1
#endif

#include "NvPmApi.h"
#include "NvPmApi.Manager.h"


static NVPMContext profile_context;
static NvPmApiManager s_NvPmApiManager;


void draw_profile_Init()
{
	//s_NvPmApiManager.Construct((wchar_t *)pew_GetPath());
	//s_NvPmApiManager.Api()->Init();
}


void draw_profile_Finish()
{
	//s_NvPmApiManager.Api()->Shutdown();
}


char *draw_profile_NVPMResultToString(NVPMRESULT result)
{
    switch (result)
    {
        case NVPM_FAILURE_DISABLED:                 return "NVPM_FAILURE_DISABLED";             break;
        case NVPM_FAILURE_32BIT_ON_64BIT:           return "NVPM_FAILURE_32BIT_ON_64BIT";       break;
        case NVPM_NO_IMPLEMENTATION:                return "NVPM_NO_IMPLEMENTATION";            break;
        case NVPM_LIBRARY_NOT_FOUND:                return "NVPM_LIBRARY_NOT_FOUND";            break;
        case NVPM_FAILURE:                          return "NVPM_FAILURE";                      break;
        case NVPM_OK:                               return "NVPM_OK";                           break;
        case NVPM_ERROR_INVALID_PARAMETER:          return "NVPM_ERROR_INVALID_PARAMETER";      break;
        case NVPM_ERROR_DRIVER_MISMATCH:            return "NVPM_ERROR_DRIVER_MISMATCH";        break;
        case NVPM_ERROR_NOT_INITIALIZED:            return "NVPM_ERROR_NOT_INITIALIZED";        break;
        case NVPM_ERROR_ALREADY_INITIALIZED:        return "NVPM_ERROR_ALREADY_INITIALIZED";    break;
        case NVPM_ERROR_BAD_ENUMERATOR:             return "NVPM_ERROR_BAD_ENUMERATOR";         break;
        case NVPM_ERROR_STRING_TOO_SMALL:           return "NVPM_ERROR_STRING_TOO_SMALL";       break;
        case NVPM_ERROR_INVALID_COUNTER:            return "NVPM_ERROR_INVALID_COUNTER";        break;
        case NVPM_ERROR_OUT_OF_MEMORY:              return "NVPM_ERROR_OUT_OF_MEMORY";          break;
        case NVPM_ERROR_EXPERIMENT_INCOMPLETE:      return "NVPM_ERROR_EXPERIMENT_INCOMPLETE";  break;
        case NVPM_ERROR_INVALID_PASS:               return "NVPM_ERROR_INVALID_PASS";           break;
        case NVPM_ERROR_INVALID_OBJECT:             return "NVPM_ERROR_INVALID_OBJECT";         break;
        case NVPM_ERROR_COUNTER_NOT_ENABLED:        return "NVPM_ERROR_COUNTER_NOT_ENABLED";    break;
        case NVPM_ERROR_COUNTER_NOT_FOUND:          return "NVPM_ERROR_COUNTER_NOT_FOUND";      break;
        case NVPM_ERROR_EXPERIMENT_NOT_RUN:         return "NVPM_ERROR_EXPERIMENT_NOT_RUN";     break;
        case NVPM_ERROR_32BIT_ON_64BIT:             return "NVPM_ERROR_32BIT_ON_64BIT";         break;
        case NVPM_ERROR_STATE_MACHINE:              return "NVPM_ERROR_STATE_MACHINE";          break;
        case NVPM_ERROR_INTERNAL:                   return "NVPM_ERROR_INTERNAL";               break;
        case NVPM_WARNING_ENDED_EARLY:              return "NVPM_WARNING_ENDED_EARLY";          break;
        case NVPM_ERROR_TIME_OUT:                   return "NVPM_ERROR_TIME_OUT";               break;
        case NVPM_WARNING_DUPLICATE:                return "NVPM_WARNING_DUPLICATE";            break;
        case NVPM_ERROR_COUNTERS_ENABLED:           return "NVPM_ERROR_COUNTERS_ENABLED";       break;
        case NVPM_ERROR_CONTEXT_NOT_SUPPORTED:      return "NVPM_ERROR_CONTEXT_NOT_SUPPORTED";  break;
        case NVPM_ERROR_INVALID_CONTEXT:            return "NVPM_ERROR_INVALID_CONTEXT";        break;
        case NVPM_ERROR_GPU_UNSUPPORTED:            return "NVPM_ERROR_GPU_UNSUPPORTED";        break;
        case NVPM_INCORRECT_VALUE_TYPE:             return "NVPM_INCORRECT_VALUE_TYPE";         break;
        default:                                    return "NVPM_ERROR_UNKNOWN";                break;
    }
}

