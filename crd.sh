#!/bin/bash

make clean && make && LIBGL_ALWAYS_SOFTWARE=1 mgba-qt gbagame.gba
