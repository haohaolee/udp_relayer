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

#ifndef SERVER_H_INC_
#define SERVER_H_INC_

#pragma once

#include <set>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "udp_relayer.h"

namespace hhl {


namespace ba = boost::asio;
using ba::ip::address_v4;

class server: private boost::noncopyable
{
public:
    typedef std::set<address_v4>        address_set;
    typedef std::vector<unsigned short> port_vector;

    server(port_vector ports,
            bool ios_per_thread = false);

    void start();

    void stop();

    void add_target(address_v4 const&);
    void remove_target(address_v4 const&);

private:
    // io_service vector
    typedef std::unique_ptr<ba::io_service>     io_service_ptr;
    typedef std::vector<io_service_ptr>     io_service_vector;

    // udp_relayer vector
    typedef std::unique_ptr<udp_relayer>    udp_relayer_ptr;
    typedef std::vector<udp_relayer_ptr>    udp_relayer_vector;

    void create_ios_vector();
    void create_udp_relayer_vector();

private:

    port_vector             ports_;
    //address_set             targets_;
    bool                    ios_per_thread_;
    io_service_vector       io_service_vec_;
    udp_relayer_vector      relayer_vec_;
    std::size_t             thread_count_;
    boost::thread_group     threads_;

};

}

#endif //SERVER_H_INC_

