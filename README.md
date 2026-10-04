# Tabby GPU

This project aims at building a 2D GPU from scratch using TLM modeling and RTL implementation.

## Setup

### MacOS

```bash
brew install systemc cmake ninja
```

## Run

### TLM-Model

```bash
cd model/systemc
mkdir build
cd build
cmake .. -GNinja
./tabby_tlm_model
```

## Contribute

### Pre-commit hooks

Install and enable the formatting hook:

```bash
python3 -m pip install pre-commit
pre-commit install
```

## TODOs

- [x] Pre-commit hooks
