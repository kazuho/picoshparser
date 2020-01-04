picoshparser
===

A (experimental) parser of [Structured Headers](https://httpwg.org/http-extensions/draft-ietf-httpbis-header-structure.html).

The design goals are:
* high performance
* correctness
* enforce correct use through API design

To achieve high performance, we follow the design principles applied by [picohttpparser](https://github.com/h2o/picohttpparser): minimize the number of instructions per character, minimize the number of branch mispredictions.
In addition, the implementation looks into minimizing the cost of skipping unneeded parts of the header field, as it would often the case that the application would recognize and use only a small portion of a header field (consider Cache-Control).

See test.c to understand the API.
