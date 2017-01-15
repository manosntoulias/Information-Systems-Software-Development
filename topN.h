typedef struct forum_size {
	int size;
	int id;
} forum_size;

typedef struct forum {
	int size;
	char name[70];
} pam_forum;

// reads file with name forums an returns the largest N forums
// NULL on failure
pam_forum *topN(int N, pam_graph** main, pam_graph*** top_graphs);
