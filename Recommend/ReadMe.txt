Clustering -- Project 3 : Recommendation

by	Galatas Epameinondas		( 1115201300026 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                                                                              ///
///                                                          Compilation and execution                                                           ///
///                                                                                                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Compile: (static lib first!!)
$ cd Shared
$ make
$ cd ../Recommend
$ make

IMPORTANT NOTE: C++14 requires gcc version 4.9 and higher. Tested only with version 6.3.0 (Ubuntu 17.04).

Execute:
$ ./run -d <data file>

Optional arguments:
-o <output file>
-T <integer> (#recommended items, default: 10)
--hamming | --cos (changed used metric, default is L2)
--lsh (use lsh instead of Clustering)
-N <integer> (#lsh NNs, default: 20)
--eval (run in k-fold evaluation mode)
-F <integer> (#folds, default: 10)
