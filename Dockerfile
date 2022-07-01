FROM ubuntu:16.04

ENV DEBIAN_FRONTEND noninteractive

RUN \
  apt update && apt upgrade -y && \
  apt install -y \
  build-essential \
  git \
  autoconf \
  cmake \
  python \
  zip \
  unzip \
  cpio \
  wget \
  vim \
  locales \
  lzop \
  bc \
  nano \
  libncurses5-dev \
  nfs-kernel-server \
  curl \
  python3 \
  python-lzma

RUN locale-gen --no-purge en_US.UTF-8
ENV LANG="en_US.UTF-8" LANGUAGE="en_US:en" LC_ALL="en_US.UTF-8" PATH="$PATH:/usr/local/node/bin"
WORKDIR "/usr/local"
RUN curl -o - https://nodejs.org/dist/v16.15.1/node-v16.15.1-linux-x64.tar.xz | tar Jxvf - && \
  ln -s /usr/local/node-v16.15.1-linux-x64 /usr/local/node && \
  /usr/local/node/bin/npm config set user 0 && \
  /usr/local/node/bin/npm config set unsafe-perm true

RUN mkdir -p /atomtools/build
WORKDIR "/atomtools/build"
RUN curl -o - https://buildroot.org/downloads/buildroot-2016.02.tar.gz | tar zxvf -

# Change to the buildroot directory
WORKDIR "/atomtools/build/buildroot-2016.02"

# Copy the entire source tree. Expect it to be mounted over when developing.
COPY . /src

RUN /src/buildscripts/setup_buildroot.sh
