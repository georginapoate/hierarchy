#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hierarchy.h"

Tree create(char *name) {
	Tree e = NULL;
	e = malloc(sizeof(TreeNode));
	if (e == NULL) return NULL;
	e->name = strdup(name);
	if (e->name == NULL) return NULL;
	e->manager = NULL;
	e->team = NULL;
	e->direct_employees_no = 0;
	return e;
}


Tree get(Tree root, char *name) {
	if (root == NULL) return NULL;
	if (strcmp(name, root->name) == 0) return root;
	Tree member = NULL;  
	for (int i = 0; member == NULL && i < root->direct_employees_no; i++)
		member = get(root->team[i], name);
	return member;
}

void sort_team(Tree *team, int size) {
	Tree e = NULL;
	if (team == NULL) return;
	for (int i = 0; i < size - 1; i++) {
		for (int j = 0; j < size - 1; j++) {
			if (strcmp(team[j]->name, team[j + 1]->name) > 0) {
				e = team[j];
				team[j] = team[j + 1];
				team[j + 1] = e;
			}
		}
	}
}

void set(Tree boss, Tree member) {
	if (boss == NULL || member == NULL) return;
	int size = boss->direct_employees_no + 1;
	boss->team = realloc(boss->team, sizeof(Tree) * size);
	boss->direct_employees_no = size;
	boss->team[size - 1] = member;
	sort_team(boss->team, size);
	member->manager = boss;
}

void remove_from_manager(Tree boss, Tree member) {
	int i = 0;
	Tree *team = boss->team;
	int size = boss->direct_employees_no - 1;
	boss->direct_employees_no = size;
	while (i < size && team[i] != member) i++;
	while (i < size) {
		team[i] = team[i + 1];
		i++; 
	}
	if (size == 0) {
		free(boss->team);
		boss->team = NULL;
	} else {
		boss->team = realloc(team, size * sizeof(Tree));
	}
}

void transfer_team(Tree boss, Tree member) {
	if (member->direct_employees_no == 0) return;
	int size = boss->direct_employees_no;
	int size2 = member->direct_employees_no;

	boss->team = realloc(boss->team, (size + size2) * sizeof(Tree));
	boss->direct_employees_no = size + size2;
	member->direct_employees_no = 0;
	for (int i = 0; i < size2; i++) {
		member->team[i]->manager = boss;
		boss->team[size + i] = member->team[i];
	}
	sort_team(boss->team, size + size2);
	free(member->team);
	member->team = NULL;
}

/* Adauga un angajat nou in ierarhie.
 * 
 * tree: ierarhia existenta
 * employee_name: numele noului angajat
 * manager_name: numele sefului noului angajat
 *
 * return: intoarce ierarhia modificata. Daca tree si manager_name sunt NULL, 
           atunci employee_name e primul om din ierarhie.
 */

Tree hire(Tree tree, char *employee_name, char *manager_name) {
	if (manager_name == NULL) return create(employee_name);
	set(get(tree, manager_name), create(employee_name));
	return tree;
}

/* Sterge un angajat din ierarhie.
 * 
 * tree: ierarhia existenta
 * employee_name: numele angajatului concediat
 *
 * return: intoarce ierarhia modificata.
 */

void free_member(Tree member) {
	if (member == NULL) return;
	if (member->name != NULL) free(member->name);
	if (member->team != NULL) free(member->team);
	free(member);
}

Tree fire(Tree tree, char *employee_name) {
	Tree member = get(tree, employee_name);
	if (member == NULL) return tree;
	if (member == tree) return tree;
	for (int i = 0; i < member->direct_employees_no; i++) {
		set(member->manager, member->team[i]);
	}
	sort_team(member->manager->team, member->manager->direct_employees_no);
	remove_from_manager(member->manager, member);
	free_member(member);
	return tree;
}

/* Promoveaza un angajat in ierarhie. Verifica faptul ca angajatul e cel putin 
 * pe nivelul 2 pentru a putea efectua operatia.
 * 
 * tree: ierarhia existenta
 * employee_name: numele noului angajat
 *
 * return: intoarce ierarhia modificata.
 */
