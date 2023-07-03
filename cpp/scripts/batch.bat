xcopy ..\build\backtracker\bin\Backtracker.exe workdir_backtracker /i /y
PUSHD workdir_backtracker
start Backtracker.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv  
start Backtracker.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv 
start Backtracker.exe d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256.csv d:\Git\TheSil\edge_puzzle\data\eternity2\eternity2_256_all_hints.csv 
PUSHD ..\..\..\
start python monitor.py -conf data\eternity2\eternity2_256.csv -dir cpp\scripts\workdir_backtracker
POPD
POPD