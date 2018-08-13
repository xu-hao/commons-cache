data geo =
  | geo : integer * integer -> geo

getIpGeo(*addr) {
  geo(0,0); # replace with real geolocation ip lookup service
}

getRescGeo(*resc) {
  foreach(*r in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = "geo" and RESC_NAME = *resc) {
    *geo = *r.META_RESC_ATTR_VALUE;
  }
  *ret = split(*get, ",");
  geo(elem(*ret,0), elem(*ret,1));
}

distance(*geo0, *geo1) {
  geo(*lat0, *lon0) = *geo0;
  geo(*lat1, *lon1) = *geo1;
  sqrt(sqr(*lat0 - *lat1) + sqr(*lon0 - *lon1));
}

sortRescByDistance(*addr) {
  *geoIp = getIpGeo(*addr);
  *rescList = list();
  foreach(*r in select RESC_NAME where META_RESC_ATTR_NAME = "cache") {
    *resc = *r.RESC_NAME
    *geo = getRescGeo(*resc);
    *dist = distance(*geoIp, *geo);
    *rescList = cons((*dist, *resc), *rescList);
  }
  quicksort(*list);
  *list;
}
