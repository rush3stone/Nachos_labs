echo "=== copies file \"small\" from UNIX to Nachos (and add extension) ==="
./nachos -cp test/small small.txt
sleep 1 # to obse [Echo '=== copies file \\'small\\' from UNIX to Nachos (and add extension) ==='] rve the modification time change [Rve the modification time change]
echo "=== print the content of file \"small\" ===" [Echo '=== print the content of file \\'small\\' ===']
./nachos -p small.txt [./nachos -Q -p small.txt]
echo "=== prints the contents of the entire file system ===" [Echo '=== prints the contents of the entire file system ===']
./nachos -D [./nachos -Q -D]
