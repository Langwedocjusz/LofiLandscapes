# LofiLandscapes

### Cloning
This repository contains submodules and should be cloned recursively:

	git clone --recursive https://github.com/Langwedocjusz/LofiLandscapes <TargetDir>
  
### Building and running
Regardless of the platform make sure cmake is installed and added to your path.

#### On Linux:
For building you can use the provided script `BuildProjects.sh`. It will ask you to choose either 'Debug' of 'Release' configuration.
Afterwards you can run the program from the top level of the repository:

	./build/bin/LofiLandscapes
  
#### On Windows:
The provided batchfile `WIN_GenerateProjects.bat` will generate a Visual Studio solution.
After running it you can open `build/LofiLandscapes.sln` to build and run the program (start project and working directory should already be set).
