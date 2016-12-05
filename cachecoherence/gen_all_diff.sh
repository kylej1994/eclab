rm expected_*
rm our_*
rm diff_*
./test.sh goldensim5b mp_test1.x >> expected_test1
./test.sh goldensim5b mp_test2.x >> expected_test2
./test.sh goldensim5b mp_fibonacci.x >> expected_fibonacci

./test.sh sim mp_test1.x >> our_test1
./test.sh sim mp_test2.x >> our_test2
./test.sh sim mp_fibonacci.x >> our_fibonacci

diff expected_test1 our_test1 >> diff_test1
diff expected_test2 our_test2 >> diff_test2
diff expected_fibonacci our_fibonacci >> diff_fibonacci
