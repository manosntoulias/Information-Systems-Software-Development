all:
	gcc -g3 -o pam main.c pam_graph.c pam_utilities.c data_storage_utilities.c MAN_2.c -lm
