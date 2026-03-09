Common prefix symbols (to avoid name collision with methods):
  prefix e_ : enumeration case
  prefix m_ : member variable
  prefix s_ : static (non-instance) constant or variable
  prefix w_ : widget member variable (derived from wxWidgets class)

Common suffix symols (to indicate the variable content):
  suffix _a : array
  suffix _c : count (int, size_t)
  suffix _d : *Data (this is not a pointer; use _p or _n for pointer to *Data)
  suffix _m : ordered map, unordered map, set
  suffix _n : nullable (e.g., nullptr, nullopt, value out of range)
  suffix _p : non-null (e.g., pointer which is always non-null)
  suffix _t : typedef

