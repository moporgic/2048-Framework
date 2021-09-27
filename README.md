# 2048-Framework

Framework for 2048 & 2048-like Games (C++ 11)

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

To display the statistic every 1000 episodes:
```bash
./2048 --total=100000 --block=1000 --limit=1000
```

To specify the total games to run, and seed the environment:
```bash
./2048 --total=100000 --evil="seed=12345" # need to inherit from random_agent
```

To save the statistic result to a file:
```bash
./2048 --save=stat.txt
```

To load and review the statistic result from a file:
```bash
./2048 --load=stat.txt
```

## Advanced Usage

To train the network for 100000 games, and save the weights to a file:
```bash
./2048 --total=100000 --block=1000 --limit=1000 --play="save=weights.bin" # need to inherit from weight_agent
```

To load the weights from a file, train the network for 100000 games, and save the weights:
```bash
./2048 --total=100000 --block=1000 --limit=1000 --play="load=weights.bin save=weights.bin" # need to inherit from weight_agent
```

To train the network for 1000 games, with a specific learning rate:
```bash
./2048 --total=1000 --play="alpha=0.0025" # need to inherit from learning_agent
```

To load the weights from a file, test the network for 1000 games, and save the statistic:
```bash
./2048 --total=1000 --play="load=weights.bin alpha=0" --save="stat.txt"
```

## Author

[Computer Games and Intelligence (CGI) Lab](https://cgilab.nctu.edu.tw/), NYCU, Taiwan
