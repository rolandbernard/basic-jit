#ifndef _FILEEXEC_H_
#define _FILEEXEC_H_

#include <stdbool.h>

#include "codegen/codegen.h"

int executeFile(const char* filename);

int generateMcIntoData(const char* filename, MCGenerationData* data);

#endif
