# FROM gcc:13
FROM ubuntu:22.04
USER root

RUN apt-get update -y
# RUN apt-get upgrade -y

RUN apt-get install -y \
    xz-utils \
    wget \
    libncurses5 \
    cmake \
    make \
    libstdc++6-12-dbg

# installing LLVM 17
RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz 
RUN tar xf clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz

# RUN cp -r clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04/* /usr/local/
RUN cp -r clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04/* /usr/
# RUN export "LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH"
ENV LD_LIBRARY_PATH=/usr/local/lib:/usr/lib$LD_LIBRARY_PATH


RUN mkdir -p /usr/src/mole
WORKDIR /usr/src/mole
COPY . .

RUN cmake -B ./build -DCMAKE_BUILD_TYPE=Release -DCOVERAGE=OFF -DTESTS=OFF
RUN cmake --build ./build --config Release --target molec
RUN cp ./build/molec /usr/local/bin

# installation: docker build -t mole .