////////////////////////////////////////////////////////////////////////////////
//
// http://github.com/haohaolee/udp_relayer
//
// Copyright (C) 2014 haohaolee <hayatelee[at]gmail.com>
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
/////////////////////////////////////////////////////////////////////////////////
#include "common.h"
#include "interface.h"
#include "udp_relayer.h"

namespace hhl
{
using namespace boost::asio;
using namespace boost::system;

void udp_relayer::relay_task::do_receive()
{
    status_ = running;
    auto& receiver = relayer_ptr_->receiver_; // udp_relayer receiving socket
    auto& strand = relayer_ptr_->strand_;

    receiver.async_receive_from(
            buffer(data_), receive_end_,
            strand.wrap(
                [this](
                    error_code const& ec,
                    std::size_t bytes
                    )
    {
        if (ec) {
            TRACEA(
                "error ocurrs in receive handler: %s",
                ec.message().c_str()
            );
        }
        else if (bytes > 0) {
            do_send(bytes);
            return;
        }
        do_receive();

    }));
}

void udp_relayer::relay_task::do_send(std::size_t length)
{
    auto& relayer = *relayer_ptr_;
    size_t count = relayer.targets_.size();
    if (count == 0)
    {
        status_ = stopped;
        return;
    }

    auto& strand = relayer.strand_;
    for(auto addr : relayer.targets_)
    {
        relayer.sender_.async_send_to(
                buffer(data_, length),
                udp::endpoint(addr, relayer.port_),
                strand.wrap(
                    [this, count](
                        boost::system::error_code const& ec,
                        std::size_t
                    )
        {
            if (ec)
            {
                TRACEA("error ocurrs in send handler: %s", ec.message().c_str());
            }

            if (++send_count_ == count)
            {
                send_count_ = 0; // all sending is complete
                do_receive();
            }
        }));

    }
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////

udp_relayer::udp_relayer(io_service& service,
                         unsigned short port,
                         std::size_t concurrent_tasks,
                         address_set targets)
: strand_(service)
, port_(port)
, receiver_(service, udp::endpoint(udp::v4(), port_))
, sender_(service, udp::endpoint(udp::v4(), 0))
, targets_(std::move(targets))
, pause_(targets_.empty())
{
    sender_.set_option(udp::socket::reuse_address(true));
    sender_.set_option(udp::socket::broadcast(true));

    tasks_.reserve(concurrent_tasks);
    for (std::size_t i = 0; i < concurrent_tasks; ++i)
    {
        tasks_.emplace_back(this);
    }

    do_start();
}

void udp_relayer::do_start()
{
    if (pause_)
        return;

    for (auto& task : tasks_)
    {
        if (task.is_stopped())
            task.do_receive();
    }
}

void udp_relayer::add_target(ip::address_v4 const& addr)
{
    strand_.post([this, addr]()
    {
        targets_.insert(addr);
        if (pause_)
        {
            pause_ = false;
            do_start();
        }
    });
}

void udp_relayer::remove_target(ip::address_v4 const& addr)
{
    strand_.post([this, addr]()
    {
        targets_.erase(addr);
        if (targets_.empty()) {
            pause_ = true;
        }
    });
}



}

