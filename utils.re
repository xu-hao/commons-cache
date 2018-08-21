join(*rescs, *delim) {
  *s = "";
  foreach(*resc in *rescs) {
    (*_, *rescstr) = *resc;
    if (*s == "") {
      *s = *rescstr;
    } else {
      *s = "*s*delim*rescstr";
    }
  }
  *s;
}

quicksort : input output list (double * string) -> integer
quicksort(*list) {
  quicksortRange(*list, 0, size(*list));
}

quicksortRange(*list, *start, *finish) {
  *n = *finish - *start;
  if(*n > 1) {
    (*pivot, *_) = elem(*list, *start);
    *j = *start + 1;
    for(*i = *start + 1; *i < *finish; *i = *i + 1) {
      (*curr, *_) = elem(*list, *i);
      if (*curr < *pivot) {
        swap(*list, *j, *curr);
	      *j = *j + 1;
      }
    }
    swap(*list, *j - 1, *start);
    quicksortRange(*list, *start, *j - 1);
    quicksortRange(*list, *j, *finish);
  }
}

swap(*list, *a, *b) {
  if(*a != *b) {
    *tmp = elem(*list, *a);
    setelem(*list, *a, elem(*list, *b));
    setelem(*list, *b, *tmp);
  }
}

pad(*s, *pad, *len) {
  *ret = *s;
  *n = strlen(*s);
  *st = strlen(*pad);
  *nst = (*len - *n) / *st;
  for(*i = 0; *i < *nst; *i = *i + 1) {
    *ret = "*pad*ret";
  }
  *ret;
}

data maybe(X) = 
  | just : X -> maybe(X)
  | nothing : maybe(X)

