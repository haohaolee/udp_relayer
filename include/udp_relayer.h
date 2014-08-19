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


namespace hhl {

namespace ba=boost::asio;
using ba::ip::udp;
using ba::ip::address_v4;


class udp_relayer
{
public:
//    typedef std::shared_ptr<concurrent_set<address_v4> > ip_map_ptr;
    typedef std::set<address_v4> address_set;

    udp_relayer(unsigned short port,
                std::size_t concurrent_tasks = 1,
                address_set targets = address_set())
    : service_()
    , port_(port)
    , receiver_(service_, udp::endpoint(udp::v4(), port_))
    , sender_(service_, udp::endpoint(udp::v4(), 0))
    , targets_(std::move(targets))
    , dummy_work_(service_)
    , pause_(targets_.empty())
    {
        sender_.set_option(udp::socket::reuse_address(true));
        sender_.set_option(udp::socket::broadcast(true));
     //   do_receive();
        tasks_.reserve(concurrent_tasks);
        for (std::size_t i = 0; i < concurrent_tasks; ++i)
        {
            tasks_.emplace_back(this, i);
        }
    }

    void run(); //blocking method
    void stop(); // call from internal or from other thread
    void add_target(address_v4); // thread-safe too
    void remove_target(address_v4); // thread-safe three

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
        udp_relayer*                server_ptr_;
        std::size_t                 id_;
        std::size_t                 send_count_;
        status                      status_;
        udp::endpoint               receive_end_;

    public:
        relay_task(udp_relayer* server_ptr, std::size_t id)
        : server_ptr_(server_ptr)
        , id_(id)
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

    boost::asio::io_service        service_;
    const unsigned short           port_;
    udp::socket                    receiver_;
    udp::socket                    sender_;
    address_set                    targets_;
    boost::asio::io_service::work  dummy_work_;
    bool                           pause_;
    std::vector<relay_task>        tasks_;
};


}


#endif

