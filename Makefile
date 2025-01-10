build:
	g++ -Wall src/main.cpp src/stats.cpp src/database.cpp src/serverdata.cpp src/data/packetbuffer.cpp src/mcping.cpp src/scanner.cpp src/cidr.cpp src/exception/net_exception.cpp src/exception/ping_parse_exception.cpp -lpqxx -Iinclude/ -o bin/out