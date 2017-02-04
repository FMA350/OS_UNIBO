#include <stdio.h>
#include "mikabooq.h"
#include "const.h"
#include "listx.h"



void test_init(void);

void test_proc_firstchild(void);

void test_proc_firstthread(void);

void test_proc_alloc(void);

void test_proc_delete(void);



int main(int argc, char const *argv[]) {

    test_proc_delete();

    return 0;
}

void free_memory(struct pcb_t *root_proc) {
    struct list_head *curr;
    int counter = 0;

    list_for_each(curr, &(root_proc->p_siblings))
        counter++;

    printf("number of free process memory slots == %d\n", counter);
}

void test_init(void){
    struct pcb_t *root_proc = proc_init();
    printf("root_proc->p_parent == %d\n", (long) root_proc->p_parent);
    printf("MAXPROC == %d\n", MAXPROC);
    free_memory(root_proc);
}

void test_proc_firstchild(void){
    struct pcb_t *root_proc = proc_init();
    struct pcb_t *firstchild = proc_firstchild(root_proc);
    printf("firstchild --> %p\n", firstchild);
    firstchild = proc_alloc(root_proc);
    printf("firstchild --> %p\n", firstchild);
}

void test_proc_firstthread(void){
    struct pcb_t *root_proc = proc_init();
    printf("first thread -> %p\n", proc_firstthread(root_proc));
}
/* to use this test it is necessary to add a field array
   of char called name to struct pcb_t */
/*
void test_proc_alloc(void) {
    struct pcb_t *root_proc = proc_init();
    strcpy(root_proc->name, "root");
    printf("MAXPROC == %d\n", MAXPROC);

    struct pcb_t *first_child_root = proc_alloc(root_proc);
    strcpy(first_child_root->name, "first_child_root");
    struct pcb_t *second_child_root = proc_alloc(root_proc);
    strcpy(second_child_root->name, "second_child_root");
    struct pcb_t *first_child_first = proc_alloc(first_child_root);
    strcpy(first_child_first->name, "first_child_first");
    struct pcb_t *second_child_first = proc_alloc(first_child_root);
    strcpy(second_child_first->name, "second_child_first");
    struct pcb_t *first_child_second = proc_alloc(second_child_root);
    strcpy(first_child_second->name, "first_child_second");
    struct pcb_t *second_child_second = proc_alloc(second_child_root);
    strcpy(second_child_second->name, "second_child_second");

    struct list_head *curr;

    printf("----------------------ELDEST CHILDREN------------------------\n");

    printf("Process name --> %s\n", root_proc->name);
    list_for_each(curr, &(root_proc->p_children)) {
        printf("Process name --> %s\n", container_of(curr, struct pcb_t, p_children)->name);
    }

    printf("----------------------ROOT'S CHILDREN------------------------\n");

    struct pcb_t *first_child = container_of(list_next(&(root_proc->p_children)), struct pcb_t, p_children);
    printf("Process name --> %s\n", first_child->name);
    list_for_each(curr, &(first_child->p_siblings)) {
        printf("Process name --> %s\n", container_of(curr, struct pcb_t, p_siblings)->name);
    }

    printf("----------------------FIRST CHILD ROOT'S CHILDREN------------------------\n");

    first_child = container_of(list_next(&(first_child_root->p_children)), struct pcb_t, p_children);
    printf("Process name --> %s\n", first_child->name);
    list_for_each(curr, &(first_child->p_siblings)) {
        printf("Process name --> %s\n", container_of(curr, struct pcb_t, p_siblings)->name);
    }

    printf("----------------------SECOND CHILD ROOT'S CHILDREN------------------------\n");

    first_child = container_of(list_next(&(second_child_root->p_children)), struct pcb_t, p_children);
    printf("Process name --> %s\n", first_child->name);
    list_for_each(curr, &(first_child->p_siblings)) {
        printf("Process name --> %s\n", container_of(curr, struct pcb_t, p_siblings)->name);
    }
    printf("----------------------FREE LIST------------------------\n");

    free_memory(root_proc);
}
*/

void test_proc_delete(void){
    struct pcb_t *root_proc = proc_init();
    printf("creating root process\n");
    //printf("root delete --> %d\n", proc_delete(root_proc));
    free_memory(root_proc);
    printf("creating first process\n");
    struct pcb_t *first = proc_alloc(root_proc);
    free_memory(root_proc);
    printf("creating second process\n");
    struct pcb_t *second = proc_alloc(root_proc);
    free_memory(root_proc);
    printf("first delete --> %d\n", proc_delete(first));
    free_memory(root_proc);
    printf("second delete--> %d\n", proc_delete(second));
    free_memory(root_proc);
    printf("creating first process child\n");
    struct pcb_t *first_first = proc_alloc(first);
    free_memory(root_proc);
    printf("first delete--> %d\n", proc_delete(first));
    free_memory(root_proc);
    printf("first_first delete--> %d\n", proc_delete(first_first));
    free_memory(root_proc);
    printf("first delete --> %d\n", proc_delete(first));
    free_memory(root_proc);
}
