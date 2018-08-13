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
      *replicatedKey = "replicated:*name";
      foreach(*r in select count(META_DATA_ATTR_VALUE) where COLL_NAME = *coll and DATA_NAME = *data and META_DATA_ATTR_NAME = *replicatedKey) {
        *count = int(*r.META_DATA_ATTR_VALUE);
      }
      if (*count == 0) {
        writeLine("serverLog", "no copy on cache resc *name, try delayed replication to the cache resc");
        delayReplicate($objPath, *name);
      }
      msiSetDataObjPreferredResc(join(*rescs, "%"));
    }
  }
  or {
    writeLine("serverLog", "cache is not supported for this operation $oprType, $writeFlag");
  }
}

delayReplicate(*objPath, *resc) {
  *replicatedKey = "replicated:*resc";
  *kvp.*replicatedKey = pad(str(double(time())), "0", 11);
  msiAssociateKeyValuePairsToObj(*kvp, *objPath, "-d");
  
  delay("<PLUSET>1s</PLUSET>") {
    foreach(*r in select count(DATA_NAME) where RESC_NAME = *resc) {
      *count = int(*r.DATA_NAME);
    }
    writeLine("serverLog", "found *count objects in resc *resc");
    foreach(*r in select META_RESC_ATTR_VALUE where RESC_NAME = *resc and META_RESC_ATTR_NAME = "capacity") {
      *capacity = int(*r.META_RESC_ATTR_VALUE);
    }
    writeLine("serverLog", "capacity of resc *resc is *capacity");

    if(*count >= *capacity) {
      foreach(*r in select order_asc(META_DATA_ATTR_VALUE), DATA_NAME, COLL_NAME where META_DATA_ATTR_NAME = *replicatedKey) {
        *data = *r.DATA_NAME;
	*coll = *r.COLL_NAME;
      }
      writeLine("serverLog", "trimming oldest data object on resc *coll/*data");
      msiDataObjTrim("*coll/*data", *resc, "", "", "", *status);
    }
    
    writeLine("serverLog", "replicating data object *objPath");
    msiDataObjRepl(*objPath, "destRescName=*resc", *status);
    writeLine("serverLog", "replicated data object *objPath");
  }
}