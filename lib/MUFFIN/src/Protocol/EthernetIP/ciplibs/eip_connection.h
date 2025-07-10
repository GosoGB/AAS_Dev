#if defined(MT11)
#ifndef EIP_CONNECTION_H
#define EIP_CONNECTION_H

#include "eip_types.h"

// Forward Open 명령 (Class 3 연결 설정)
bool forwardOpen(EIPSession& session);

// Forward Close 명령 (Class 3 연결 해제)
bool forwardClose(EIPSession& session);

#endif
#endif