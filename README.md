Datatypetests
=============
This repo contains tests that test the Flexible Datatype of Couchbase Server 3.0. You need a libcouchbase library capable of making a hello call to the server, such as https://github.com/uvenum/libcouchbase (which was forked from the original libcouchbase library). Datatypetests also contains dataclient.c source file that is a modification of the minimal.c file from the original libcouchbase library. 

Requirments: needs Google Test and Google snappy libraries to run.

How to compile the test code:

g++ -o Datatypetest -I{Google Test include folder path} -L{path to Google Test compiled library} -L{path to libcouchbase compiled library} -L{path to libcouchbase compiled library} -lcouchbase -lsnappy -lgtest DataTypeTests.cc

Usage:

./Datatypetest --gtest_output="xml" localhost:9000 bucket_2 password
