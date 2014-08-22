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
#ifndef UDP_RELAYER_H_INC
#define UDP_RELAYER_H_INC

#pragma once

#include <set>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>


namespace hhl {

namespace ba=boost::asio;
using ba::ip::udp;
using ba::ip::address_v4;


class udp_relayer : private boost::noncopyable
{
public:
//    typedef std::shared_ptr<concurrent_set<address_v4> > ip_map_ptr;
    typedef std::set<address_v4> address_set;

    udp_relayer(ba::io_service& service,
                unsigned short port,
                std::size_t concurrent_tasks = 1,
                address_set targets = address_set());

    void add_target(address_v4 const&); // thread-safe too
    void remove_target(address_v4 const&); // thread-safe three

    unsigned short get_port() const
    { return port_; }

private:
    class relay_task
    {
        enum {
            data_size = 1500
        };

        enum status {
            stopped,
            running
        };

        std::array<char, data_size> data_;
        udp_relayer*                relayer_ptr_;
        std::size_t                 send_count_;
        status                      status_;
        udp::endpoint               receive_end_;

    public:
        relay_task(udp_relayer* relayer_ptr)
        : relayer_ptr_(relayer_ptr)
        , send_count_(0)
        , status_(stopped)
        {}

        bool is_stopped()
        {
            return status_ == stopped;
        }

        void do_receive();
        void do_send(std::size_t);
    };

private:
    void do_start();

private:

    ba::io_service::strand         strand_;
    const unsigned short           port_;
    udp::socket                    receiver_;
    udp::socket                    sender_;
    address_set                    targets_;
    bool                           pause_;
    std::vector<relay_task>        tasks_;
};


}


#endif

