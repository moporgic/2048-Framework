# 2048-Framework

Framework for 2048 & 2048-Like Games (C++ 11)

## Basic Usage

To make the sample program:
```bash
make # see makefile for details
```

To run the sample program:
```bash
./2048 # by default the program runs 1000 games
```

To specify the total games to run:
```bash
./2048 --total=100000
```

To display the statistics every 1000 episodes:
```bash
./2048 --total=100000 --block=1000 --limit=1000
```

To specify the total games to run, and seed the environment:
```bash
./2048 --total=100000 --place="seed=12345" # need to inherit from random_agent
```

To save the statistics result to a file:
```bash
./2048 --save=stats.txt
```

To load and review the statistics result from a file:
```bash
./2048 --load=stats.txt
```

## Advanced Usage

To initialize the network, train the network for 100000 games, and save the weights to a file:
```bash
weights_size="65536,65536,65536,65536,65536,65536,65536,65536" # 8x4-tuple
./2048 --total=100000 --block=1000 --limit=1000 --slide="init=$weights_size save=weights.bin" # need to inherit from weight_agent
```

To load the weights from a file, train the network for 100000 games, and save the weights:
```bash
./2048 --total=100000 --block=1000 --limit=1000 --slide="load=weights.bin save=weights.bin" # need to inherit from weight_agent
```

To train the network for 1000 games, with a specific learning rate:
```bash
weights_size="65536,65536,65536,65536,65536,65536,65536,65536" # 8x4-tuple
./2048 --total=1000 --slide="init=$weights_size alpha=0.0025" # need to inherit from weight_agent
```

To load the weights from a file, test the network for 1000 games, and save the statistics:
```bash
./2048 --total=1000 --slide="load=weights.bin alpha=0" --save="stats.txt" # need to inherit from weight_agent
```

To perform a long training with periodic evaluations and network snapshots:
```bash
weights_size="65536,65536,65536,65536,65536,65536,65536,65536" # 8x4-tuple
./2048 --total=0 --slide="init=$weights_size save=weights.bin" # generate a clean network
for i in {1..100}; do
	./2048 --total=100000 --block=1000 --limit=1000 --slide="load=weights.bin save=weights.bin alpha=0.0025" | tee -a train.log
	./2048 --total=1000 --slide="load=weights.bin alpha=0" --save="stats.txt"
	tar zcvf weights.$(date +%Y%m%d-%H%M%S).tar.gz weights.bin train.log stats.txt
done
```

## Author

Hung Guei, [Computer Games and Intelligence (CGI) Lab](https://cgilab.nctu.edu.tw/), NYCU, Taiwan
