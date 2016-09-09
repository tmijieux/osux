# Taiko Ranking Test
A few tests are available for the taiko ranking and other taiko stuff. The tests are the following python files:
* tr_test_convert.py
* tr_test_stars
* tr_test_mapset

There is also linear_fun.py which is a small tool to check how looks the piecewise linear functions used in the taiko ranking.

### Test convert
For checking if the autoconvert algorithm is correct. Use the following command line to run the test:

`./tr_test_convert.py yaml/convert/test_convert.yaml`

They are basicely unit test only on sliders. Even if it looks quite good at the moment, osux convertion algorithm differs from the real one on some maps.

### Test stars
For checking star rating based on a given output order. If the beatmaps are not in the same order the number of errors is computed using Levenshtein's distance. First you will have to generate the beatmaps with:

`./generate_beatmaps.py`

Then you can run all the test like this:

`./tr_test_convert.py yaml/test_generated_main.yaml`

### Test mapset
For checking star rating in a mapset without a given output order. The program guess the expecting output based on the difficulty names: Inner Oni > Oni > Muzukashi...

`./tr_test_mapset.py "path/to/dir/"`

The program is recursive and will search for directories inside the argument. It is useful as it creates a lot of tests but I think they are easy tests. Well, it's still better than nothing.