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
#include "catch.hpp"

#include <mapnik/metrics.hpp>

using namespace mapnik;

TEST_CASE("metrics")
{
    SECTION("Construtors and assigments")
    {
        metrics m(false);
        CHECK(m.enabled_ == false);
        CHECK(m.to_string() == "{}");
        m.enabled_ = true;
        m.measure_add("test");

        metrics m2(m);
        CHECK(m.to_string() == m2.to_string());
        m.measure_add("new test", 25);
        CHECK(m.to_string() == m2.to_string());
        m2.measure_add("other test", 100, measurement_t::TIME_MICROSECONDS);
        CHECK(m.to_string() == m2.to_string());
    }

    SECTION("Find")
    {
        metrics m(true);
        m.measure_add("exists", 10);
        m.measure_add("prefix.sub");

        CHECK(m.find("exists"));
        CHECK(!m.find("doesn't exists"));
        CHECK(m.find("prefix.sub"));
    }


    SECTION("Additions")
    {
        metrics m(true);
        m.measure_add("test");
        auto it = m.find("test");
        REQUIRE(it);
        CHECK(it->value_ == 1);
        CHECK(it->calls_ == 1);
        CHECK(it->type_ == measurement_t::VALUE);

        m.measure_add("test");
        CHECK(it->value_ == 2);
        CHECK(it->calls_ == 2);

        m.measure_add("test", 20);
        CHECK(it->value_ == 22);
        CHECK(it->calls_ == 3);

        m.enabled_ = false;
        m.measure_add("test");
        CHECK(it->value_ == 22);
        CHECK(it->calls_ == 3);

        m.measure_add("test_disabled");
        CHECK(!m.find("test_disabled"));
    }

    SECTION("Time")
    {
        metrics m(true);
        {
            auto t = m.measure_time("new_measurement");
            usleep(1000); // microseconds
        }
        auto it = m.find("new_measurement");
        REQUIRE(it);
        CHECK(it->value_ > 1000);
    }

    SECTION("Time: Measurements with the same name are added")
    {
        static const int times = 8;
        metrics m(true);
        for (uint i = 0; i < times; i++)
        {
            auto time_metric = m.measure_time("same_measurement");
            auto time_metric2 = m.measure_time("other measurement");
            usleep(100); // microseconds
        }
        auto it = m.find("same_measurement");
        REQUIRE(it);
        CHECK(it->value_ > 100 * times);
        CHECK(it->calls_ == times);
    }

    SECTION("Copy and move")
    {
        metrics m(true);
        m.measure_add("measurement", 100, measurement_t::TIME_MICROSECONDS);

        metrics m2 = m;
        CHECK(m2.enabled_ == true);
        CHECK(m.to_string() == m2.to_string());

        metrics m3(true);
        m3.measure_add("other", 15);
        m3 = m;
        CHECK(m3.enabled_ == true);
        CHECK(m.to_string() == m3.to_string());

        metrics m4(std::move(m));
        CHECK(m4.enabled_ == true);
        CHECK(m4.to_string() == m3.to_string());
    }

    SECTION("to_string")
    {
        metrics m(true);
        CHECK(m.to_string() == "{}");
        m.measure_add("new_metric", 10);
        CHECK(m.to_string() == R"^({"Mapnik":{"new_metric":10}})^");
        m.measure_add("new_metric", 15);
        CHECK(m.to_string() == R"^({"Mapnik":{"new_metric":25}})^");

        metrics m2(true);
        m2.measure_add("metric.render.vortex", 10);
        CHECK(m2.to_string() == R"^({"Mapnik":{"metric.render.vortex":10}})^");

        m2.measure_add("metric.render", 100);
        CHECK(m2.to_string() == R"^({"Mapnik":{"metric.render.vortex":10,"metric.render":100}})^");

        metrics m3(true);
        m3.measure_add("metric.a", 10);
        m3.measure_add("metric.b", 20);
        CHECK(m3.to_string() == R"^({"Mapnik":{"metric.a":10,"metric.b":20}})^");

        metrics m4(true);
        m4.measure_add("time metric", 100, measurement_t::TIME_MICROSECONDS);
        m4.measure_add("time metric", 150, measurement_t::TIME_MICROSECONDS);
        CHECK(m4.to_string() == R"^({"Mapnik":{"time metric":{"Time (us)":250,"Calls":2}}})^");
    }

} //Test case

#endif /* MAPNIK_METRICS */
