# syntax=docker/dockerfile:1
FROM ubuntu:22.04 AS builder
RUN apt-get update \ 
 && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
    build-essential \ 
    cmake \ 
    git \ 
    libjsoncpp-dev \ 
    libssl-dev \ 
    uuid-dev \ 
    zlib1g-dev \ 
    libdrogon-dev \ 
    pkg-config

WORKDIR /workspace
COPY . .

# Build with source EIPScanner to support multiple architectures
RUN cmake --preset linux-eipscanner-src -S . -B build/eipscanner-src \ 
 && cmake --build build/eipscanner-src --config Release

FROM ubuntu:22.04 AS runtime
RUN useradd -m -u 1000 tcms
WORKDIR /opt/tcms-cip-sim
COPY --from=builder /workspace/build/eipscanner-src/tcms_cip_sim ./tcms_cip_sim
COPY config ./config
COPY views ./views
COPY --from=builder /workspace/build/eipscanner-src/generated ./generated

EXPOSE 8080
USER tcms
ENTRYPOINT ["/opt/tcms-cip-sim/tcms_cip_sim", "--conf", "config/config.json"]
