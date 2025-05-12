server_and_client_terminal_emulator: client.c server.c
	gcc -o server tcp_server.c
	gcc -o client tcp_client.c
