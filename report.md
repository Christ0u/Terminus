# Compte rendu

Terminus est un interpréteur de commandes shell Linux implémenté en C.

Ce compte rendu détaille son principe de fonctionnement global ainsi que certains aspects plus spécifiques tel que la logique utilisée pour l'implémentation des différents opérateurs, par exemple.

## I - Boucle REPL

La boucle REPL (Read, Evaluate, Print, Loop) est une méthode d'interraction avec un programme informatique particulièrement adaptée à l'implémentation d'un terminal.

Elle se compose de trois séquences répétées indéfiniement jusqu'à la sortie du programme :

### > Lecture (Read)

Tout d'abord, l'utilisateur saisit une série d'instructions dans une invite de commandes. Une fois validée (touche `Enter`), les instructions sont lues pas le programme.

### > Evaluation (Evaluate)

Ensuite, les instructions sont décomposées en un tableau de jetons.

Par exemple, l'utilisateur saisit l'entrée suivante :

```BASH
ls -l | grep toto
```

Chaque élément (jeton) composant cette suite d'instructions est décomposée de la manière suivante : `["ls", "-l", "|", "grep", "toto"]`.

Une fois les jetons isolés, chacun d'entre eux est interprété et déclenche une action bien définie. Par exemple, le caractère `|` crée un tube (voir chapitre IV).

### > Affichage (Print)

Le résultat des opérations est affiché dans le terminal. Le programme peut également afficher des messages d'erreurs dans le cas d'instructions erronées.

## II - Définition d'une commande

<!--
On détaillera comment est définie une commande.
(c'est le moment de parler de la structure, de justifier son utilisation et d'expliquer comment elle est exploitée:D)
-->

## III - Commandes "built-in"

<!--
On détaille ici comment on fait la différence entre une builtin et une commande bash native
-->

Les commandes dites "built-in" sont des commandes internes qui ont leur propre implémentation au sein du code.

Terminus embarque les commandes built-in suivantes :

- `cd` : Change le répertoire de travail du shell
- `pwd` : Affiche le nom du répertoire de travail courant
- `echo` : Écrit les arguments sur la sortie standard
- `exit` : Termine le shell

Une distinction doit donc être faite entre ces commandes built-in intégrée à Terminus et les commandes natives Bash.

Pour ce faire, nous nous appuyons sur la structure `buitlin` qui, à un nom, associe une fonction.

Une boucle parcourt l'ensemble des built-in enregistrées. Si la saisie utilisateur correspond à l'une d'entre-elle, la fonction associée est alors exécutée. En revanche, si la saisie utilisateur ne correspond à aucune built-in enregistrée, elle est intérprétée comme étant une commande native Bash.

## IV - Opérateurs

<!--
Un ptit point sur les opérateurs serait pas de refus je pense.
- Opérateurs de redirection : < << > >> |
- Opérateurs de contrôle : && || &
-->

## Conclusion

<!--
Blabla de fin
-->
