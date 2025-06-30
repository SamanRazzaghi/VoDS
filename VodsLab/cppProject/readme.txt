it is pre built but if you want to build it on your own machine do these:
    on linux machines:
        make sure cmake make and mingw are installed.
        make sure buildall.sh is an executable by typing chmod +x buildall.sh in the terminal.
        then in the terminal run ./buildall.sh and it will build it both for linux and windows(under mingw)
    on Windows machines:
        make sure msvc and cmake are installed.
        then run these commands in the cmd(terminal in the newer windows) to build for windows under msvc
            mkdir build-msvc
            cd build-msvc
            cmake ..
            cmake --build .
        you can find the executable in the Debug folder
