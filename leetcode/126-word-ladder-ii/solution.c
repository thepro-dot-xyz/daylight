#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Object {
    char *begin_word;
    char *end_word;
    char **word_list;
    int len;
};

void print_strs(char **strs, int size)
{
    if (size < 1) {
        printf("[]");
        return;
    }
    int i = 0;
    printf("[\"%s\"", strs[i]);
    for (i = 1; i < size; i++) {
        printf(", \"%s\"", strs[i]);
    }
    printf("]");
}

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (size_t)&(((type *)0)->member)))

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, field) list_entry((ptr)->next, type, field)
#define list_last_entry(ptr, type, field) list_entry((ptr)->prev, type, field)

#define list_for_each(p, head) \
    for (p = (head)->next; p != (head); p = p->next )

#define list_for_each_safe(pos, n, head) \
    for (p = (head)->next, n = p->next; p != (head); p = n, n = p->next)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->first; pos && ({ n = pos->next; true; }); pos = n)

struct hlist_node;

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

void INIT_HLIST_HEAD(struct hlist_head *h)
{
    h->first = NULL;
}

int hlist_empty(struct hlist_head *h)
{
    return !h->first;
}

void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    if (h->first != NULL) {
        h->first->pprev = &n->next;
    }
    n->next = h->first;
    n->pprev = &h->first;
    h->first = n;
}

struct list_head {
    struct list_head *next, *prev;
};

void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list->prev = list;
}

int list_empty(const struct list_head *head)
{
    return (head->next == head);
}

void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_add(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head, head->next);
}

void list_add_tail(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head->prev, head);
}

