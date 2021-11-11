FROM gitpod/workspace-full

USER root
# Install util tools.

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test

RUN apt-get update \
 && apt-get install -y \
  apt-utils \
  aria2 \
# c++ stuff \
  gcc-11 \
  g++-11 \
  ccache \
  cmake-curses-gui \
  cppcheck \
  doctest-dev \
  kcachegrind-converters \
  kcachegrind \
  lcov \
  libbenchmark-dev \
  libboost-dev \
  libfmt-dev \
#  libjsoncpp-dev \
#  libopenblas-dev \
  librange-v3-dev \
  libspdlog-dev \
  ninja-build \
# utilities (not ripgrep, gh) \
  asciinema \
  bat \
  byobu \
  curl \
  elinks \
  fd-find \
  fish \
  mdp \
  ncdu \
  neofetch \
  patat \
  pkg-config \
  ranger \
  w3m \
# just for fun (not cmatrix) \
  cowsay \
  figlet \
  fortune \
  toilet \
  tty-clock

RUN apt-get clean && rm -rf /var/cache/apt/* && rm -rf /var/lib/apt/lists/* && rm -rf /tmp/*
