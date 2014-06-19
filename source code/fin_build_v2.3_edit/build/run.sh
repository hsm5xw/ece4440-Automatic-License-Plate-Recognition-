#!/bin/bash

# file: run.sh

# name: Hong Moon (hsm5xw) & Zachary Zydron (zmz3af)

# description: automate building the project

./trainSVM 23 23 train/plate_regions/ train/non_plate_regions/

./ANPR 1
