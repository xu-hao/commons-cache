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

    if(size(*rescs) != 0) {
      (*_, *name) = elem(*rescs, 0);
      writeLine("serverLog", "try using cache resc *name");
      msiSplitPath($objPath, *coll, *data);
      *replicatedKey = usedKey(*name);
      foreach(*r in select count(META_DATA_ATTR_VALUE) where COLL_NAME = *coll and DATA_NAME = *data and META_DATA_ATTR_NAME = *replicatedKey) {
        *count = int(*r.META_DATA_ATTR_VALUE);
      }
      updateUsedTime($objPath, *name);
      if (*count == 0) {
        writeLine("serverLog", "no copy on cache resc *name, try delayed replication to the cache resc");
        delayReplicate($objPath, *name);
      } else {
        foreach(*r2 in select DATA_REPL_STATUS where COLL_NAME = *coll and DATA_NAME = *data and RESC_NAME = *name) {
          *stat = *r2.DATA_REPL_STATUS;
        }
        if (*stat == "0") {
          writeLine("serverLog", "repl on cache resc *name is outdated, try delayed replication to the cache resc");
          delayReplicate($objPath, *name);
        }
      }
      msiSetDataObjPreferredResc(join(*rescs, "%"));
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

delayReplicate(*objPath, *resc) {
  delay("<PLUSET>1s</PLUSET>") {
    foreach(*r in select sum(DATA_SIZE) where RESC_NAME = *resc) {
      *sum = int(*r.DATA_SIZE);
    }
    writeLine("serverLog", "found *sum bytes of objects in resc *resc");
    foreach(*r in select META_RESC_ATTR_VALUE where RESC_NAME = *resc and META_RESC_ATTR_NAME = "capacity") {
      *capacity = int(*r.META_RESC_ATTR_VALUE);
    }
    writeLine("serverLog", "capacity of resc *resc is *capacity");
    *replicatedKey = usedKey(*resc);
    while(*sum + $dataSize > *capacity) {
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