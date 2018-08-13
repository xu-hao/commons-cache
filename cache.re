acPreprocForDataObjOpen {
  on($writeFlag == "0") {
    *rescs = sortRescByDistance($clientAddr);
    msiSetDataObjPreferredResc(join(*rescs, "%"));
  }
  or {
  }
}
