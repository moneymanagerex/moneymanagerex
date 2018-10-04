# Run Docker Configuration for Money Manager Ex

This is a dockerized version of [Money Manager Ex](https://github.com/moneymanagerex/moneymanagerex). It runs unprivileged inside of the container and is able to persist data using a shared folder. It docker environment is pre-configured by providing a `Makefile` template that's ready to use.

## Running

Call `make build && make run` in order to build the image and run a container from it.