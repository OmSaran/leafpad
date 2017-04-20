#!/bin/bash

make clean;
sudo make uninstall;
make;
make install -strip;