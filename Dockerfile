FROM emscripten/emsdk:3.1.0

RUN apt-get update
RUN apt-get install gperf
