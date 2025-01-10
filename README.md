# CPinger

Easy-to-use tool written in C++ that can scan the entire internet (or IP ranges of your choosing) for Minecraft servers. This tool leverages the Minecraft protocol's `Server List Ping` system documented [here](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Server_List_Ping).

# Building

Install the required dependencies:
```bash
sudo apt-get install libpqxx-dev libpq-dev
```

Compile the tool using Make:
```bash
make
```

The binary will be at `./bin/out`

# Contributions
All contributions are welcome