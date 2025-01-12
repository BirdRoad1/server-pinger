# MCPinger

Easy-to-use tool written in C++ that can scan the entire internet (or IP ranges of your choosing) for Minecraft servers. This tool leverages the Minecraft protocol's `Server List Ping` system documented [here](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Server_List_Ping).

This tool only supports Linux-based operating systems. Windows and macOS will not be considered.

Currently, this tool requires a PostgreSQL database to store data in.

To use this program, follow the steps in the `Building` section. Then, create a "cidrs.txt" file and enter a list of IP ranges in CIDR notation. Next, create a "db.txt" file with the url of the PostgreSQL database (an example is available in db.txt.example). Finally, run the binary.

# Masscan

This project currently isn't as fast as I'd like it to be. However, you can use [Masscan](https://github.com/robertdavidgraham/masscan) to initially scan for hosts with their port 25565 open to reduce the number of IPs to scan. Here's an example command:

```bash
masscan 0.0.0.0/0 -p25565 --excludefile exclude.conf -oX output.xml --rate 1000000
```

Next, you can run:

```bash
python3 scripts/convertMasscanOutput.py
```

to convert output.xml to a plaintext list of IP addresses (which will be stored in output.txt).

Finally, you can run this program to look for online Minecraft servers in the list.

# Building

Install the required dependencies (Ubuntu):

```bash
sudo apt-get install libpqxx-dev libpq-dev g++ make cmake
```

Generate the Makefile using CMake:

```bash
cmake .
```

Compile the tool:

```bash
make
```

The binary will be at `./bin/mcpinger`

# Contributions

All contributions are welcome. To contribute, you can create an Issue or Pull Request. Feature requests are also welcome.