Tree promote(Tree tree, char *employee_name) {
	Tree member = get(tree, employee_name);
	if (member == NULL) return tree;
	if (member->manager == NULL) return tree;
	if (member->manager->manager == NULL) return tree;
	for (int i = 0; i < member->direct_employees_no; i++) {
		set(member->manager, member->team[i]);
	}
	sort_team(member->manager->team, member->manager->direct_employees_no);
	remove_from_manager(member->manager, member);
	set(member->manager->manager, member);

	if (member->team != NULL) {
		free(member->team);
		member->team = NULL;
		member->direct_employees_no = 0;
	}
	
	return tree;
}

/* Muta un angajat in ierarhie.
 * 
 * tree: ierarhia existenta
 * employee_name: numele angajatului
 * new_manager_name: numele noului sef al angajatului
 *
 * return: intoarce ierarhia modificata.
 */

Tree move_employee(Tree tree, char *employee_name, char *new_manager_name) {
	Tree member = get(tree, employee_name);
	if (member == NULL || member == tree) return tree;
	Tree boss = get(tree, new_manager_name);
	if (boss == NULL || member->manager == boss) return tree;
	transfer_team(member->manager, member);
	remove_from_manager(member->manager, member);
	set(boss, member);
	return tree;
}

/* Muta o echipa in ierarhie.
 * 
 * tree: ierarhia existenta
 * employee_name: numele angajatului din varful echipei mutate
 * new_manager_name: numele noului sef al angajatului
 *
 * return: intoarce ierarhia modificata.
 */

Tree move_team(Tree tree, char *employee_name, char *new_manager_name) {
	Tree member = get(tree, employee_name);
	if (member == NULL || member == tree) return tree;
	Tree boss = get(tree, new_manager_name);
	if (boss == NULL) return tree;
	remove_from_manager(member->manager, member);
	set(boss, member);
	return tree;
}

/* Concediaza o echipa din ierarhie.
 * 
 * tree: ierarhia existenta
 * employee_name: numele angajatului din varful echipei concediate
 *
 * return: intoarce ierarhia modificata.
 */

Tree fire_team(Tree tree, char *employee_name) {
	Tree employee = get(tree, employee_name);
	if (employee == NULL || employee == tree) return tree;
	remove_from_manager(employee->manager, employee);
	destroy_tree(employee);
	return tree;
}

/* Afiseaza toti angajatii sub conducerea unui angajat.
 * 
 * f: fisierul in care se va face afisarea
 * tree: ierarhia existenta
 * employee_name: numele angajatului din varful echipei
 */
void get_employees_by_manager(FILE *f, Tree tree, char *employee_name) {
}

/* Afiseaza toti angajatii de pe un nivel din ierarhie.
 * 
 * f: fisierul in care se va face afisarea
 * tree: ierarhia existenta
 * level: nivelul din ierarhie
 */
void get_employees_by_level(FILE *f, Tree tree, int level) {
}

/* Afiseaza angajatul cu cei mai multi oameni din echipa.
 * 
 * f: fisierul in care se va face afisarea
 * tree: ierarhia existenta
 */

void get_best_manager(FILE *f, Tree tree) {
}

/* Reorganizarea ierarhiei cu un alt angajat in varful ierarhiei.
 * 
 * tree: ierarhia existenta
 * employee_name: numele angajatului care trece in varful ierarhiei
 */

Tree reorganize(Tree tree, char *employee_name) {
	return tree;
}

/* Parcurge ierarhia conform parcurgerii preordine.
 *
 * f: fisierul in care se va face afisarea
 * tree: ierarhia existenta
 */

void preorder_traversal_help(FILE *f, Tree tree) {
	if (tree == NULL) return;
	fprintf(f, "%s-%s ", tree->name, tree->manager->name);
	int size = tree->direct_employees_no;
	for (int i = 0; i < size; i++) preorder_traversal_help(f, tree->team[i]);
}

void preorder_traversal(FILE *f, Tree tree) {
	if (tree == NULL) return;
	fprintf(f, "%s ", tree->name);
	int size = tree->direct_employees_no;
	for (int i = 0; i < size; i++) preorder_traversal_help(f, tree->team[i]);
	fprintf(f, "\n");
}

/* Elibereaza memoria alocata nodurilor din arbore
 *
 * tree: ierarhia existenta
 */
void destroy_tree(Tree tree) {
	if (tree == NULL) return;
	int size = tree->direct_employees_no;
	Tree *team = tree->team;
	free(tree->name);
	free(tree);
	if (team == NULL) return;
	for (int i = 0; i < size; i++) destroy_tree(team[i]);
	free(team);
}