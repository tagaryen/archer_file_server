#include "Initializer.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

using namespace fs;

int main() {
    fs::initializer::startAll();
    return 0;
}