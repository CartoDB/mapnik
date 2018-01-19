# Metrics

## Enabling metrics

Capturing metrics is enabled during the configuration step:

```bash
./configure ENABLE_METRICS=True
```

## Usage

### Mapnik

The metrics object is defined in `mapnik/metrics.hpp` and it's designed to be shared between objects. Currently the metrics are being added in `include/mapnik/feature_style_processor_impl.hpp` which shares them, via composition with `grid_renderer` and `agg_renderer` which share them with the `grid` and `image` classes.

The metrics are added in a tree by the name, using `.` as separators. For example:
```json
{
   "Mapnik":{
      "Time (us)":7229,
      "Calls":1,
      "Setup":{
         "Time (us)":2256,
         "Calls":1,
         "Datasource: Get Feature":{
            "Time (us)":2231,
            "Calls":1
         }
      },
      "Render":{
         "Time (us)":4886,
         "Calls":1,
         "Style":{
            "Time (us)":4883,
            "Calls":1,
            "features":284
         }
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
        METRIC_UNUSED auto t = metrics_.measure_time("Mapnik.Setup");
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
    inline void measure_add(std::string const& name, int64_t value = 1,
                            measurement_t type = measurement_t::VALUE)
```
  The `value` parameter will be added (or substracted if negative) to the value of the metric. The `measurement_t` type is used only for new metrics with `TIME_MICROSECONDS` returning "Time (us)" and `VALUE` returning the name of the metric as a final leaf.

Example:
```c++
    metrics_.measure_add("Mapnik.Render.Style.features", features_count);
```

```json
{
   "Mapnik":{
      "Render":{
         "Style":{
            "features":284
         }
      }
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
