xcopy ..\build\swapper\bin\Swapper.exe workdir_swapper /i /y
PUSHD workdir_swapper
start Swapper.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv
start Swapper.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv
PUSHD ..\..\..\
start python monitor.py -conf data\eternity2\eternity2_256.csv -dir cpp\scripts\workdir_swapper
POPD
POPD