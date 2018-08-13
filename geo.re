data geo =
  | geo : double * double -> geo

split2(*s, *delim) {
  *ret = list(*s);
  *n = strlen(*s);
  for(*i = 0; *i < *n; *i = *i + 1) {
    if(substr(*s, *i, *i + 1) == *delim) {
      *ret = cons(substr(*s, 0, *i), split2(substr(*s, *i + 1, *n), *delim));
      break;
    }
  }
  *ret;
}

getIpGeo(*addr) {
  # use extreme ip lookup for development purpose
  msiCurlGetStr("https://extreme-ip-lookup.com/csv/*addr", *output);
  writeLine("serverLog", *output);
  *res = split2(*output, ",");
  # for(*i = 0; *i < size(*res); *i = *i + 1) {
  #   writeLine("stdout", "*i " ++ elem(*res, *i));
  # }
  *lat = double(elem(*res, 11));
  *lon = double(elem(*res, 12));
  geo(*lat, *lon);
}

getRescGeo(*resc) {
  foreach(*r in select META_RESC_ATTR_VALUE where META_RESC_ATTR_NAME = "geo" and RESC_NAME = *resc) {
    *geo = *r.META_RESC_ATTR_VALUE;
  }
  *ret = split(*geo, ",");
  geo(double(elem(*ret,0)), double(elem(*ret,1)));
}

distance(*geo0, *geo1) {
  geo(*lat0, *lon0) = *geo0;
  geo(*lat1, *lon1) = *geo1;
  ((*lat0 - *lat1) ^ 2 + (*lon0 - *lon1) ^ 2) ^^ 2;
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
  if(size(*rescList) > 0) {
    (_, *nearest) = elem(*rescList, 0);
    msiSplitPath($objPath, *coll, *data);
  }
  quicksort(*rescList);
  *rescList;
}

