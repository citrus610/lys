# Lys
An AI designed to play Puyo Puyo Tsu 1P and PVP.

## Build
- Clone and `cd` to the repository.
- Run `make` to build the client.
- Run `make PEXT=true` for the best performance, check if your cpu supports `pext`.
- Get the binary in `bin`.

## Directory description
- `core` - puyo puyo core logic implementation.
- `ai` - the AI implementation.
- `cli` - the standalone client, for now only showing the AI playing 1P.
- `test` - tool for testing the AI's chaining capability.
- `tuner` - tuner for the AI using generic algorithm.

## License
This project is licensed under [MIT LICENSE](LICENSE).
