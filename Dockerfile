FROM emscripten/emsdk:3.1.17

RUN apt-get update
RUN apt-get install gperf
