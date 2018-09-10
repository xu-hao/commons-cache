# commons-cache


## Installation

### `cache.dvm`

Copy `cache.dvm` to irods config dir (default `/etc/irods`)

Add `cache` in front of `core` in `server_config.json`
```
"re_data_variable_mapping_set": [
                        "cache",
                        "core"
                    ]
```

### rules                   

Copy `cache.re`, `geo.re`, `utils.re` to irods config dir (default `/etc/irods`)

Add `cache,geo,utils` in front of `core` in `server_config.json`
```
"re_rulebase_set2": [
                        "utils",
                        "geo",
                        "cache",
                        "core"
                    ]
```




## Usage

To make a resource as cache resource set the following AVU

`capacity=max size of cached objects in bytes`

`cache=true`

`geo=lat,lon`

`path=regex for paths of objects to be cached by resc`

`maxDataSize=maximum data size` (optional)

For each object that's been cached, the rules add the following metadata

`replicated:<resc name>=replicated time padded with leading 0s to 11 chars`
