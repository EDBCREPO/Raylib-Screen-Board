export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH

#WINDOWS
#time g++ -o main main.cpp -I ./include -L ./lib -lraylib -lssl -lcrypto

#LINUX
time g++ -o main main.cpp -I./include -L./lib -lraylib -lssl -lcrypto

if [ ! $? -eq 0 ]; then
    echo "exit error"; exit;
fi

./main