#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>


#define USE_DEBUG_SCREEN_PRINTF 0

#define IP_SERVER 	"192.168.1.107"
#define port_server 18194


#if USE_DEBUG_SCREEN_PRINTF == 1
	#include "debugScreen.h"
#else
	#include <debugnet.h>
#endif



class Logger{
	
	public:
		static void Info(const std::string & logmsg);
		static void InfoDelay(const std::string & logmsg);
		static void Warning(const std::string & logmsg);
		static void Error(const std::string & logmsg);
		static void PrintDis( int which );
		static void ClearScreen();
		static void Setup();
		static void End();
	
	private:
		Logger(){} // Disallow creating an instance of this object
	
	
};




#endif