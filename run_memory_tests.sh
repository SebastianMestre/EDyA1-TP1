
for TEST_FILE in tests/memory_test*
do
	valgrind ./interprete < $TEST_FILE
	echo ""
done
