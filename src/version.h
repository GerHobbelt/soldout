/* version.h - holds Upskirt's version */

#ifndef UPSKIRT_VERSION_H
#define UPSKIRT_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif


/*************
 * CONSTANTS *
 *************/

#define UPSKIRT_VERSION "3.0.7"
#define UPSKIRT_VERSION_MAJOR 3
#define UPSKIRT_VERSION_MINOR 0
#define UPSKIRT_VERSION_REVISION 7


/*************
 * FUNCTIONS *
 *************/

/* sd_version: retrieve Upskirt's version numbers */
void sd_version(int *major, int *minor, int *revision);


#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_VERSION_H **/
