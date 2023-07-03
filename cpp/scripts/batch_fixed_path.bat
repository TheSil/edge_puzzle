xcopy ..\build\backtracker_fixed_path\bin\BacktrackerFixedPath.exe workdir_backtracker /i /y
PUSHD workdir_backtracker
start BacktrackerFixedPath.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv 
start BacktrackerFixedPath.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv 
start BacktrackerFixedPath.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv 
PUSHD ..\..\..\
start python monitor.py -conf data\eternity2\eternity2_256.csv -dir cpp\scripts\workdir_backtracker
POPD
POPD