void __list_del(struct list_head *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

void list_del(struct list_head *entry)
{
    __list_del(entry);
    entry->next = entry->prev = NULL;
}

struct word_node {
    int step;
    char *word;
    struct hlist_node node;
};

struct word_tree {
    char *word;
    struct list_head sibling;
    struct list_head link;
    struct word_tree **parents;
    int par_num;
    int par_cap;
    int step;
};

int BKDRHash(char *str, int size)
{
    int seed = 131;
    unsigned int hash = 0;
    while (*str != '\0') {
        hash = hash * seed + (*str++);
    }
    return hash % size;
}

struct word_node *find(char *word, struct hlist_head *hhead, int size, int step)
{
    struct hlist_node *p;
    int hash = BKDRHash(word, size);
    hlist_for_each(p, &hhead[hash]) {
        struct word_node *node = list_entry(p, struct word_node, node);
        if (!strcmp(node->word, word)) {
            if (node->step == 0 || node->step == step) {
                return node;
            }
        }
    }
    return NULL;
}

void parent_add(struct word_tree *parent, struct word_tree *child)
{
    if (child->par_num + 1 > child->par_cap) {
        child->par_cap *= 2;
        struct word_tree **parents = malloc(child->par_cap * sizeof(void *));
        memcpy(parents, child->parents, child->par_num * sizeof(void *));
        free(child->parents);
        child->parents = parents;
    }
    child->parents[child->par_num++] = parent;
}

char *** find_ladders(char *begin_word, char *end_word, char **word_list, int word_list_size, int **column_sizes, int *return_size)
{
    int i, j;
    int len = strlen(begin_word);
    int hashsize = word_list_size * 2;
    char *word = malloc(len + 1);

    struct hlist_head *hhead = malloc(hashsize * sizeof(*hhead));
    for (i = 0; i < hashsize; i++) {
        INIT_HLIST_HEAD(hhead + i);
    }

    struct list_head *level_heads = malloc(word_list_size * sizeof(*level_heads));
    for (i = 0; i < word_list_size; i++) {
        INIT_LIST_HEAD(&level_heads[i]);
    }

    struct word_node *node;
    for (i = 0; i < word_list_size; i++) {
        node = malloc(sizeof(* node));
        node->word = word_list[i];
        node->step = 0;
        int hash = BKDRHash(word_list[i], hashsize);
        hlist_add_head(&node->node, &hhead[hash]);
    }

    struct list_head *p, queue;
    INIT_LIST_HEAD(&queue);

    struct word_tree *root = malloc(sizeof(*root));
    root->word = begin_word;
    root->step = 1;
    root->par_cap = 1;
    root->par_num = 1;
    root->parents = malloc(sizeof(void *));
    root->parents[0] = NULL;
    list_add_tail(&root->sibling, &level_heads[0]);
    node = find(begin_word, hhead, hashsize, 1);
    if (node != NULL) {
        node->step = 1;
    }

    struct word_tree *first = root;
    while (strcmp(first->word, end_word)) {
        strcpy(word, first->word);
        for (i = 0; i < len; i++) {
            char c;
            char o = word[i];
            for (c = 'a'; c <= 'z'; c++) {
                word[i] = c;
                node = find(word, hhead, hashsize, first->step + 1);
                if (node != NULL) {
                    int enqueue = 1;
                    list_for_each(p, &level_heads[first->step]) {
                        struct word_tree *w = list_entry(p, struct word_tree, sibling);
                        if (!strcmp(w->word, node->word)) {
                            enqueue = 0;
                            parent_add(first, w);
                            break;
                        }
                    }

                    if (enqueue) {
                        node->step = first->step + 1;
                        struct word_tree *new = malloc(sizeof(*new));
                        new->word = node->word;
                        new->step = node->step;
                        new->par_cap = 10;
                        new->par_num = 0;
                        new->parents = malloc(new->par_cap * sizeof(void *));
                        list_add_tail(&new->sibling, &level_heads[first->step]);
                        list_add_tail(&new->link, &queue);
                        parent_add(first, new);
                    }
                }
            }
            word[i] = o;
        }

        if (list_empty(&queue)) {
            *return_size = 0;
            return NULL;
        } else {
            first = list_first_entry(&queue, struct word_tree, link);
            list_del(&first->link);
        }
    }

    i = 0;
    int size = first->step;
    char ***results = malloc(1000 * sizeof(char **));
    int *indexes = malloc(size * sizeof(int));
    memset(indexes, 0, size * sizeof(int));
    struct word_tree **nodes = malloc(size * sizeof(*nodes));
    list_for_each(p, &level_heads[size - 1]) {
        struct word_tree *end = list_entry(p, struct word_tree, sibling);
        if (!strcmp(end->word, end_word)) {
            int move_on = 1;
            while (move_on) {
                move_on = 0;
                struct word_tree *w = end;
                char **list = results[i] = malloc(size * sizeof(char *));
                for (j = size - 1; j >= 0; j--) {
                    list[j] = malloc(len + 1);
                    strcpy(list[j], w->word);
                    nodes[j] = w;
                    w = w->parents[indexes[j]];
                }

                for (j = 0; j < size; j++) {
                    if (indexes[j] < nodes[j]->par_num - 1) {
                        indexes[j]++;
                        memset(indexes, 0, j * sizeof(int));
                        move_on = 1;
                        break;
                    }
                }

                i++;
            }
        }
    }

    *column_sizes = malloc(i * sizeof(int));
    for (j = 0; j < i; j++) {
        (*column_sizes)[j] = size;
    }
    *return_size = i;
    return results;
}

int main(int argc, char **argv)
{
    char *begin_word1 = "hit", *end_word1 = "cog";
    char *word_list1[] = { "hot", "dot", "dog", "lot", "log", "cog" };
    int len1 = sizeof(word_list1) / sizeof(char *);
    struct Object inputs[] = {
        { .begin_word = begin_word1, .end_word = end_word1, .word_list = word_list1, .len = len1 },
    };
    int i, len = sizeof(inputs) / sizeof(struct Object);
    int indent = 2, return_size, *column_sizes;

    for (i = 0; i < len; i++) {
        char *begin_word = inputs[i].begin_word;
        char *end_word = inputs[i].end_word;
        char **word_list = inputs[i].word_list;
        int size = inputs[i].len;

        printf("\n Input:\n");
        printf(" beginWord = \"%s\"\n", begin_word);
        printf(" endWord = \"%s\"\n", end_word);
        printf(" wordList = ");
        print_strs(word_list, size);
        printf("\n");

        char ***lists = find_ladders(begin_word, end_word, word_list, size, &column_sizes, &return_size);

        printf("\n Output:\n");
        
        if (return_size < 1) {
            printf("[]\n");
            continue;
        }
        int j = 0;
        printf("[\n%*s", indent, "");
        print_strs(lists[j], column_sizes[j]);
        for (j = 1; j < return_size; j++) {
            printf(",\n%*s", indent, "");
            print_strs(lists[j], column_sizes[j]);
        }
        printf("\n]\n");
    }

    return EXIT_SUCCESS;
}
