#include <config.h>  // IWYU pragma: keep

#ifdef WITH_PROTOBUF
#include "eixTk/diagnostics.h"
GCC_DIAG_OFF(sign-conversion)
#include "output/eix.pb.cc"
GCC_DIAG_ON(sign-conversion)
#endif
