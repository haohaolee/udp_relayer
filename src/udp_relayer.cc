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
#include <algorithm>
#include <boost/thread.hpp>
#include "interface.h"
#include "udp_relayer.h"

namespace hhl
{
using namespace boost::asio;
using namespace boost::system;

void udp_relayer::relay_task::do_receive()
{
    status_ = running;
    auto& receiver = server_ptr_->receiver_; // udp_relayer receiving socket
    receiver.async_receive_from(
            buffer(data_), receive_end_,
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

    });
}

void udp_relayer::relay_task::do_send(std::size_t length)
{
    auto& server = *server_ptr_;
    size_t count = server.targets_.size();
    if (count == 0)
    {
        status_ = stopped;
        return;
    }

    for(auto addr : server.targets_)
    {
        server.sender_.async_send_to(
                buffer(data_, length),
                udp::endpoint(addr, server.port_),
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
        });

    }
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

void udp_relayer::add_target(ip::address_v4 addr)
{
    service_.post([this, addr]()
    {
        targets_.insert(addr);
        if (pause_)
        {
            pause_ = false;
            do_start();
        }
    });
}

void udp_relayer::remove_target(ip::address_v4 addr)
{
    service_.post([this, addr]()
    {
        targets_.erase(addr);
        if (targets_.empty()) {
            pause_ = true;
        }
    });
}

void udp_relayer::run()
{
    do_start();
    service_.run();
}

void udp_relayer::stop()
{
    service_.stop();
}

////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//

typedef std::unique_ptr<udp_relayer> server_ptr;
std::vector<server_ptr>  g_servers;
boost::thread_group g_group;

static
udp_relayer& create_udp_relayer(
        unsigned short port
)
{
    auto it = std::find_if(
                std::begin(g_servers),
                std::end(g_servers),
				[port](server_ptr const &server_ptr)
            {
                return server_ptr->get_port() == port;
            });

    if (it == std::end(g_servers)) {

        g_servers.push_back(
                server_ptr(new udp_relayer(port))
                );
		return *g_servers.back();
    }
	else
	{
		return *(*it);
	}
}

template<typename Handler>
int catch_exception(Handler&& h)
{
    try {
        (h)();
        return 0;
    }
    catch (boost::system::system_error const &e)
    {
        TRACEA("system_error: %s", e.what());
        return e.code().value();
    }
    catch (std::exception const& e)
    {
        TRACEA("std::exception: %s", e.what()); 
    }
    catch (...)
    {
        TRACEA("Unknown error: %u", ::GetLastError());
    }
    return -1;
}

int start_udp_relayer()
{
    return catch_exception([]()
    {
        using namespace boost::system;
        auto ports = get_ports_from_config();
        if (ports.empty())
            throw system_error(::GetLastError(), system_category());

        for (auto port : ports)
        {
            TRACE(_T("start server on port: %u"), port);
            auto& server = create_udp_relayer(port);
            g_group.create_thread([&server]()
            {
                server.run();
            });
        }
    });
}

int stop_udp_relayer()
{
    return catch_exception([]()
    {
		for (auto& server_ptr : g_servers)
		{
			server_ptr->stop();
		}
		
		g_group.join_all();
		g_servers.clear();
    });
}

void add_target(DWORD ip)
{
    ip::address_v4 addr(
            *reinterpret_cast<ip::address_v4::bytes_type*>(&ip)
            );
    TRACEA("Add target: %s", addr.to_string().c_str());
    for (auto& server_ptr : g_servers)
    {
        server_ptr->add_target(addr);
    }
}

void remove_target(DWORD ip)
{
    ip::address_v4 addr(
            *reinterpret_cast<ip::address_v4::bytes_type*>(&ip)
            );
    TRACEA("Remove target: %s", addr.to_string().c_str());
    for (auto& server_ptr : g_servers)
    {
        server_ptr->remove_target(addr);
    }
}

}

