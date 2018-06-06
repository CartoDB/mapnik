# CARTO Mapnik Changelog

## 3.0.15.10
**Release date**: 2018-XX-XX

Changes:
 - Improve clip calculation to avoid issues when using offset.


## 3.0.15.9
**Release date**: 2018-05-14

Changes:
 - Fix bug that failed to place lines in the MarkerSymbolizer if its length is 0. Sent upstream.

## 3.0.15.8

**Release date**: 2018-05-07

Changes:
 - Qualify ICU types explicitly (ICU 61 Compatibility). Sent upstream.
 - Move markers cache from a global scope to a per map scope. Multiple improvements to increase lookup speed and avoid the need for mutexes.
 - Apply overlap optimization to `load_map_string` too. Generalized it to be used also for points and text. Sent upstream.
 - Use pkg-config to find FreeType2 if available (Freetype 2.9.1+ compatibility). Sent upstream.

## 3.0.15.7

**Release date**: 2018-04-02

Changes:
 - Metrics: Add metrics in the grid renderer
 - Metrics: Compatibily fix for older compilers (80ecf2741 by @dprophet)
 - Fix build issue with MAPNIK_THREADSAFE disabled. Sent upstream.
 - Metrics: Internal rework (no mutex, no tree) to improve performance
 - Metrics: Add metrics for feature types
 - Metrics: Add metrics to the agg renderer
 - Metrics: Add metrics for cache misses
 - Markers Symbolizer Caches: Add a map extra parameter `markers_symbolizer_caches_disabled` that can be set to true to disable them.
 - Markers cache: Use pointer to cached attributes as part of the key. Also avoid a double free.

## 3.0.15.6

**Release date**: 2018-03-06

Changes:
 - Fixed some issues with memory management of cache and calculation of cache keys.
 - Increased cache sampling_rate to 8, which makes visual tests more robust.
 - Bring back osx visual tests checks

## 3.0.15.5

**Release date**: 2018-03-01

Changes:
 - Improve precision of the marker symbolizer cache.
 - Travis: Run visual test with overwrite and upload differences to transfer.sh

## 3.0.15.4

**Release date**: 2018-02-22

Changes:
 - Use metrics without guards
 - Add docs/metrics.md
 - PostGIS: Variables in postgis SQL queries must now additionally be wrapped in `!` (refs [#29](https://github.com/CartoDB/mapnik/issues/29), [mapnik/#3618](https://github.com/mapnik/mapnik/pull/3618)):
```sql
-- Before
SELECT ... WHERE trait = @variable

-- Now
SELECT ... WHERE trait = !@variable!
```
 - Add an internal cache to the marker symbolizer in the agg renderer.

## 3.0.15.3

**Release date**: 2018-01-19

Changes:
 - Switch back to harfbuzz 1.3.0

## 3.0.15.2

**Release date**: 2018-01-04

Changes:
 - Enable metrics by default [2036217](https://github.com/CartoDB/mapnik/commit/20362178de4aab1d758ea19137037fcbb470ef83).


## 3.0.15.1

**Release date**: 2018-01-03

Changes:
 - Add this very changelog
 - Add metrics support. See [#3767](https://github.com/mapnik/mapnik/pull/3767)
 - Add support for U_ICU_VERSION_MAJOR_NUM >= 59 [#3729](https://github.com/mapnik/mapnik/issues/3729)
 - Fix travis CI [#24](https://github.com/CartoDB/mapnik/pull/24)


## 3.0.15.0

**Release date**: 2017-12-22

Vanilla Mapnik 3.0.15 in which the rest of the code is based upon. See https://github.com/mapnik/mapnik/blob/v3.0.15/CHANGELOG.md
