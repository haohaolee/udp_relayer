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
#include <fstream>
#include <sstream>
#include <boost/system/system_error.hpp>
#include "rapidjson/document.h"
#include "config.h"


namespace hhl {
namespace {

using namespace rapidjson;

void parse_ports_config(
    Document& d,
    char const* key,
    boost::optional<std::vector<unsigned short> >& config
)
{
    std::vector<unsigned short> ports;
    auto const& value = d[key];
    if (value.IsArray())
    {
        for(auto it = value.Begin(); it != value.End(); ++it)
        {
            if (it->IsUint())
                ports.emplace_back(it->GetUint());
        }
    }

    if (!ports.empty())
        config = boost::make_optional(std::move(ports));
    
}

void parse_ios_per_thread_config(
        Document& d,
        char const* key,
        boost::optional<bool>& config
       )
{
    auto& b = d[key];
    if (b.IsBool())
        config = boost::make_optional(b.GetBool());
}

std::string
get_json_string(std::string const& file_path)
{
    std::ifstream in(
            file_path, std::ios::in | std::ios::binary
            );
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return contents.str();
    }

    using namespace boost::system;
    int error = ::GetLastError();
    if (error != 0)
	   throw system_error(
                error, system_category());

    throw std::runtime_error("unknown error in get_ports_from_config");

}

}

///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//

server_config::server_config()
: ports_config_("ports")
, ios_per_thread_config_("io_service_per_thread")
{
}


void server_config::parse(std::string const& path)
{
    std::string json = get_json_string(path);

    using namespace rapidjson;
    Document d;
    d.Parse(json.c_str());

    parse_ports_config(d,
            ports_config_.key.c_str(),
            ports_config_.value
    );

    parse_ios_per_thread_config(d,
            ios_per_thread_config_.key.c_str(),
            ios_per_thread_config_.value
    );

}


bool server_config::get_ios_per_thread_config()
{
    if (ios_per_thread_config_.value)
        return *(ios_per_thread_config_.value);

    throw std::runtime_error("config ios_per_thread is invalid");
}

std::vector<unsigned short> const&
server_config::get_ports_config()
{
    if (ports_config_.value)
        return *(ports_config_.value);

    throw std::runtime_error("config ports is invalid");
}


}

