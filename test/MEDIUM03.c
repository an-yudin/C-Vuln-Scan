#include <stdlib.h>

struct char2 {
  char *a;
  char *b;
};

int bad(struct char2 *in) {
  *(in->a) = 'a'; // Use after free
  return 0;
}

int good2bad(struct char2 *in, int (*fun_ptr)(struct char2 *)) {
  struct char2 *local = in;
  (*fun_ptr)(local);
  return 0;
}

int main() {
  struct char2 *c2_alias;
  struct char2 *c2 = (struct char2 *)malloc(sizeof(struct char2));
  c2->a = (char *)malloc(sizeof(char)); // Memory allocation
  c2_alias = c2;
  free(c2->a); // Free allocated memory for 'c2->a'

  int (*fun_ptr)(struct char2 *) = &bad;

  good2bad(c2_alias, fun_ptr);
  free(c2);
  return 0;
}
