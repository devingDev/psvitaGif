
#include <psp2/kernel/processmgr.h>

#include "header/Logger.hpp"


void Logger::End(){
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	
#else
	debugNetFinish();
#endif
}

void Logger::Setup(){
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	psvDebugScreenInit();
	psvDebugScreenSetFgColor(COLOR_WHITE);
	psvDebugScreenSetBgColor(COLOR_BLACK);
	
#else
	int ret = debugNetInit(IP_SERVER , port_server , DEBUG);
	debugNetPrintf(INFO, "Connected debugnet! %d \r\n " , ret);
	
#endif
}

void Logger::Info(const std::string & logmsg)
{
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	psvDebugScreenSetFgColor(COLOR_WHITE);
    psvDebugScreenPrintf(logmsg.c_str());
	psvDebugScreenSetFgColor(COLOR_WHITE);
	#else
		
	debugNetPrintf(INFO, logmsg.c_str());
#endif
}
void Logger::InfoDelay(const std::string & logmsg)
{
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	psvDebugScreenSetFgColor(COLOR_WHITE);
    psvDebugScreenPrintf(logmsg.c_str());
	psvDebugScreenSetFgColor(COLOR_WHITE);
	#else
		
	debugNetPrintf(INFO, logmsg.c_str());
#endif
	//sceKernelDelayThread(50*1000);
}

void Logger::Warning(const std::string & logmsg)
{
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	psvDebugScreenSetFgColor(COLOR_YELLOW);
    psvDebugScreenPrintf(logmsg.c_str());
	psvDebugScreenSetFgColor(COLOR_WHITE);
	#else
		
	debugNetPrintf(DEBUG, logmsg.c_str());
#endif
}

void Logger::Error(const std::string & logmsg)
{
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	psvDebugScreenSetFgColor(COLOR_RED);
    psvDebugScreenPrintf(logmsg.c_str());
	psvDebugScreenSetFgColor(COLOR_WHITE);
	#else
		
	debugNetPrintf(ERROR, logmsg.c_str());
#endif
}

void Logger::ClearScreen(){
#if USE_DEBUG_SCREEN_PRINTF == 1
	clearscreen();
	psvDebugScreenSetFgColor(COLOR_WHITE);
	psvDebugScreenSetBgColor(COLOR_BLACK);
#endif
}


void Logger::PrintDis(int which){
#if	USE_DEBUG_SCREEN_PRINTF == 0

#elif USE_DEBUG_SCREEN_PRINTF == 1
	if(which == 1){
		printdis();
	}else if(which == 2){
		printdis2();
	}else{
		printdis3();
	}
#endif
}
