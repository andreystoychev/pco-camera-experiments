#include <cstring>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include "pco_err.h"
//#include "sc2_SDKStructures.h"
//#include "SC2_SDKAddendum.h"
//#include "SC2_CamExport.h"
//#include "SC2_Defs.h"


//#define _CRT_SECURE_NO_WARNINGS


void store_tiff(const char *filename, int width, int height, int colormode, void *bufadr);