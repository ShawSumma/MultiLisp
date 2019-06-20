sudo pypy3 main.py -i $1 -o out/out.pack
sudo haxe -main Main -java out/java
sudo haxe -main Main -lua out/lua.lua
sudo haxe -main Main -cpp out/cpp
sudo haxe -main Main -cs out/cs
sudo haxe -main Main -python out/out.py
sudo haxe -lib hxnodejs -main Main -js out/out.js
echo
echo
echo "Java"
time java -jar out/java/Main.jar out/out.pack
echo
echo
echo "NodeJS"
time nodejs out/out.js out/out.pack
echo
echo
echo "PyPy3"
time pypy3 out/out.py out/out.pack
echo
echo
echo "C++"
time ./out/cpp/Main out/out.pack
echo
echo
echo "C#"
time ./out/cpp/Main out/out.pack
echo
echo
echo "LuaJIT"
time luajit out/lua.lua out/out.pack
# echo
# echo
# echo "Lua5.3"
# time lua5.3 out/lua.lua out/out.pack
# echo
# echo
# echo "python3"
# time python3 out/out.py out/out.pack