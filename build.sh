#!/bin/bash

set -e -x

mkdir -p "build"

# Compile src/html_entities.gperf

gperf src/html_entities.gperf > build/html_entities.c

# Build Snudown

emcc snudown.c src/autolink.c src/buffer.c src/markdown.c src/stack.c html/houdini_href_e.c html/houdini_html_e.c html/html.c build/html_entities.c \
-I src -I html \
--pre-js header.js --post-js footer.js \
-o build/snudown_emscripten.js \
-Oz -flto=full --closure 2 -DNDEBUG \
-s WASM=0 \
-s SINGLE_FILE=1 \
-s EXPORTED_FUNCTIONS=['_default_renderer','_wiki_renderer','_malloc','_free'] \
-s MALLOC=emmalloc \
-s ALLOW_MEMORY_GROWTH=1 \
-s STACK_SIZE=8192 \
-s INITIAL_MEMORY=65536 \
-s MAXIMUM_MEMORY=16777216 \
-s ENVIRONMENT=web \
-s TEXTDECODER=0 \
-s DYNAMIC_EXECUTION=0 \
-s MINIMAL_RUNTIME=2 \
-s STRICT=1 \
-Wno-tautological-compare \
-Wno-logical-op-parentheses \
-Wno-almost-asm \

# Remove line breaks which closure randomly inserts, breaking sed replacements
./node_modules/uglify-js/bin/uglifyjs ./build/snudown_emscripten.js -o ./build/snudown_oneline.js --comments

# Remove IIFE wrapper (for exports)
sed -r 's/\(function\(\)\{// ; s/\}\)\(\);//' ./build/snudown_oneline.js > ./build/snudown_nowrapper.js

# Minify
./node_modules/uglify-js/bin/uglifyjs ./build/snudown_nowrapper.js -o ./build/snudown_uglify.js \
--comments \
--toplevel \
-c negate_iife=false,passes=100,pure_getters,unsafe \
-m \
--mangle-props reserved=['markdown','markdownWiki','nofollow','target','tocIdPrefix','enableToc'] \
-b beautify=false,wrap_iife \
--define Module=undefined \

# Convert window exports to ES exports
sed -r 's/,window\.(\w+)=function/;export function \1/g' ./build/snudown_uglify.js > ./build/snudown_exports.js

# Generate dist files
mkdir -p "dist"
cp ./build/snudown_exports.js ./dist/snudown.js
cp ./snudown.d.ts ./dist/snudown.d.ts

rm -r "build"
