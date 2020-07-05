PATH=%PATH%;C:\work\simlabs-repo\bin\debug;C:\work\webstream\ws_server\build_x64\Debug
cd ..

start q2rtx.exe +set game server_game +connect 127.0.0.1
start q2rtx.exe +set game client_game +map base1 +streaming_client
