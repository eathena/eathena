
all txt sql clean confclean:
	@echo "running configure first"
	@echo 
	@chmod +x configure
	./configure --enable-txt
	@echo 
	@echo "this was an initial configure run done with '--enable-txt' option only"
	@echo "if other options are required run configure again with your choise"
	@echo "to get a list of possible options, run './configure --help'"
