if [ $1 == "java" ]
then
    sudo pypy3 main.py -i $2 -o out/out.txt
    sudo haxe -main Main -java out/java
    time sudo java -jar out/java/Main.jar out/out.txt
fi
if [ $1 == "node" ]
then
    sudo pypy3 main.py -i $2 -o out/out.txt
    sudo haxe -lib hxnodejs -main Main -js out/out.js
    time node out/out.js out/out.txt
fi
if [ $1 == "cpp" ]
then
    sudo pypy3 main.py -i $2 -o out/out.txt
    sudo haxe -main Main -cpp out/cpp
    time ./out/cpp/Main out/out.txt
fi
if [ $1 == "build-all" ]
then
    sudo haxe -main Main -java out/java
    sudo haxe -main Main -lua out/lua.lua
    sudo haxe -main Main -cpp out/cpp
    sudo haxe -main Main -cs out/cs
    sudo haxe -main Main -python out/out.py
    sudo haxe -lib hxnodejs -main Main -js out/out.js
fi
if [ $1 == "run-all" ]
then
    pypy3 main.py -i $2 -o out/out.txt
    echo
    echo "NodeJS"
    time node out/out.js out/out.txt
    echo
    echo "Java"
    time java -jar out/java/Main.jar out/out.txt
    echo
    echo "C++"
    time ./out/cpp/Main out/out.txt
    echo
    echo "C#"
    time ./out/cpp/Main out/out.txt
    echo
    echo "PyPy3"
    time pypy3 out/out.py out/out.txt
    echo
    echo "LuaJIT"
    time luajit out/lua.lua out/out.txt
    echo
    echo "Lua5.3"
    time lua5.3 out/lua.lua out/out.txt
    echo
    echo "Python3"
    time python3 out/out.py out/out.txt
    echo
    echo "Lua5.1"
    time lua5.1 out/lua.lua out/out.txt
    echo
    echo "PyPy3 --jit off"
    time pypy3 --jit off out/out.py out/out.txt
fi