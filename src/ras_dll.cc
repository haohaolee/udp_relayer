///////////////////////////////////////////////////////////////////////////////
//
// http://github.com/haohaolee/udp_relayer
//
// Copyright (C) 2014 haohaolee <hayatelee[at]gmail.com>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
///////////////////////////////////////////////////////////////////////////////
#include "common.h"
#include <mprapi.h>
#include "interface.h"

DWORD __stdcall MprAdminInitializeDll()
{
    TRACE(_T("Initializing Mpr Admin dll\n"));
    return hhl::start_udp_relayer();
}

DWORD __stdcall MprAdminTerminateDll()
{
    TRACE(_T("Terminating Mpr Admin dll\n"));
    return hhl::stop_udp_relayer();
}

BOOL __stdcall MprAdminAcceptNewLink(RAS_PORT_0 *pRasPort0,
                            RAS_PORT_1 *)
{
    (void)pRasPort0;
    TRACEW(L"AcceptNewLink: Port name is %s\n", pRasPort0->wszPortName);
    return TRUE;
}

void __stdcall MprAdminLinkHangupNotification(RAS_PORT_0 *pRasPort0,
                                    RAS_PORT_1 *)
{
    (void)pRasPort0;
    TRACEW(L"HangupNotification: Port Name is %s\n", pRasPort0->wszPortName);
}

BOOL __stdcall MprAdminAcceptNewConnection(
        RAS_CONNECTION_0 *pRasConnection0,
        RAS_CONNECTION_1 *
)
{
    (void)pRasConnection0;
    TRACEW(L"AcceptNewConnection: Interface is %s, Username is %s\n",
            pRasConnection0->wszInterfaceName,
            pRasConnection0->wszUserName);
    return TRUE;
}

void __stdcall MprAdminConnectionHangupNotification(
        RAS_CONNECTION_0 *pRasConnection0,
        RAS_CONNECTION_1 *
)
{
    (void)pRasConnection0;
    TRACEW(L"ConnectionHangupNotification: Interface is %s, Username is %s\n",
            pRasConnection0->wszInterfaceName,
            pRasConnection0->wszUserName);
}

DWORD __stdcall MprAdminGetIpAddressForUser(
        WCHAR *,
        WCHAR *,
        DWORD *lpdwIpAddress,
        BOOL *bNotifyRelease
)
{
    TRACE(_T("GetIpAddressForUser: The ip is %u\n"), *lpdwIpAddress);
    hhl::add_target(*lpdwIpAddress);
    *bNotifyRelease = TRUE;
    return NO_ERROR;
}

void __stdcall MprAdminReleaseIpAddress(
        WCHAR *,
        WCHAR *,
        DWORD *lpdwIpAddress
)
{
    TRACE(_T("ReleaseIpAddres: The ip is %u\n"), *lpdwIpAddress);
    hhl::remove_target(*lpdwIpAddress);
}


