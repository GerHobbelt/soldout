FROM emscripten/emsdk:3.1.13

RUN apt-get update
RUN apt-get install gperf
