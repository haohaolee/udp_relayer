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
#include <Shlwapi.h>
#include <boost/system/system_error.hpp>
#include <boost/range.hpp>
#include "rapidjson/document.h"
#include "interface.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace hhl {

static std::string get_config_path()
{
    static char dll_path[MAX_PATH];
    std::size_t size = ::GetModuleFileNameA(
                        (HINSTANCE(&__ImageBase)), dll_path, boost::size(dll_path)
                        );
    if (size > 0 && size < boost::size(dll_path))
	{
        std::string config_path = std::string(dll_path)+".json";
        if (PathFileExistsA(config_path.c_str()))
            return std::move(config_path);
	}

    using namespace boost::system;
	throw system_error(
            errc::no_such_file_or_directory, generic_category(), "no config file");

}

static std::vector<unsigned short>
get_ports_from_json(std::string const& json)
{
    using namespace rapidjson;
    std::vector<unsigned short> ports;
    Document d;
    d.Parse(json.c_str());
    auto const& value = d["ports"];
    if (value.IsArray())
    {
        for(auto it = value.Begin(); it != value.End(); ++it)
        {
            if (it->IsUint())
                ports.emplace_back(it->GetUint());
        }
    }

    return std::move(ports);
}

std::vector<unsigned short>
get_ports_from_config()
{
    std::ifstream in(
            get_config_path(), std::ios::in | std::ios::binary
            );
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return get_ports_from_json(contents.str());
    }

    using namespace boost::system;
    int error = ::GetLastError();
    if (error != 0)
	   throw system_error(
                error, system_category());

    throw std::runtime_error("unknown error in get_ports_from_config");
}

}

