# for each resource
# capacity=max number of cached objects
# cache=true
# geo=lat,lon
#
# for each cached data object:
# replicated:<resc name>=replicated time padded with leading 0s to 11 chars

acPreprocForDataObjOpen {
  on($writeFlag == "0") {
    writeLine("serverLog", "looking for nearest cache resc for $objPath");
    *rescs = sortRescByDistance($clientAddr);
    writeLine("serverLog", "found resources");
    foreach(*resc in *rescs) {
      (*dist, *name) = *resc;
      writeLine("serverLog", "*dist=*name");
    }

    foreach(*resc in *rescs) {
      (*_, *name) = *resc;
      writeLine("serverLog", "try using cache resc *name");
      *path = getCachePath(*name);
      writeLine("serverLog", "path for cache resc *name is *path");
      
      if($objPath like regex *path) {
        writeLine("serverLog", "path $objPath matches for cache resc *name is *path");
        *overflow = match getCacheMaxDataSize(*name) with
          | nothing => false
          | just(*maxDataSize) => $dataSize > *maxDataSize
        if(*overflow) {
          writeLine("serverLog", "data size is larger than cache maxDataSize: *path");  
        } else {
          *capacity = getCacheCapacity(*name);
          writeLine("serverLog", "capacity of resc *name is *capacity");
          if($dataSize > *capacity) {
            writeLine("serverLog", "data size is larger than cache capacity: $objPath");
          } else {
            msiSplitPath($objPath, *coll, *data);
            *replicatedKey = usedKey(*name);
            foreach(*r in select count(META_DATA_ATTR_VALUE) where COLL_NAME = *coll and DATA_NAME = *data and META_DATA_ATTR_NAME = *replicatedKey) {
              *count = int(*r.META_DATA_ATTR_VALUE);
            }
            updateUsedTime($objPath, *name);
            if (*count == 0) {
              writeLine("serverLog", "no copy on cache resc *name, try delayed replication to the cache resc");
              delayReplicate($objPath, $dataSize, *name, *capacity);
            } else {
              *found = false;
              foreach(*r2 in select DATA_REPL_STATUS where COLL_NAME = *coll and DATA_NAME = *data and RESC_NAME = *name) {
                *stat = *r2.DATA_REPL_STATUS;
                *found = true;
              }
              if (!*found) {
                writeLine("serverLog", "$objPath is not in the cache");
              } else {
                if (*stat == "0") {
                  writeLine("serverLog", "repl on cache resc *name is outdated, try delayed replication to the cache resc");
                  delayReplicate($objPath, $dataSize, *name, *capacity);
                }
              }
            }
            msiSetDataObjPreferredResc(join(*rescs, "%"));
            break;
          }
        }
      }
    }
  }
  or {
    writeLine("serverLog", "cache is not supported for this operation $oprType, $writeFlag");
  }
}

usedKey(*name) = "used:*name"

updateUsedTime(*objPath, *resc) {
  *replicatedKey = usedKey(*resc);
  *kvp.*replicatedKey = pad(str(double(time())), "0", 11);
  msiSetKeyValuePairsToObj(*kvp, *objPath, "-d");
}

getCacheLoad(*resc) {
  foreach(*r in select sum(DATA_SIZE) where RESC_NAME = *resc) {
    *sum = double(*r.DATA_SIZE);
  }
  *sum;
}

getCacheCapacity(*resc) {
  foreach(*r in select META_RESC_ATTR_VALUE where RESC_NAME = *resc and META_RESC_ATTR_NAME = "capacity") {
    *capacity = double(*r.META_RESC_ATTR_VALUE);
  }
  *capacity;
}

getCacheMaxDataSize : string -> maybe(double)
getCacheMaxDataSize(*resc) {
  *found = false;
  foreach(*r in select META_RESC_ATTR_VALUE where RESC_NAME = *resc and META_RESC_ATTR_NAME = "maxDataSize") {
    *maxDataSize = double(*r.META_RESC_ATTR_VALUE);
    *found = true;
  }
  if *found then just(*maxDataSize) else nothing;
}

getCachePath(*resc) {
  foreach(*r in select META_RESC_ATTR_VALUE where RESC_NAME = *resc and META_RESC_ATTR_NAME = "path") {
    *path = *r.META_RESC_ATTR_VALUE;
  }
  *path;
}

delayReplicate(*objPath, *dataSize, *resc, *capacity) {
  delay("<PLUSET>1s</PLUSET>") {
    *sum = getCacheLoad(*resc);
    writeLine("serverLog", "found *sum bytes of objects in resc *resc");
    *replicatedKey = usedKey(*resc);
    while(*sum + *dataSize > *capacity) {
      *found = false
      foreach(*r in select order_asc(META_DATA_ATTR_VALUE), DATA_NAME, COLL_NAME, DATA_SIZE where META_DATA_ATTR_NAME = *replicatedKey) {
        *data = *r.DATA_NAME;
        *coll = *r.COLL_NAME;
        *sum = *sum - double(*r.DATA_SIZE);
        *found = true;
      }
      if(*found) {
        writeLine("serverLog", "trimming oldest data object on resc *coll/*data");
        msiDataObjTrim("*coll/*data", *resc, "", "", "", *status);      
      } else {
        break;
      }
    }
    writeLine("serverLog", "replicating data object *objPath");
    msiDataObjRepl(*objPath, "destRescName=*resc", *status);
    writeLine("serverLog", "replicated data object *objPath");
  }
}

printAllCacheStatus() {
  foreach(*r in select RESC_NAME where META_RESC_ATTR_NAME = "cache" and META_RESC_ATTR_VALUE = "true") {
    printCacheStatus(*r.RESC_NAME);
  }
}

printCacheStatus(*resc) {
  writeLine("stdout", "[*resc]\ncapacity: " ++ str(getCacheCapacity(*resc)) ++ "\nload: " ++ str(getCacheLoad(*resc)));
}

