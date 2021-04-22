#include "version.h"

void
sd_version(int *major, int *minor, int *revision)
{
	*major = UPSKIRT_VERSION_MAJOR;
	*minor = UPSKIRT_VERSION_MINOR;
	*revision = UPSKIRT_VERSION_REVISION;
}
