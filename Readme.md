

# autocereal

This is a C++26 project which I'm currently using with a freshly-built gcc-16.
it's designed to give cereal the ability to serialize any class without
having to write load/save functions for them.

## What's here RIGHT NOW

Serializing structs with public members seems to work. Serializing shared pointers
of those structs seems to work. Needs more testing, but the concept appears to be
sound. Serializing with private members also now works and does not seem to
require `cereal::access` friend access to work.

Including the autocereal header also gets you `to_json`, `from_json`, `to_xml` and
`from_xml` functions. These will work with either strings or Streams. These
functions are two-liners that leverage `to_output_archive` and `from_input_archive`.
That means if you're using a third party cereal archiver like the YAML one,
you could write your own to and from functions for it if you want to extend
this functionality to that archive type.

Now with JsonSchema! Need a schema for your JSON? JsonSchema! Just create a
JsonSchema templated with the class you want to generate your schema for and
then call to_string on that object. JsonSchema currently only generates a
schema for one class, so it doesn't handle refs, but it works pretty well
for the simple classes I'm planning to use for data models. The tests
in SchemaTest.cpp illustrate its usage and contains some expected output.

I'll probably end up adding another object that accumulates JsonSchema
objects from a typelist or something that could change types in the
JsonSchema objects it holds such that objects it knows about become
refs.

Third party contributed module currently only supports ClassSingleton
and JsonSchema due to current cereal limitations. I also haven't
tested it beyond verifying that it compiles now. I can probably
add the various to/from functions to it, but I'm not using modules
and don't really want to write the tests for them right now.

# Limitations

No, it just seems to work.

# Using

Just include the header `fr/autocereal/autocereal.h` and serialize stuff. See `tests/AcSerialize.cpp`
and `tests/ToFromFunctions.cpp`. `ToFromFunctions.cpp` outputs what the
JSON and XML output of the serialization function would look like.
