# Cpuloadgen

Cpuloadgen is a simple tool for CPU load generation on Linux written from scratch in C. It allows you to generate desired CPU utlisation by percentage for given time period on desired cores.

# Build

```
gcc cpuloadgen -o cpuloadgen -lm
```

## Usage

Generate 80% load on all cores for 60s:

```
./cpuloadgen -t 60 -p 80
```

Generate 80% load on core 0, 2 and 4 for60s

```
./cpuloadgen -t 60 -p 80 -c 0 -c 2 -c 4
```

