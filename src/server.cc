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
#include "interface.h"
#include "server.h"


namespace hhl {


server::server(port_vector ports,
               bool ios_per_thread)
: ports_(std::move(ports))
, ios_per_thread_(ios_per_thread)
, thread_count_(0)
, threads_()
{
    create_ios_vector();
    create_udp_relayer_vector();
}


void server::create_ios_vector()
{
    using namespace boost::asio;

    if (ios_per_thread_) {
        for (std::size_t i = 0; i < ports_.size(); ++i)
        {
            io_service_vec_.emplace_back(new io_service(1));
        }
        thread_count_ = io_service_vec_.size();
    }
    else
    {
        std::size_t cpu_count
                = boost::thread::hardware_concurrency();
        io_service_vec_.emplace_back(new io_service(cpu_count));
        thread_count_ = cpu_count;
    }
}

void server::create_udp_relayer_vector()
{
    if (ios_per_thread_)
    {
        ASSERT(ports_.size() == io_service_vec_.size());
        for (std::size_t i = 0; i < ports_.size(); ++i)
        {
            auto& ios = *(io_service_vec_[i]);
            auto port = ports_[i];
            relayer_vec_.emplace_back(
				new udp_relayer(ios, port)
				);
        }
    }
    else
    {
        ASSERT(io_service_vec_.size() == 1);
        auto& ios = *(io_service_vec_.front());
        for(std::size_t i = 0; i < ports_.size(); ++i)
        {
            auto port = ports_[i];
			relayer_vec_.emplace_back(
				new udp_relayer(ios, port)
				);
        }
    }
}


void server::start()
{
    using namespace boost::asio;

    if (ios_per_thread_)
    {
        for (auto& ios_ptr: io_service_vec_)
        {
            auto& ios = *ios_ptr;
            threads_.create_thread([&ios]()
            {
                io_service::work work(ios);
                ios.run();
            });
        }
    }
    else
    {
        auto& ios = *io_service_vec_.front();
        
        for (std::size_t i = 0; i < thread_count_; ++i)
        {
            threads_.create_thread([&ios]()
            {
                io_service::work work(ios);
                ios.run();
            });
        }

    }
}

void server::stop()
{
   for (auto& ios : io_service_vec_)
   {
       ios->stop();
   }

   threads_.join_all();
}

using boost::asio::ip::address_v4;
void server::add_target(address_v4 const& addr)
{
    for (auto& relayer_ptr : relayer_vec_)
    {
        relayer_ptr->add_target(addr);
    }
}


void server::remove_target(address_v4 const& addr)
{
    for (auto& relayer_ptr : relayer_vec_)
    {
        relayer_ptr->remove_target(addr);
    }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


std::unique_ptr<server> g_server_ptr;


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

int start_server()
{
    return catch_exception([]()
    {
        using namespace boost::system;
        auto ports = get_ports_from_config();
        if (ports.empty())
            throw system_error(::GetLastError(), system_category());

        g_server_ptr =
            std::unique_ptr<server>(
                    new server(ports)
                    );

        g_server_ptr->start();

    });
}

int stop_server()
{
    return catch_exception([]()
    {
		g_server_ptr->stop();
        g_server_ptr.reset();
    });
}

void add_target(DWORD ip)
{
    address_v4 addr(
            *reinterpret_cast<address_v4::bytes_type*>(&ip)
            );
    TRACEA("Add target: %s", addr.to_string().c_str());
    g_server_ptr->add_target(addr);
}

void remove_target(DWORD ip)
{
    address_v4 addr(
            *reinterpret_cast<address_v4::bytes_type*>(&ip)
            );
    TRACEA("Remove target: %s", addr.to_string().c_str());
    g_server_ptr->remove_target(addr);
}

}

