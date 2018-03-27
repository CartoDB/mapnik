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

#ifdef MAPNIK_METRICS

#include <mapnik/metrics.hpp>

#include <mapnik/make_unique.hpp>

#include <algorithm>
#include <sstream>
#include <utility>

namespace mapnik {

const std::string measurement_str[TOTAL_ENUM_SIZE] =
{
    "UNKNOWN",
    "Time (us)",
    "Value",
    "Calls"
};

measurement::measurement(const char* const name, int64_t value, measurement_t type)
    : value_(value),
      type_(type),
      name_(name)
{
}

autochrono::autochrono(metrics* m, const char* const name)
    : metrics_(m),
      name_(name)
{
    start_ = clock_.now();
}

autochrono::~autochrono()
{
    auto ns = std::chrono::duration_cast<time_units>(clock_.now() - start_);
    metrics_->measure_add(name_, ns.count(), measurement_t::TIME_MICROSECONDS);
}

metrics::metrics(bool enabled)
    : storage_(new metrics_array),
      enabled_(enabled)
{
}

/* Copy constructor */
metrics::metrics(metrics const &m)
{
    storage_ = m.storage_;
    enabled_ = m.enabled_;
}

/* Move constructor */
metrics::metrics(metrics const &&m)
{
    storage_ = m.storage_;
    enabled_ = m.enabled_;
}

/* Copy assignment operator */
metrics& metrics::operator=(metrics const& m)
{
    enabled_ = m.enabled_;
    storage_ = m.storage_;
    return *this;
}

/* Move assignment operator */
metrics& metrics::operator=(metrics&& m)
{
    storage_ = m.storage_;
    enabled_ = m.enabled_;
    return *this;
}

std::unique_ptr<autochrono> metrics::measure_time_impl(const char* const name)
{
    return std::make_unique<autochrono>(this, name);
}

void metrics::measure_add_impl(const char* const name, int64_t value, measurement_t type)
{
    auto it = std::find_if(storage_->begin(), storage_->end(), [&](const measurement& m)
    {
        return (m.name_ == name);
    });

    if (it == storage_->end())
    {
        storage_->emplace_back(name, value, type);
    }
    else
    {
        it->value_ += value;
        it->calls_++;
    }
}


boost::optional<measurement &> metrics::find(const char* const name)
{
    auto it = std::find_if(storage_->begin(), storage_->end(), [&](const measurement& m)
    {
        return (m.name_ == name);
    });

    return (it != storage_->end() ? *it : boost::optional<measurement &>());
}

std::string metrics::to_string()
{
    std::ostringstream buf;

    buf << "{";
    if (enabled_ && !storage_->empty())
    {
        buf << R"("Mapnik":{)";
        for (auto it = storage_->begin(); it != storage_->end(); it++)
        {
            buf << R"(")" << it->name_ << R"(":)";
            switch (it->type_)
            {
                case (measurement_t::VALUE):
                    buf << it->value_;
                    break;
                case (measurement_t::TIME_MICROSECONDS):
                    buf << "{";
                    buf << R"(")" << measurement_str[measurement_t::TIME_MICROSECONDS];
                    buf << R"(":)" << it->value_;
                    buf << R"(,")" << measurement_str[measurement_t::CALLS];
                    buf << R"(":)" << it->calls_;
                    buf << "}";
                    break;
                default:
                    break;
            }

            if (std::next(it) != storage_->end())
            {
                buf << ",";
            }
        }
        buf << "}";
    }

    buf << "}";
    return buf.str();
}

} //namespace mapnik

#endif //MAPNIK_METRICS