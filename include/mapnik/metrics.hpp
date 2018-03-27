/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_METRICS_HPP
#define MAPNIK_METRICS_HPP

#include <mapnik/config.hpp>

#include <boost/optional/optional.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace mapnik {

enum measurement_t : int_fast8_t
{
    UNASSIGNED = 0,
    TIME_MICROSECONDS,
    VALUE,
    CALLS,
    TOTAL_ENUM_SIZE
};

#if defined(__has_cpp_attribute)
    #if __has_cpp_attribute(maybe_unused)
        #define METRIC_UNUSED [[maybe_unused]]
    #elif __has_cpp_attribute(gnu::unused)
        #define METRIC_UNUSED [[gnu::unused]]
    #else
        #define METRIC_UNUSED
    #endif
#else
    #define METRIC_UNUSED
#endif


#ifndef MAPNIK_METRICS

class MAPNIK_DECL metrics
{
public:
    static const bool enabled_ = false;
    inline metrics() {}
    inline metrics(bool) {}
    inline ~metrics() {}

    METRIC_UNUSED inline int measure_time(std::string const&) { return 0; }
    inline void measure_add(char*, int64_t = 0,
                            measurement_t = measurement_t::UNASSIGNED) {}

    METRIC_UNUSED inline int find(const char* const) { return 0; }
    METRIC_UNUSED inline std::string to_string()  { return ""; }
};

#else

struct MAPNIK_DECL measurement
{
    measurement() = default;
    explicit measurement(const char* const name,
                         int64_t value,
                         measurement_t type = measurement_t::UNASSIGNED);

    int64_t value_ = 0;
    int_fast32_t calls_ = 1;
    measurement_t type_ = measurement_t::UNASSIGNED;
    const char* const name_;
};

class metrics;

class MAPNIK_DECL autochrono
{
    using steady_clock = std::chrono::steady_clock;
    using time_units = std::chrono::microseconds;

public:
    autochrono(metrics* m, const char* const name);
    ~autochrono();

    autochrono() = delete;
    autochrono(autochrono const &&) = delete;
    autochrono& operator=(autochrono const &) = delete;
    autochrono& operator=(autochrono &&) = delete;
    autochrono(autochrono const &) = delete;


private:
    metrics* metrics_;
    const char* const name_;
    steady_clock clock_;
    steady_clock::time_point start_;
};


class MAPNIK_DECL metrics
{
    friend autochrono;
public:
    using metrics_array = std::vector<struct measurement>;

    /**
     * Default constructor with an empty tree
     */
    metrics() = delete;
    metrics(bool enabled);

    /* Copy constructor */
    metrics(metrics const &m);

    /* Move constructor */
    metrics(metrics const &&m);

    /* Copy assignment operator */
    metrics& operator=(metrics const &);

    /* Move assignment operator */
    metrics& operator=(metrics &&);

    ~metrics() = default;

    /**
     * Sets up a timer to measure an event. The timer will be stopped and saved
     * when the return value is destroyed / out of scope
     * @param metric_name - Name to use to store the metric
     * @return Smart pointer to hold the reference to the timer
     */
    inline std::unique_ptr<autochrono> measure_time(const char* const name)
    {
        if (!enabled_) return nullptr;
        return measure_time_impl(name);
    }

    /**
     * Increment the value of a metric
     * @param name - Name of the metric
     * @param value - Value to increment. Default = 1
     * @param type - Type of the stored metric. Default: VALUE
     */
    inline void measure_add(const char* const name, int64_t value = 1,
                            measurement_t type = measurement_t::VALUE)
    {
        if (!enabled_) return;
        measure_add_impl(name, value, type);
    }

    /**
     * Find a metric by name
     * @param name - Name of the metric
     * @param ignore_prefix - Whether to ignore stored prefix in the search
     * @return optional value with the measurement
     */
    boost::optional<measurement &> find(const char* const name);

    /**
     * Generates a string with the metrics (for the full tree)
     * @return std::string in JSON style
     */
    std::string to_string();

private:
    std::unique_ptr<autochrono> measure_time_impl(const char* const name);
    void measure_add_impl(const char* const name, int64_t value, measurement_t type);

    std::shared_ptr<metrics_array> storage_ = nullptr;
public:
    /* Whether metrics are enabled or not. If disabled any calls to
     * measure_XXX (add/dec/time) will be ignored */
    bool enabled_ = false;
};

#endif /* ifndef MAPNIK_METRICS */

} //namespace mapnik

#endif /* MAPNIK_METRICS_HPP */
