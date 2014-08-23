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
#ifndef INTERFACE_H_INC_
#define INTERFACE_H_INC_

#pragma once

namespace hhl {

int start_server();

int stop_server();

void add_target(DWORD);

void remove_target(DWORD);

}

#endif
