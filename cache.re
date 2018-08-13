acPreprocForDataObjOpen {
  on($writeFlag == "0") {
    *rescs = sortRescByDistance($clientAddr);
    if(*rescs != "") {
      msiSetDataObjPreferredResc(join(*rescs, "%"));
    }
  }
  or {
  }
}
