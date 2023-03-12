FROM emscripten/emsdk:3.1.33

RUN apt-get update
RUN apt-get install gperf
