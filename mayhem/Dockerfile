FROM --platform=linux/amd64 ubuntu:22.04 AS builder

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        wget gnupg lsb-release software-properties-common && \
    wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
    echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" > /etc/apt/sources.list.d/llvm.list && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        g++ cmake git-core build-essential \
        clang-19 llvm-19-dev libclang-19-dev libclang-cpp19-dev && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100 && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100

COPY . /repo
WORKDIR /repo/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=/install
RUN make -j8
RUN make install
RUN clang++-19 /repo/mayhem/fuzz_unquoteString.cpp \
    -fsanitize=fuzzer,address \
    -L /install/lib/ -l:ClazyPlugin.so \
    -L /usr/lib/llvm-19/lib/ -lclang-cpp \
    -Wl,-rpath,/usr/lib/llvm-19/lib/ \
    -o /fuzz

FROM ubuntu:22.04 AS package
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        wget gnupg && \
    wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc && \
    echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" > /etc/apt/sources.list.d/llvm.list && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y libclang-cpp19 libclang1-19 libllvm19
COPY --from=builder /install /install
COPY --from=builder /fuzz /fuzz_unquoteString
ENV LD_LIBRARY_PATH=/install/lib:/usr/lib/llvm-19/lib
