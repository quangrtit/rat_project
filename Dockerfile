FROM centos:7 AS builder

# Sửa repo để dùng vault.centos.org (CentOS 7 đã EOL)
RUN sed -i 's/mirror.centos.org/vault.centos.org/g' /etc/yum.repos.d/CentOS-*.repo && \
    sed -i 's/^#.*baseurl=http/baseurl=http/g' /etc/yum.repos.d/CentOS-*.repo && \
    sed -i 's/^mirrorlist=http/#mirrorlist=http/g' /etc/yum.repos.d/CentOS-*.repo

# Cài gói cần thiết
RUN yum update -y && \
    yum install -y centos-release-scl && \
    sed -i 's/mirror.centos.org/vault.centos.org/g' /etc/yum.repos.d/CentOS-SCLo-*.repo && \
    sed -i 's/^#.*baseurl=http/baseurl=http/g' /etc/yum.repos.d/CentOS-SCLo-*.repo && \
    sed -i 's/^mirrorlist=http/#mirrorlist=http/g' /etc/yum.repos.d/CentOS-SCLo-*.repo && \
    yum install -y epel-release && \
    yum install -y devtoolset-11-gcc devtoolset-11-gcc-c++ make wget git unzip tar perl perl-IPC-Cmd perl-core && \
    yum clean all
# install rpm build
RUN yum install -y rpm-build

# Cài CMake 3.28
RUN wget https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-x86_64.tar.gz && \
    tar -zxvf cmake-3.28.0-linux-x86_64.tar.gz && \
    mv cmake-3.28.0-linux-x86_64 /usr/local/cmake && \
    ln -s /usr/local/cmake/bin/cmake /usr/bin/cmake && \
    rm cmake-3.28.0-linux-x86_64.tar.gz

# Cài Boost 1.83 (static)
RUN source /opt/rh/devtoolset-11/enable && \
    wget https://sourceforge.net/projects/boost/files/boost/1.83.0/boost_1_83_0.tar.gz/download -O boost_1_83_0.tar.gz && \
    tar -zxvf boost_1_83_0.tar.gz && \
    cd boost_1_83_0 && \
    ./bootstrap.sh --prefix=/usr/local --with-libraries=system,thread && \
    ./b2 link=static runtime-link=static -j$(nproc) install && \
    cd .. && rm -rf boost_1_83_0

# Cài OpenSSL 3.0.14 (static)
RUN source /opt/rh/devtoolset-11/enable && \
    wget https://www.openssl.org/source/openssl-3.0.14.tar.gz && \
    tar -zxvf openssl-3.0.14.tar.gz && \
    cd openssl-3.0.14 && \
    ./config no-shared --prefix=/usr/local/openssl-3.0.14 && \
    make -j$(nproc) && make install && \
    cd .. && rm -rf openssl-3.0.14

# Cài Protobuf 3.21.12 (static)
RUN source /opt/rh/devtoolset-11/enable && \
    wget https://github.com/protocolbuffers/protobuf/releases/download/v21.12/protobuf-cpp-3.21.12.tar.gz && \
    tar -zxvf protobuf-cpp-3.21.12.tar.gz && \
    cd protobuf-3.21.12 && \
    ./configure --prefix=/usr/local/protobuf-static --enable-shared=no && \
    make -j$(nproc) && make install && \
    cd .. && rm -rf protobuf-3.21.12

# Cài Catch2 2.13.0 (static)
RUN source /opt/rh/devtoolset-11/enable && \
    git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    git checkout v2.13.0 && \
    cmake -B build -S . -DBUILD_SHARED_LIBS=OFF -DCATCH_BUILD_TESTING=OFF \
          -DCMAKE_CXX_FLAGS="-Wno-error=deprecated-declarations" \
          -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build build --target install && \
    cd .. && rm -rf Catch2

# Copy mã nguồn RAT
COPY . /rat_project
WORKDIR /rat_project

# Thiết lập môi trường khi vào container
ENV PATH=/opt/rh/devtoolset-11/root/usr/bin:/usr/local/cmake/bin:$PATH
ENV LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/usr/local/openssl-3.0.14/lib:/usr/local/protobuf-static/lib
RUN echo "source /opt/rh/devtoolset-11/enable" >> ~/.bashrc

# Khi chạy container thì vào shell
CMD ["/bin/bash"]
