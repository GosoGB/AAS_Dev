#if defined(MT11)
#ifndef EIP_SESSION_H
#define EIP_SESSION_H

#include <IPAddress.h>
#include <vector>
#include "eip_types.h"

bool eipInit(EIPSession& session, IPAddress ip, uint16_t port = 44818);
bool registerSession(EIPSession& session);
bool unregisterSession(EIPSession& session);
void eipClose(EIPSession& session);


extern EIPSession embeddedEipSession_t;
extern EIPSession link1EipSession_t;

#endif
#endif