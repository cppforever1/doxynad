sudo apt update
sudo apt install libspdlog-dev libfmt-dev


cmake -S /home/nad/doxynad -B /home/nad/doxynad/out/build
cmake --build /home/nad/doxynad/out/build --target cmake_graphviz

