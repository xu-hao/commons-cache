# commons-cache


## Installation
Copy `cache.dvm`, `cache.re`, `geo.re`, `utils.re` to irods config dir (default `/etc/irods`)

Add `cache,geo,utils` in front of `core` in `server_config.json`





## Usage

To make a resource as cache resource set the following AVU

`capacity=max number of cached objects`

`cache=true`

`geo=lat,lon`

For each object that's been cached, the rules add the following metadata

replicated:<resc name>=replicated time padded with leading 0s to 11 chars
