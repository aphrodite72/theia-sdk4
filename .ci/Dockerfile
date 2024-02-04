FROM alpine:3.18.3

# Install binary dependencies
RUN apk add --no-cache python3 py3-pip tidyhtml doxygen curl tar mdbook

# Install python dependencies
RUN python3 -m pip install json-schema-for-humans==0.44.4

# Set up dependencies
WORKDIR /app
COPY . /app
RUN chmod +x build-pages.sh
ENTRYPOINT ["/app/build-pages.sh"]