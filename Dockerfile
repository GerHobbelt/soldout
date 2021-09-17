FROM emscripten/emsdk:2.0.30

RUN apt-get update
RUN apt-get install gperf
