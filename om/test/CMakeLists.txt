function(omr_add_om_test testname)
	add_executable("${testname}"
		"${testname}.cpp"
	)
	target_link_libraries("${testname}"
		PUBLIC
			omr_om
			gtest_main
	)
	add_test(
		NAME    "${testname}"
		COMMAND "${testname}"
	)
endfunction(omr_add_om_test)

omr_add_om_test(MemoryManagerTest)
omr_add_om_test(RuntimeTest)
omr_add_om_test(ValueTest)
omr_add_om_test(RootTest)
omr_add_om_test(DoubleTest)
