if [ $1 == "java" ]
then
    sudo pypy3 main.py -i $2 -o out/out.pack
    sudo haxe -main Main -java out/java
    time sudo java -jar out/java/Main.jar out/out.pack
fi
if [ $1 == "node" ]
then
    sudo pypy3 main.py -i $2 -o out/out.pack
    sudo haxe -lib hxnodejs -main Main -js out/out.js
    time nodejs out/out.js out/out.pack
fi
if [ $1 == "run" ]
then
    sudo pypy3 main.py -i $2 -o out/out.pack
    sudo haxe --run Main out/out.pack
fi
if [ $1 == "build-all" ]
then
    sudo pypy3 main.py -i $2 -o out/out.pack
    sudo haxe -main Main -java out/java
    sudo haxe -main Main -lua out/lua.lua
    sudo haxe -main Main -cpp out/cpp
    sudo haxe -main Main -cs out/cs
    sudo haxe -main Main -python out/out.py
    sudo haxe -lib hxnodejs -main Main -js out/out.js
fi
if [ $1 == "run-all" ]
then
    echo
    echo "Java"
    java -jar out/java/Main.jar out/out.pack
    echo
    echo "NodeJS"
    nodejs out/out.js out/out.pack
    echo
    echo "PyPy3"
    pypy3 out/out.py out/out.pack
    echo
    echo "C++"
    ./out/cpp/Main out/out.pack
    echo
    echo "C#"
    ./out/cpp/Main out/out.pack
    echo
    echo "LuaJIT"
    luajit out/lua.lua out/out.pack
    echo
fi