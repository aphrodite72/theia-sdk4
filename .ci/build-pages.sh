#!/bin/sh
# Input is mounted at /repo. Output is mounted at /build.

set -e

# We use the built-in `index` preprocessor to rename our `README.md` files
# to `index.md`, but it doesn't currently fixup links
# https://github.com/rust-lang/mdBook/issues/984
find /repo/ -type f -name "*.md" ! -iname "SUMMARY.md" -exec sed -ri 's/(\[.*\])\((.*\/)?(readme\.md)(#.*)?\)/\1\(\2index\.md\4\)/gI' {} \;

# Generate mdbook
mdbook build /repo -d /build

# Generate schemas
mkdir -p /build/schemas
generate-schema-doc --no-minify --config with_footer=false /repo/schemas/module-config.schema.json /build/schemas/module-config.schema.html
tidy --tidy-mark false -i -o /build/schemas/module-config.schema.html /build/schemas/module-config.schema.html || true
generate-schema-doc --no-minify --config with_footer=false /repo/schemas/runtime-config.schema.json /build/schemas/runtime-config.schema.html
tidy --tidy-mark false -i -o /build/schemas/runtime-config.schema.html /build/schemas/runtime-config.schema.html || true

# Generate doxygen
mkdir -p /build/doxygen
doxygen doxygen.conf
