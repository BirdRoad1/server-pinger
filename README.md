# MCPinger

Easy-to-use tool written in C++ that can scan the entire internet (or IP ranges of your choosing) for Minecraft servers. This tool leverages the Minecraft protocol's `Server List Ping` system documented [here](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Server_List_Ping).

This tool only supports Linux-based operating systems. Windows and macOS will not be considered.

To use this program, follow the steps in the `Building` section. Then, create a "cidrs.txt" file and enter a list of IP ranges in CIDR notation. Next, create a "db.txt" file with the url of the PostgreSQL database (an example is available in db.txt.example). Finally, run the binary.

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

All contributions are welcome. To contribute, you can create an Issue or Pull Request. Feature requests are also welcome.
