#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX 1000

typedef struct
{
  char name[MAX];
} task_t;

typedef struct
{
  size_t task_len;
  task_t *tasks;
} task_list_t;

int init_task_list(task_list_t *task_list)
{
  task_list->task_len = 0;
  task_list->tasks = malloc(sizeof(task_t) * MAX);

  if (task_list->tasks == NULL) return 0;

  return 1;
}

int add_task(task_list_t *task_list, const char *name)
{
  task_t *task;
  
  if (task_list->task_len > MAX) {
    return 0;
  }

  memset(task_list->tasks[task_list->task_len].name, 0, MAX);
  strncpy(task_list->tasks[task_list->task_len].name, name, MAX - 1);
  task_list->task_len++;

  return 1;
}

void read_from_db(const char *dbpath, task_list_t *task_list)
{
  FILE *dbr = fopen(dbpath, "rb");
  size_t reads = 0;

  if (dbr == NULL) {
    fprintf(stderr, "Could not read from db file!\n");
    exit(EXIT_FAILURE);
  }

  if (feof(dbr)) {
    return;
  }

  fscanf(dbr, "%ld", &reads);

  task_list->task_len = reads;
  fread(task_list->tasks,
	sizeof(task_t),
	reads,
	dbr);
}

void write_to_db(const char *dbpath, task_list_t *task_list)
{
  FILE *dbw = fopen(dbpath, "wb");
  size_t writes;
  
  if (dbw == NULL) {
    fprintf(stderr, "Could not write to db file!\n");
    exit(EXIT_FAILURE);
  }

  fprintf(dbw, "%ld", task_list->task_len);

  writes = fwrite(task_list->tasks,
		  sizeof(task_t),
		  task_list->task_len,
		  dbw);

  if (writes != task_list->task_len) {
    fprintf(stderr, "Number of writes do not match number of tasks!\n");
  }

  fclose(dbw);
}

void debug_print_task_list(task_list_t *task_list)
{
  int i;

  for (i = 0; i < task_list->task_len; i++) {
    printf(". %s\n", task_list->tasks[i].name);
  }
}

void free_task_list(task_list_t *task_list)
{
  free(task_list->tasks);
}

const char *get_home_dir(void)
{
  const char *home = getenv("HOME");

  if (home) return home;

  uid_t uid = getuid();
  struct passwd *pw = getpwuid(uid);

  if (pw == NULL) {
    fprintf(stderr, "Could not get home directory, abort\n");
    exit(EXIT_FAILURE);
  }

  return pw->pw_dir;
}

enum COMMAND {
  ADD = 1,
  LIST,
  DELETE,
  COMPLETE
};

int parse_command(int argc, char **argv, char *param, size_t max)
{
  const char *head;
  
  if (argc <= 1) return 0;

  memset(param, 0, max);
  head = argv[1];
  
  if (!strcmp(head, "add") ||
      !strcmp(head, "delete") ||
      !strcmp(head, "complete")) {
    int i;
    size_t m = max - 1;
    
    for (i = 2; i < argc; i++) {
      m = m - strlen(param);
      strncat(param, argv[i], m);
      m = m - 1;
      strcat(param, " ");
    }
    
    if (!strcmp(head, "add")) return ADD;
    if (!strcmp(head, "delete")) return DELETE;
    if (!strcmp(head, "complete")) return COMPLETE;
  }
  else if (!strcmp(head, "list") || !strcmp(head, "LIST")) {
    return LIST;
  }
  else {
    return 0;
  }
}

int main(int argc, char **argv)
{
  const char *home = get_home_dir();
  const char *dbname = "tasks.bin";
  char dbpath[MAX];
  char command[MAX];
  task_list_t task_list;
  int op;
  
  memset(dbpath, 0, MAX);
  strncat(dbpath, home, (MAX - strlen(dbname) - 2));
  strcat(dbpath, "/");
  strcat(dbpath, dbname);
  sprintf(command, "touch %s", dbpath);
  system(command);

  init_task_list(&task_list);
  read_from_db(dbpath, &task_list);

  op = parse_command(argc, argv, command, MAX);

  if (op) {
    switch (op) {
    case ADD:
      add_task(&task_list, command);
      break;
    case LIST:
      debug_print_task_list(&task_list);
      break;
    case DELETE:
      if (strlen(command) == 0) {
	int ch;
	
	printf("Delete all tasks? (y/n) ");
	ch = getchar();

	if (ch == 'y') {
	  free_task_list(&task_list);
	  init_task_list(&task_list);
	}
      }
      else {
	puts("no implementation yet");
      }
      break;
    }
  }
  
  write_to_db(dbpath, &task_list);
  free_task_list(&task_list);
  return EXIT_SUCCESS;
}
