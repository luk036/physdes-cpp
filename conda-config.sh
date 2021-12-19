export CONDA_PREFIX=/workspace/miniconda3
export PATH="$CONDA_PREFIX/bin:$PATH" # overwrite the system python
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 90
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 90
