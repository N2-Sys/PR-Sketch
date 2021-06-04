# Source Code of PR-Sketch

## Requirements

- Libs: pcap, iniparser, Eigen
	+ Download and configure each lib
	+ Put headers into `include` directory
	+ Put linked libraries into `lib` directory
- Datasets: CAIDA, UNIV1, UNIV2, Kosarak, Retail
	+ Download each dataset XXX into `data/XXX` directory

## Run

- Step 1: compile
	+ Use `make all` to generate all executable files
- Step 2: preprocess datasets with pcap files (i.e., CAIDA, UNIV1, and UNIV2)
	+ Modify `config.ini` to specify the dataset
	+ Run `./trace_preprocess config.ini`
	+ Run `./dataset_split config.ini epochidx` to get data of an epoch
- Step 3: run experiment for sketch XXX
	+ Modify `sketching/utils/util.h` to change configuration if needed and then re-compile as step 1
	+ Run `./XXX_test memorycost` to evalutate the sketch
