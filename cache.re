# for each resource
# capacity=max number of cached objects
# cache=true
# geo=lat,lon
#
# for each cached data object:
# replicated:<resc name>=replicated time padded with leading 0s to 11 chars

acPreprocForDataObjOpen {
  on($writeFlag == "0") {
    *rescs = sortRescByDistance($clientAddr);
    if(size(*rescs) != 0) {
      foreach(*r in select count(RESC_NAME) where COLL_NAME = *coll and DATA_NAME = *data) {
        *count = int(*r.RESC_NAME);
      }
      if (*count == 0) {
        delayReplicate($objPath, *nearest);
      }
      msiSetDataObjPreferredResc(join(*rescs, "%"));
    }
  }
  or {
  }
}

delayReplicate(*objPath, *resc) {
  delay("<PLUSTET>1s</PLUSET>") {
    foreach(*r in select count(DATA_NAME) where RESC_NAME = *resc) {
      *count = int(*r.DATA_NAME);
    }
    foreach(*r in select META_RESC_ATTR_VALUE where RESC_NAME = *resc, META_RESC_ATTR_NAME = "capacity") {
      *capacity = int(*r.META_RESC_ATTR_VALUE);
    }

    *replicatedKey = "replicated:*resc";
    if(*count >= *capacity) {
      foreach(*r in select order_asc(META_DATA_ATTR_VALUE), DATA_NAME, COLL_NAME where META_DATA_ATTR_NAME = *replicatedKey) {
        *data = *r.DATA_NAME;
	*coll = *r.COLL_NAME;
      }
      msiDataObjTrim("*coll/*data", *resc, "", "", "", *status);
    }
    
    msiDataObjRepl(*objPath, "destRescName=*resc", *status);
    *kvp.*replicatedKey = pad(str(double(time())), "0", 11);
    msiAssociateKeyValuePairsToObj(*kvp, *objPath, "-d");
  }
}