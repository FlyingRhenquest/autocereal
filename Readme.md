# autocereal

This is a C++26 project which I'm currently using with a freshly-built gcc-16.
it's designed to give cereal the ability to serialize any class without
having to write load/save functions for them.

## What's here RIGHT NOW

Serializing structs with public members seems to work. Serializing shared pointers
of those structs seems to work. Needs more testing, but the concept appears to be
sound. Serializing with private members also now works and does not seem to
require cereal::access friend access to work.

Including the autocereal header also gets you to\_json, from\_json, to\_xml and
from\_xml functions. These will work with either strings or Streams. These
functions are two-liners that leverage to\_output\_archive and from\_input\_archive.
That means if you're using a third party cereal archiver like the YAML one,
you could write your own to and from functions for it if you want to extend
this functionality to that archive type.

# Limitations

No, it just seems to work.

# Using

Just include the header and serialize stuff. See tests/AcSerialize.cpp
and tests/ToFromFunctions.cpp. ToFromFunctions.cpp outputs what the
JSON and XML output of the serialization function would look like.
