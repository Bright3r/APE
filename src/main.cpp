#include "core/AppRunner.h"
#include "util/Logger.h"

int main() 
{
	APE::Logger::init();

	AppRunner::init("APE Physics Engine", 1200, 800);
	AppRunner::run();

	return 0; 
}
