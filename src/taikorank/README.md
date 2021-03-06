# Taiko Ranking project
This is an amateur star rating for taiko mode in osu!

## How it works
The star rating is based on my personnal perception of what is hard in osu!taiko. Thus it may not correspond to everyone point of vue, but everyone's opinion is welcomed as the rating still need some improvement. Basicely, the taiko ranking project splits the computation in four skills: 
* *Density* how hard is an object to hit. The quicker you have to hit, the harder.
* *Reading* how hard is an object to read. Like playing with hidden, flashlight or high slider velocity. 
* *Pattern* how hard is a pattern to play. For example ddddddd is easier than ddkdkkd. 
* *Accuracy* how hard it is to play accuratly. Focusing on overall difficulty and odd spacing between objects.

A final value is then computed giving global star rating for the map.

## Requirements
##### Needed
* osux and mostly everything that comes with it
* GNU Triangulated Surface

##### Optional
* openmp, use `-fopenmp` in CFLAGS to enable
* mysqlclient, use `-lmysqlclient` in LDFLAGS and `-DUSE_TR_MYSQL_DB` to enable

## Usage
`taiko_ranking [GLOBAL_OPTION] ... [LOCAL_OPTIONS] [FILE|HASH] ... [LOCAL_OPTION] ... [FILE|HASH] ... `

#### Configuration
Configuration files are in the yaml/ directory.
* config.yaml, is the general configuration.
* density_cst.yaml, contains variables related to density.
* reading_cst.yaml, same as before.
* pattern_cst.yaml, same as before.
* accuracy_cst.yaml, same as before.
* final_cst.yaml, contains varaibles for computing the global star rating.

#### Options
Change the config in config.yaml for an execution. There are two types of options: global options, applying to all files and local options applying to all files after them.

##### Global options
Global options are prefixed with `+`

###### Autoconvert
* `+autoconvert [0|1]` allow autoconvert. The autoconvert algorithm is not perfect and can be wrong with some beatmaps. Checking the maximum combo might give you a big hint.

###### Database
* `+db [0|1]` store results in the database

###### Osux database 
* `+odb [0|1]` enable or disable osux database
* `+odb_path [PATH]` path to osuxdb

###### Print
* `+ptro [0|1]` print all objects
* `+pyaml [0|1]` print result in yaml
* `+pfilter [bB+drRpa*]` print specific information. (b = basic, B = basic+, + = additionnal, d = density, r = reading, R = reading+, p = pattern, a = accuracy, * = star)
* `+porder [FDRPA]` choose order (F = final, D = density, R = reading, P = pattern, A = accuracy)

##### Local options
Local options are prefixed with `-`

###### Score
* `-score [0|1]` compute a score
* `-quick [0|1]` do not recompute all objects after every modification
* `-input [0|1]` change input to accuracy (0) or great/good/miss (1)
* `-step [DOUBLE]` set a step to print score every time the step is reached. Use a negative value to only print the last score. Use 0 to print every step.
* `-ggm [GOOD] [MISS]` set number of good and number of miss for a score
* `-acc [0-100]` set accuracy for a score

###### Modifier
* `-mods [HD|HR|DT|...]` change mods. Don't use space between mods. Use __ for no mod
* `-flat [0|1]` change D to d, etc
* `-no_bonus [0|1]` remove bonus notes

