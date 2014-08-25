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

#ifndef CONFIG_H_INC_
#define CONFIG_H_INC_

#pragma once

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace hhl {

class server_config : private boost::noncopyable
{
public:
    typedef std::vector<unsigned short>  ports_type;

    server_config();
    void parse(std::string const&);

    ports_type const& get_ports_config();
    bool get_ios_per_thread_config();
private:

    template<typename T>
    struct item
    {
        std::string         key;
        boost::optional<T>  value;
		item(std::string const& k)
		:key(k) {}
    };

    item<ports_type>        ports_config_;
    item<bool>              ios_per_thread_config_;
};


}

#endif //CONFIG_H_INC_

