# Terminus

Terminus is a minimal bash-style command interpreter implemented in C

```TEXT
████████╗███████╗██████╗ ███╗   ███╗██╗███╗   ██╗██╗   ██╗███████╗
╚══██╔══╝██╔════╝██╔══██╗████╗ ████║██║████╗  ██║██║   ██║██╔════╝
   ██║   █████╗  ██████╔╝██╔████╔██║██║██╔██╗ ██║██║   ██║███████╗
   ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║██║██║╚██╗██║██║   ██║╚════██║
   ██║   ███████╗██║  ██║██║ ╚═╝ ██║██║██║ ╚████║╚██████╔╝███████║
   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝
```

## Requirements

- Linux environment
- gcc 11.4 (or higher)
- GNU Make 4.3 (or higher)

## Installation

Clone this repository

```BASH
git clone https://github.com/Christ0u/Terminus.git
```

## Build

Enter in the directory `/Terminus`

```BASH
cd ./Terminus
```

Compile the program

```BASH
make
```

## Configuration

To be able to read the Terminus man page, you will need to copy the man file to a specific location on your system

```BASH
sudo cp ./doc/man_Terminus.1 /usr/local/share/man/man1/Terminus.1
```

## Usage

Launch the interactive shell

```bash
./bin/Terminus
```

Now, you have a minimal command interpreter

```TEXT
[ Terminus ] : /home/<user>/Documents/Sources/Terminus >
```

To find help about options and features, you can :

- use the Terminus man page

```BASH
man Terminus
```

- use the embedded help

```BASH
./bin/Terminus --help
```

## Authors

- Océanne DRUENNE ([odruenne](https://github.com/odruenne))
- Christopher GERARD ([Christ0u](https://github.com/Christ0u))

## License

[MIT](./LICENSE)
