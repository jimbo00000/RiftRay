// AppDirectories.h
// A place to store the app's home data directory

#pragma once

#ifdef _DEBUG
#define HOME_DATA_DIR "../"
#else
#define HOME_DATA_DIR "./"
#endif
