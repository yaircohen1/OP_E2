./ttt "123456789" < AI_win.txt
./ttt "123456789" < draw.txt || true
./ttt "123456789" < empty.txt || true
./ttt "123456789" < invalid.txt || true
./ttt "123456789" < invalid2.txt || true
./ttt "123456789" < taken_cell.txt || true
./ttt "123456789" < user_win_col.txt || true
./ttt "123456789" < user_win_row.txt || true
./ttt "123456789" < user_win_diag.txt || true
./ttt >/dev/null || true 
./ttt "12348" >/dev/null || true 
./ttt "12344667588" >/dev/null || true 
./ttt "a" >/dev/null || true 

# tcp
echo "Running TCP test..."
# tcp server
echo "Running TCP Server..."
timeout 5 ./mync -e "ttt 123456789" -i TCPS1234 &
sleep 2
echo -e "ok\n3\n6\n9\n" | nc localhost 1234 || true

# tcp client
echo "Running TCP client connection test..."
nc -l -p1234 &
sleep 2
echo -e "ok\n3\n6\n9\n" | timeout 5 ./mync -e "ttt 123456789" -o TCPClocalhost,1234 || true

# tcp client & server
echo "Running TCP client & server connection test..."
nc -l -p4455 &
sleep 2
timeout 5 ./mync -e "ttt 123456789" -i TCPS1234 -o TCPClocalhost,4455 &
sleep 2
echo -e "ok\n3\n6\n9\n" | nc localhost 1234 || true

# tcp -b
echo "Running TCP -b flag..."
timeout 5 ./mync -e "ttt 123456789" -b TCPS1234 &
sleep 2
echo -e "ok\n3\n6\n9\n" | nc localhost 1234 || true

# UDP
echo "Running UDP test..."
# UDP server
echo "Running UDP Server with t flag..."
./mync -e "ttt 123456789" -i UDPS4080 -t 10
sleep 2
echo -e "ok\n3\n6\n9\n" | nc -u localhost 4080 || true


# UDP client
echo "Running UDP client connection test..."
nc -u -l -p1234 &
sleep 2
echo -e "ok\n3\n6\n9\n" | timeout 5 ./mync -e "ttt 123456789" -o UDPClocalhost,1234 || true

#TCP without -e flag
echo "Running TCP server without -e flag..."
timeout 5 ./mync -i TCPS4070 &
server_pid=$!
sleep 2
echo "Quit\n" | nc localhost 4070 || true
# Wait for the server process to complete
wait $server_pid

echo "TCP server without -e flag test completed."

# Unix domain socket stream server
echo "Running Unix domain socket stream server..."
timeout 5 ./mync -e "ttt 123456789" -i UDSSS/home/yairco/OS/EX2_/my_socket &
server_pid=$!
sleep 2
echo "3\n6\n9\n" | nc -U /home/yairco/OS/EX2_/my_socket || true
wait $server_pid
echo "Unix domain socket stream server test completed."

# Unix domain socket stream client
echo "Running Unix domain socket stream client..."
nc -lU /home/yairco/OS/EX2_/my_socket &
sleep 2
echo "3\n6\n9\n" | timeout 5 ./mync -e "ttt 123456789" -o UDSCS/home/yairco/OS/EX2_/my_socket || true
echo "Unix domain socket stream client test completed."

# Unix domain socket datagram server
echo "Running Unix domain socket datagram server..."
timeout 5 ./mync -e "ttt 123456789" -i UDSSD/home/yairco/OS/EX2_/my_socket &
server_pid=$!
sleep 2
echo "Sending data to the server..."
echo -e "3\n6\n9\n" | socat - UNIX-SENDTO:/home/yairco/OS/EX2_/my_socket || true
wait $server_pid
echo "Unix domain socket datagram server test completed."

echo "Tests completed."
