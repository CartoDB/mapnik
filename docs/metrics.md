# Metrics

## Enabling metrics

Capturing metrics is enabled during the configuration step:

```bash
./configure ENABLE_METRICS=True
```

## Usage

### Mapnik

The metrics object is defined in `mapnik/metrics.hpp` and it's designed to be shared between objects but beware that it doesn't have any internal synchronization, so if you want to share it between threads you'll need to use some external synchronization method.

Currently the metrics are being added in `include/mapnik/feature_style_processor_impl.hpp` which shares them, via composition with `agg_renderer`, `grid_renderer` and  which share them with the `grid` and `image` classes.

The metrics are stored in a vector for fast access and they differ by its `name_` only. For performance reasons the internal comparison (to add new measurements) is done by comparing the pointer itself (`char *`) and, since it doesn't store a copy of the name the string has to outlive the metric, so using a `static char*` is recommended.

The metrics are output in a tree under the `Mapnik` property:
```json
{
   "Mapnik":{
      "Datasource":{
         "Time (us)":10897,
         "Calls":2
      },
      "Setup":{
         "Time (us)":10920,
         "Calls":2
      },
      "Agg_PPolygonS":{
         "Time (us)":10,
         "Calls":2
      },
      "Features_cnt_Polygon":4,
      "Render_Style":{
         "Time (us)":3650,
         "Calls":4
      },
      "Agg_PLineS":{
         "Time (us)":10,
         "Calls":2
      },
      "Render":{
         "Time (us)":3831,
         "Calls":2
      },
      "Agg_PMS_AttrCache_Miss":1,
      "Agg_PMS_ImageCache_Miss":41,
      "Agg_PMarkerS":{
         "Time (us)":3217,
         "Calls":469
      },
      "Features_cnt_Point":938,
      "All":{
         "Time (us)":14830,
         "Calls":1
      }
   }
}
```

There are two main ways to add a new metric:

#### Time

Returns  an unique_ptr to class that will automatically add the metric once it's out of scope. This call will increase the "Time (us)" metric with the time passed and increase the "Calls" metric. In order to remove warnings when the compilation is done without `ENABLE_METRICS` please capture the metric with `METRIC_UNUSED auto`.
    For example:

```c++
    for (int i = 0; i < 10; i++)
    {
        METRIC_UNUSED auto t = metrics_.measure_time("Setup");
        //do_stuff
    }
```
Will lead to something like this:
```json
{
   "Mapnik":{
      "Setup":{
         "Time (us)":225636,
         "Calls":10
      }
    }
}
```

#### Other metrics

This is designed to add a different metric or edit an existing one. It uses the `measure_add` function:

```c++
    void measure_add(const char* const name, int64_t value = 1,
                     measurement_t type = measurement_t::VALUE)
```
  The `value` parameter will be added (or substracted if negative) to the value of the metric. The `measurement_t` type is used only for new metrics with `TIME_MICROSECONDS` returning "Time (us)" and `VALUE` returning the name of the metric as a final leaf.

Example:
```c++
    metrics_.measure_add("Features_cnt_Point", features_count);
```

```json
{
   "Mapnik":{
      "Features_cnt_Point":938
   }
}
```


### Node-mapnik

Currently only `mapnik.Grid` and `mapnik.Image` are supported. Both objects have a new attribute `metrics_enabled` and a method to access them `get_metrics`, so you can do something like this:

```javascript
    image.metrics_enabled = true;
    map.render(image, options, function(err, image) {
        if (!err) console.log(image.get_metrics());
    }
```
